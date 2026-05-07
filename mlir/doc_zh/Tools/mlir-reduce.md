# MLIR Reduce

[TOC]

MLIR 输入在经过一系列转换后可能会触发 bug。为了定位问题根源或在修复后帮助验证，开发者希望能够减小 bug 复现器的大小。本文档描述 `mlir-reduce`，它类似于 [bugpoint](https://llvm.org/docs/CommandGuide/bugpoint.html)，是一个可以减小触发错误所需输入大小的工具。

`mlir-reduce` 支持以多种方式减小输入，包括简单地删除不需要重现错误的代码、启发式地应用简化器模式，或运行优化 pass 来减小输入。要使用它，首先需要提供一个命令，说明输入是否"有趣"，例如，表现出您想关注的特征。例如，您可能想查看 `mlir-opt` 在某个特定 MLIR 输入上运行后是否失败。然后，选择您的简化策略，`mlir-reduce` 将为您完成剩余工作。

## 使用方法

`mlir-reduce` 采用简化树算法来减小输入。它生成几个简化后的输出，并根据树遍历策略在它们之间进一步简化。不同的策略可能导致不同的结果和不同的时间复杂度。例如，您可以运行 `-reduction-tree='traversal-mode=0'` 来选择模式。

### 编写测试有趣性的脚本

如前所述，您需要向 `mlir-reduce` 提供一个命令，用于识别您感兴趣的情况。对于简化过程中生成的每个中间输出，`mlir-reduce` 将在其上运行该命令，脚本应对有趣的情况返回 1，否则返回 0。示例脚本如下：

```shell
mlir-opt -convert-vector-to-spirv $1 | grep "failed to materialize"
if [[ $? -eq 1 ]]; then
  exit 1
else
  exit 0
fi
```

示例用法如下，注意 `test` 参数是模式参数的一部分：

```shell
mlir-reduce $INPUT -reduction-tree='traversal-mode=0 test=$TEST_SCRIPT'
```

## 可用的简化策略

### 操作消除

`mlir-reduce` 将尝试直接删除操作。这是最激进的简化，因为它可能导致无效输出，只要最终保留了测试脚本认为有趣的错误消息即可。为避免这种情况，`mlir-reduce` 总是检查有效性，并期望用户也提供有效输入。

### 将模式重写为更简单的形式

在某些情况下，将操作重写为更简单或更小的形式仍然可以保留有趣性。例如，`mlir-reduce` 将尝试将未知秩的 `tensor<?xindex>` 重写为固定秩的常量形式，如 `tensor<1xi32>`。这不仅能产生更简单的操作，还可能因为更精确的类型信息而引入进一步的简化机会。

MLIR 支持方言，`mlir-reduce` 也支持每个方言的重写模式。这意味着您可以拥有方言特定的重写模式。为此，您需要实现 `DialectReductionPatternInterface`。例如：

```c++
#include "mlir/Reducer/ReductionPatternInterface.h"

struct MyReductionPatternInterface : public DialectReductionPatternInterface {
  MyReductionPatternInterface(Dialect *dialect)
      : DialectReductionPatternInterface(dialect) {};

  virtual void
  populateReductionPatterns(RewritePatternSet &patterns) const final {
    populateMyReductionPatterns(patterns);
  }
}
```

`mlir-reduce` 将调用 `populateReductionPatterns` 来收集每个方言提供的简化重写模式。提示：如果您使用 [DRR](../DeclarativeRewrites.md) 编写简化模式，可以利用 `mlir-tblgen` 生成的 `populateWithGenerated` 方法。

### 使用内置优化 pass 进行简化

MLIR 提供了大量转换 pass，其中一些对减小输入大小很有用，例如 Symbol-DCE。`mlir-reduce` 将与上述两种策略一起调度它们。

## 构建自定义 mlir-reduce

在以下情况下：1. 定义了自定义语法，2. 失败特定于某些方言，或 3. 有方言特定的简化器模式，您需要构建自己的 `mlir-reduce`。将其与 `MLIRReduceLib` 链接并按如下方式实现：

```c++
#include "mlir/Tools/mlir-reduce/MlirReduceMain.h"
using namespace mlir;

int main(int argc, char **argv) {
  DialectRegistry registry;
  registerMyDialects(registry);
  // Register the DialectReductionPatternInterface if any.
  MLIRContext context(registry);
  return failed(mlirReduceMain(argc, argv, context));
}

```

## 未来工作

`mlir-reduce` 缺少几个功能：

*   `-reduction-tree` 目前只支持 `Single-Path` 遍历模式，使用不同的遍历策略扩展它可能会更好地减小输入。
*   在中断时产生最优结果。简化过程可能需要很长时间，如果在触发中断时能获得目前为止的最优结果将会更好。
