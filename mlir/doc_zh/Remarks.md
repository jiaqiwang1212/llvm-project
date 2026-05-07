# Remark 基础设施

Remark 是编译器发出的**结构化、人机可读的注释**，用于传达：

- 应用了哪些转换
- 遗漏了哪些优化
- 为什么做出了某些决策

**`RemarkEngine`** 在编译期间收集 remark 并将其路由到可插拔的**流处理器（streamer）**。默认情况下，MLIR 与 LLVM 的
[`llvm::remarks`](https://llvm.org/docs/Remarks.html) 基础设施集成，使您能够：

- 在 pass 运行时流式传输 remark
- 序列化为 **YAML** 或 **LLVM 位流**

***

## 概述

- **按需启用** – 默认禁用；除非启用，否则零开销。
- **按上下文配置** – 在 `MLIRContext` 上配置。
- **格式** – LLVM Remark 引擎（YAML / 位流）或自定义流处理器。
- **类型** – `Passed`（通过）、`Missed`（遗漏）、`Failure`（失败）、`Analysis`（分析）。
- **API** – 使用 `<<` 的轻量级流式接口（类似 MLIR 诊断信息）。

***

## 架构

remark 系统由两个主要组件组成：

### RemarkEngine

由 `MLIRContext` 持有，该引擎：

- 接收已完成的 `InFlightRemark` 对象
- 可选地将 remark 镜像到 `DiagnosticEngine`
- 分发到已安装的流处理器

### MLIRRemarkStreamerBase

一个具有单个钩子的抽象后端接口：

```c++
virtual void streamOptimizationRemark(const Remark &remark) = 0;
```

默认实现 **`MLIRLLVMRemarkStreamer`** 将 `mlir::Remark` 适配到 LLVM 的 remark 格式，并通过
`llvm::remarks::RemarkStreamer` 写入 YAML 或位流。

**所有权链：** `MLIRContext` → `RemarkEngine` → `MLIRRemarkStreamerBase`

***

## Remark 类别

MLIR 提供四个内置类别：

### Passed（通过）

优化或转换成功。

```
[Passed] RemarkName | Category:Vectorizer:myPass1 | Function=foo | Remark="vectorized loop", tripCount=128
```

### Missed（遗漏）

优化未应用，理想情况下提供可操作的反馈。

```
[Missed]  | Category:Unroll | Function=foo | Reason="tripCount=4 < threshold=256", Suggestion="increase unroll to 128"
```

### Failure（失败）

尝试了优化但失败。与 `Missed` 不同，这表示一次主动尝试但未能完成。

例如，当用户请求 `--use-max-register=100` 但分配器无法满足约束时：

```
[Failed] Category:RegisterAllocator | Reason="Limiting to use-max-register=100 failed; it now uses 104 registers for better performance"
```

### Analysis（分析）

中性的信息输出——对性能分析和调试很有用。

```
[Analysis] Category:Register | Remark="Kernel uses 168 registers"
[Analysis] Category:Register | Remark="Kernel uses 10kB local memory"
```

***

## 发出 Remark

使用 `remark::*` 辅助函数创建**飞行中的 remark（in-flight remark）**，然后用 `<<` 运算符追加内容。

### 配置 Remark 选项

每个 remark 接受四个字段（均为 `StringRef`）：

| 字段              | 描述                                     |
|------------------|----------------------------------------|
| **Name**         | remark 的可识别名称                         |
| **Category**     | 高级分类                                   |
| **Sub-category** | 细粒度分类                                  |
| **Function**     | remark 来源的函数                           |

### 基本示例

```c++
#include "mlir/IR/Remarks.h"

LogicalResult MyPass::runOnOperation() {
  Location loc = getOperation()->getLoc();

  auto opts = remark::RemarkOpts::name("VectorizeLoop")
                  .category("Vectorizer")
                  .subCategory("MyPass")
                  .function("foo");

  // Passed：转换成功
  remark::passed(loc, opts)
      << "vectorized loop"
      << remark::metric("tripCount", 128);

  // Analysis：信息输出
  remark::analysis(loc, opts)
      << "Kernel uses 168 registers";

  // Missed：优化被跳过（带原因和建议）
  remark::missed(loc, opts)
      << remark::reason("tripCount={0} < threshold={1}", 4, 256)
      << remark::suggest("increase unroll factor to {0}", 128);

  // Failure：尝试了优化但失败
  remark::failed(loc, opts)
      << remark::reason("unsupported pattern encountered");

  return success();
}
```

***

## 指标和辅助函数

所有辅助函数接受
[LLVM 格式字符串](https://llvm.org/docs/ProgrammersManual.html#formatting-strings-the-formatv-function)，
这些字符串是惰性构建的——确保在禁用 remark 时零开销。

| 辅助函数                           | 描述                                     |
|----------------------------------|----------------------------------------|
| `remark::metric(key, value)`     | 添加结构化键值对                               |
| `remark::add(fmt, ...)`          | `metric("Remark", ...)` 的快捷方式          |
| `remark::reason(fmt, ...)`       | `metric("Reason", ...)` 的快捷方式          |
| `remark::suggest(fmt, ...)`      | `metric("Suggestion", ...)` 的快捷方式     |

### 字符串简写

追加普通字符串：

```c++
remark::passed(loc, opts) << "vectorized loop";
```

等价于：

```c++
remark::passed(loc, opts) << remark::metric("Remark", "vectorized loop");
```

### 自定义指标

为机器可读性添加结构化数据：

```c++
remark::passed(loc, opts)
    << "loop optimized"
    << remark::metric("TripCount", 128)
    << remark::metric("VectorWidth", 4);
```

***

## 发出策略

`RemarkEngine` 支持可插拔的策略来控制发出哪些 remark。

### RemarkEmittingPolicyAll

无条件发出**所有** remark。

### RemarkEmittingPolicyFinal

仅发出每个位置的**最终** remark。这在多 pass 编译器中很有用，其中早期 pass 可能报告失败，但后期 pass 成功。

**示例：** 仅发出成功的 remark：

```c++
auto opts = remark::RemarkOpts::name("Unroller").category("LoopUnroll");

// 第一个 pass：报告失败
remark::failed(loc, opts) << "Loop could not be unrolled";

// 后期 pass：报告成功（这个才会被发出）
remark::passed(loc, opts) << "Loop unrolled successfully";
```

您还可以通过继承策略接口来实现自定义策略。

***

## 启用 Remark

### 选项 1：LLVM Remark 流处理器（YAML 或位流）

将 remark 持久化到文件以供后处理：

```c++
// 设置类别
remark::RemarkCategories cats{
    /*passed=*/   "LoopUnroll",
    /*missed=*/   std::nullopt,
    /*analysis=*/ std::nullopt,
    /*failed=*/   "LoopUnroll"
};

// 使用最终策略
std::unique_ptr<remark::RemarkEmittingPolicyFinal> policy =
        std::make_unique<remark::RemarkEmittingPolicyFinal>();

remark::enableOptimizationRemarksWithLLVMStreamer(
    context, outputFile, llvm::remarks::Format::YAML, std::move(policy), cats);
```

**YAML 输出**（人类可读）：

```yaml
*** !Passed
pass:     Vectorizer:MyPass
name:     VectorizeLoop
function: foo
loc:      input.mlir:12:3
args:
  - Remark:    vectorized loop
  - tripCount: 128
```

**位流格式** —— 用于大规模分析的紧凑二进制格式。

### 选项 2：诊断引擎（无流处理器）

将 remark 镜像到标准诊断输出：

```c++
// 设置类别
remark::RemarkCategories cats{
    /*passed=*/   "LoopUnroll",
    /*missed=*/   std::nullopt,
    /*analysis=*/ std::nullopt,
    /*failed=*/   "LoopUnroll"
};

// 使用最终策略
std::unique_ptr<remark::RemarkEmittingPolicyFinal> policy =
        std::make_unique<remark::RemarkEmittingPolicyFinal>();

remark::enableOptimizationRemarks(
    context,
    /*streamer=*/ nullptr,
    /*policy=*/ std::move(policy),
    cats,
    /*printAsEmitRemarks=*/ true);
```

### 选项 3：自定义流处理器

实现您自己的后端以支持专用输出格式：

```c++
class MyStreamer : public MLIRRemarkStreamerBase {
public:
  void streamOptimizationRemark(const Remark &remark) override {
    // 自定义序列化逻辑
  }
};

auto streamer = std::make_unique<MyStreamer>();
remark::enableOptimizationRemarks(context, std::move(streamer), cats);
```
