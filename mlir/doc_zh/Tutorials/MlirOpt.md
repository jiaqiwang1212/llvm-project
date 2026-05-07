# 使用 `mlir-opt`

`mlir-opt` 是一个用于在 MLIR 代码上运行通道（pass）和降低（lowering）的命令行入口点。本教程将介绍如何使用 `mlir-opt`，展示一些使用示例，并提到一些使用它的实用技巧。

前置条件：

- [从源码构建 MLIR](/getting_started/)
- [MLIR 语言参考](/docs/LangRef/)

[TOC]

## `mlir-opt` 基础

`mlir-opt` 工具将文本 IR 或字节码加载到内存结构中，并可选地在将 IR 序列化回去之前执行一系列通道（默认为文本格式）。它旨在作为测试和调试工具。

构建 MLIR 项目后，`mlir-opt` 二进制文件（位于 `build/bin` 中）是运行通道和降低以及输出调试和诊断数据的入口点。

不带任何标志运行 `mlir-opt` 将从标准输入消费文本或字节码 IR，对其进行解析并运行验证器，然后将文本格式写回标准输出。这是测试 MLIR 输入是否格式正确的好方法。

`mlir-opt --help` 显示完整的标志列表（接近 1000 个）。每个通道都有自己的标志，但建议使用 `--pass-pipeline` 而不是裸标志来运行通道。

## 运行通道

接下来我们在以下 IR 上运行 [`convert-to-llvm`](/docs/Passes/#-convert-to-llvm)，它将所有支持的方言转换为 `llvm` 方言：

```mlir
// mlir/test/Examples/mlir-opt/ctlz.mlir
module {
  func.func @main(%arg0: i32) -> i32 {
    %0 = math.ctlz %arg0 : i32
    func.return %0 : i32
  }
}
```

构建 MLIR 后，从 `llvm-project` 根目录运行

```bash
build/bin/mlir-opt --pass-pipeline="builtin.module(convert-math-to-llvm)" mlir/test/Examples/mlir-opt/ctlz.mlir
```

产生

```mlir
module {
  func.func @main(%arg0: i32) -> i32 {
    %0 = "llvm.intr.ctlz"(%arg0) <{is_zero_poison = false}> : (i32) -> i32
    return %0 : i32
  }
}
```

请注意，这里的 `llvm` 是 MLIR 的 `llvm` 方言，仍然需要通过 `mlir-translate` 处理才能生成 LLVM-IR。

## 运行带选项的通道

接下来我们将展示如何运行一个接受配置选项的通道。考虑以下包含缓存局部性较差的循环的 IR。

```mlir
// mlir/test/Examples/mlir-opt/loop_fusion.mlir
module {
  func.func @producer_consumer_fusion(%arg0: memref<10xf32>, %arg1: memref<10xf32>) {
    %0 = memref.alloc() : memref<10xf32>
    %1 = memref.alloc() : memref<10xf32>
    %cst = arith.constant 0.000000e+00 : f32
    affine.for %arg2 = 0 to 10 {
      affine.store %cst, %0[%arg2] : memref<10xf32>
      affine.store %cst, %1[%arg2] : memref<10xf32>
    }
    affine.for %arg2 = 0 to 10 {
      %2 = affine.load %0[%arg2] : memref<10xf32>
      %3 = arith.addf %2, %2 : f32
      affine.store %3, %arg0[%arg2] : memref<10xf32>
    }
    affine.for %arg2 = 0 to 10 {
      %2 = affine.load %1[%arg2] : memref<10xf32>
      %3 = arith.mulf %2, %2 : f32
      affine.store %3, %arg1[%arg2] : memref<10xf32>
    }
    return
  }
}
```

使用 [`affine-loop-fusion`](/docs/Passes/#-affine-loop-fusion) 通道运行此 IR 会产生一个融合后的循环。

```bash
build/bin/mlir-opt --pass-pipeline="builtin.module(affine-loop-fusion)" mlir/test/Examples/mlir-opt/loop_fusion.mlir
```

```mlir
module {
  func.func @producer_consumer_fusion(%arg0: memref<10xf32>, %arg1: memref<10xf32>) {
    %alloc = memref.alloc() : memref<1xf32>
    %alloc_0 = memref.alloc() : memref<1xf32>
    %cst = arith.constant 0.000000e+00 : f32
    affine.for %arg2 = 0 to 10 {
      affine.store %cst, %alloc[0] : memref<1xf32>
      affine.store %cst, %alloc_0[0] : memref<1xf32>
      %0 = affine.load %alloc_0[0] : memref<1xf32>
      %1 = arith.mulf %0, %0 : f32
      affine.store %1, %arg1[%arg2] : memref<10xf32>
      %2 = affine.load %alloc[0] : memref<1xf32>
      %3 = arith.addf %2, %2 : f32
      affine.store %3, %arg0[%arg2] : memref<10xf32>
    }
    return
  }
}
```

此通道有一些选项允许用户配置其行为。例如，`fusion-compute-tolerance` 选项描述为"融合时可容忍的额外计算量的分数增加"。如果在命令行上将此值设置为零，该通道将不会融合循环。

```bash
build/bin/mlir-opt --pass-pipeline="builtin.module(affine-loop-fusion{fusion-compute-tolerance=0})" \
mlir/test/Examples/mlir-opt/loop_fusion.mlir
```

```mlir
module {
  func.func @producer_consumer_fusion(%arg0: memref<10xf32>, %arg1: memref<10xf32>) {
    %alloc = memref.alloc() : memref<10xf32>
    %alloc_0 = memref.alloc() : memref<10xf32>
    %cst = arith.constant 0.000000e+00 : f32
    affine.for %arg2 = 0 to 10 {
      affine.store %cst, %alloc[%arg2] : memref<10xf32>
      affine.store %cst, %alloc_0[%arg2] : memref<10xf32>
    }
    affine.for %arg2 = 0 to 10 {
      %0 = affine.load %alloc[%arg2] : memref<10xf32>
      %1 = arith.addf %0, %0 : f32
      affine.store %1, %arg0[%arg2] : memref<10xf32>
    }
    affine.for %arg2 = 0 to 10 {
      %0 = affine.load %alloc_0[%arg2] : memref<10xf32>
      %1 = arith.mulf %0, %0 : f32
      affine.store %1, %arg1[%arg2] : memref<10xf32>
    }
    return
  }
}
```

传递给通道的选项使用语法 `{option1=value1 option2=value2 ...}` 指定，即对每个选项使用空格分隔的 `key=value` 对。

## 在命令行上构建通道流水线

`--pass-pipeline` 标志支持将多个通道组合成一个流水线。到目前为止，我们使用了"锚定"在顶层 `builtin.module` 操作上、只包含单个通道的简单流水线。[通道锚定](/docs/PassManagement/#oppassmanager) 是通道指定它们只在特定操作上运行的方式。虽然许多通道锚定在 `builtin.module` 上，但如果你尝试在 `--pass-pipeline="builtin.module(pass-name)"` 中运行一个锚定在其他操作上的通道，它将不会运行。

多个通道可以通过在 `--pass-pipeline` 字符串中以逗号分隔的列表提供通道名称来链接在一起，例如 `--pass-pipeline="builtin.module(pass1,pass2)"`。这些通道将顺序运行。

要使用具有非平凡锚定的通道，必须在通道流水线中指定适当的嵌套级别。例如，考虑以下具有相同冗余代码但在两个不同嵌套级别的 IR。

```mlir
module {
  module {
    func.func @func1(%arg0: i32) -> i32 {
      %0 = arith.addi %arg0, %arg0 : i32
      %1 = arith.addi %arg0, %arg0 : i32
      %2 = arith.addi %0, %1 : i32
      func.return %2 : i32
    }
  }

  gpu.module @gpu_module {
    gpu.func @func2(%arg0: i32) -> i32 {
      %0 = arith.addi %arg0, %arg0 : i32
      %1 = arith.addi %arg0, %arg0 : i32
      %2 = arith.addi %0, %1 : i32
      gpu.return %2 : i32
    }
  }
}
```

以下流水线运行 `cse`（公共子表达式消除），但仅在两个 `builtin.module` 操作内部的 `func.func` 上运行。

```bash
build/bin/mlir-opt mlir/test/Examples/mlir-opt/ctlz.mlir --pass-pipeline='
    builtin.module(
        builtin.module(
            func.func(cse,canonicalize),
            convert-to-llvm
        )
    )'
```

输出保持 `gpu.module` 不变

```mlir
module {
  module {
    llvm.func @func1(%arg0: i32) -> i32 {
      %0 = llvm.add %arg0, %arg0 : i32
      %1 = llvm.add %0, %0 : i32
      llvm.return %1 : i32
    }
  }
  gpu.module @gpu_module {
    gpu.func @func2(%arg0: i32) -> i32 {
      %0 = arith.addi %arg0, %arg0 : i32
      %1 = arith.addi %arg0, %arg0 : i32
      %2 = arith.addi %0, %1 : i32
      gpu.return %2 : i32
    }
  }
}
```

出于性能原因，指定具有嵌套锚定的通道流水线也是有益的：具有锚定的通道可以并行运行在 IR 子集上，这提供了更好的线程运行时和线程内的缓存局部性。例如，即使通道没有被限制锚定在 `func.func` 上，运行 `builtin.module(func.func(cse, canonicalize))` 也比 `builtin.module(cse, canonicalize)` 更高效。

有关通道流水线文本描述语言的规范，请参阅[文档](/docs/PassManagement/#textual-pass-pipeline-specification)。有关通道管理的更多通用信息，请参阅[通道基础设施](/docs/PassManagement/#)。

## 实用的 CLI 标志

- `--debug` 打印由 `LLVM_DEBUG` 调用产生的所有调试信息。
- `--debug-only="my-tag"` 仅打印在具有宏 `#define DEBUG_TYPE "my-tag"` 的文件中由 `LLVM_DEBUG` 产生的调试信息。这通常允许你仅打印与特定通道相关的调试信息。
    - `"greedy-rewriter"` 仅打印使用贪婪重写引擎应用的模式的调试信息。
    - `"dialect-conversion"` 仅打印方言转换框架的调试信息。
 - `--emit-bytecode` 以字节码格式输出 MLIR。
 - `--mlir-pass-statistics` 打印关于运行通道的统计信息。这些统计信息通过[通道统计](/docs/PassManagement/#pass-statistics)生成。
 - `--mlir-print-ir-after-all` 在每个通道后打印 IR。
    - 另见 `--mlir-print-ir-after-change`、`--mlir-print-ir-after-failure`，以及这些标志将 `after` 改为 `before` 的类似版本。
    - 使用 `print-ir` 标志时，添加 `--mlir-print-ir-tree-dir` 可将 IR 写入目录树中的文件，使其比大量转储到终端更易于检查。
 - `--mlir-timing` 显示每个通道的执行时间。

## 延伸阅读

- [通道列表](/docs/Passes/)
- [方言列表](/docs/Dialects/)
