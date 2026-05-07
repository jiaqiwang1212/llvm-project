# Bufferization（缓冲化）

[TOC]

## 概述

MLIR 中的 Bufferization（缓冲化）是将具有 `tensor` 语义的 op 转换为具有 `memref` 语义的 op 的过程。有多个与缓冲化相关的 MLIR pass。这些 pass 通常在 pass 流水线的最后几个步骤中运行，就在将 `memref` op 降级到 LLVM 之前。这是因为许多转换在 tensor 层面更容易实现，或仅在 tensor 层面受支持；例如，[先在 tensor 上进行 tile/fuse/…](https://llvm.discourse.group/t/rfc-linalg-on-tensors-update-and-comprehensive-bufferization-rfc/3373)，然后再对剩余 IR 进行缓冲化。

![bufferization passes](/includes/img/bufferization_passes.svg)

最重要的缓冲化 pass 是 *One-Shot Bufferize*：此 pass 将 `tensor` IR 重写为 `memref` IR。还有一些额外的辅助 pass，用于预处理 IR（例如，使 IR 能够更高效地进行缓冲化）、执行缓冲区级别的优化（例如分配提升），以及[插入缓冲区释放 op](OwnershipBasedBufferDeallocation.md)，使生成的 `memref` IR 没有内存泄漏。

## 已废弃的 Pass

缓冲区释放 pass 已被废弃，取而代之的是基于所有权的缓冲区释放流水线。废弃的 pass 存在一些限制，可能导致生成的 IR 中出现内存泄漏。

## 什么是 One-Shot Bufferize？

One-Shot Bufferize 是一个 tensor 缓冲化 pass，专为[目标传递风格（destination-passing style）](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/11/dps-fhpc17.pdf)的 IR 设计，具有激进的就地缓冲化能力。

One-Shot Bufferize 的特点：

*   **单体式（Monolithic）**：由单个 MLIR pass 完成所有工作。

*   通过 op 接口实现**可扩展性**：所有实现了 `BufferizableOpInterface` 的 op 都可以进行缓冲化。

*   **全函数一次性分析**。就地缓冲化决策通过分析 tensor 上的 SSA use-def 链来做出。Op 接口实现不仅提供了从 tensor op 到 memref op 的重写逻辑，还提供了辅助方法，供 One-Shot Bufferize 的分析查询 op 的缓冲化/内存语义信息。

*   **两阶段（2-Phase）**：缓冲化在内部分为两步：首先，分析整个 IR 并做出缓冲化决策；然后，缓冲化（重写）IR。分析阶段可以访问精确的 SSA use-def 信息。它以增量方式构建别名集和等价集，不依赖于预分配内存的事后别名分析。

*   **贪心（Greedy）**：逐个分析操作，并当场决定某个 tensor OpOperand 是否需要复制。启发式方法确定分析的顺序。

*   **模块化（Modular）**：当前的 One-Shot 分析可以替换为不同的分析。分析结果通过 `AnalysisState` 被缓冲化查询，特别是 `AnalysisState::isInPlace`。任何实现了少量虚函数的 `AnalysisState` 派生类都可以充当自定义分析。甚至可以在不进行任何分析的情况下运行 One-Shot Bufferize（`AlwaysCopyAnalysisState`），在这种情况下，One-Shot Bufferize 在每次写入之前都会复制缓冲区。

请注意，One-Shot Bufferize 不会释放缓冲区。这由[基于所有权的缓冲区释放 pass](OwnershipBasedBufferDeallocation.md) 完成。

## 缓冲化的目标

每种缓冲化技术的高层目标是：

1. 尽可能少地使用内存。
2. 尽可能少地复制内存。

这意味着在可能的情况下重用已分配的缓冲区，使缓冲化成为一个算法复杂度类似于寄存器分配的问题。

根据具体的使用场景，可能还有额外的缓冲化要求。如果缓冲区的内容计算代价高昂，可能需要在*重新计算*和*计算一次并复制*之间进行权衡。相反，在某些架构上，可能根本无法在运行时分配新的缓冲区。

## 目标传递风格（Destination-Passing Style）

缓冲化是一个算法复杂的问题。给定一个带有 tensor 结果的 op，缓冲化必须选择一个可以存储结果的 memref 缓冲区。分配一个全新的缓冲区总是安全的，但这样的缓冲化策略对于高性能代码生成来说是不可接受的。当选择一个已存在的缓冲区时，我们必须小心，不要意外覆盖程序后续仍需要的数据。

为了简化这个问题，One-Shot Bufferize 被设计为利用*目标传递风格*（DPS）。在 MLIR 中，DPS op 应实现 [`DestinationStyleOpInterface`](https://github.com/llvm/llvm-project/blob/792d437b56adfb3416daf8105942d4899fb82763/mlir/include/mlir/Interfaces/DestinationStyleOpInterface.td)。DPS 独立于缓冲化而存在，与 SSA 语义相关联：许多 op 都在"更新"其输入 SSA 变量的某个部分。例如，LLVM 指令 [`insertelement`](https://llvm.org/docs/LangRef.html#insertelement-instruction) 是在向量中插入一个元素。由于 SSA 值是不可变的，该操作返回一个插入了元素的输入向量的副本。MLIR 中的另一个例子是对 tensor 执行的 `linalg.generic`，它对每个结果都有一个额外的 `outs` 操作数，提供要更新的初始值（例如，当操作执行归约时）。

`outs` 操作数在下文中被称为"目标"（引号很重要，因为这个操作数不会被就地修改，而是被复制），并在缓冲化的上下文中充当缓冲化算法可能的"锚点"。这允许用户将输入塑造为某种形式，通过仔细选择用作"目标"的 SSA 值，可以保证接近最优的缓冲化结果。

对于每个 tensor 结果，DPS op 都有一个对应的 tensor 操作数。如果这个 tensor 没有其他冲突的使用，缓冲化可以将其与 op 结果别名，并通过重用为此"目标"输入分配的缓冲区来"就地"执行操作。

例如，考虑以下 op：`%r = tensor.insert %f into %t[%idx] : tensor<5xf32>`

![tensor.insert example](/includes/img/bufferization_tensor_insert_dst.svg)

`%t` 是此示例中的"目标"。在为结果 `%r` 选择缓冲区时（记为 `buffer(%r)`），One-Shot Bufferize 只考虑两种选择：

1.  `buffer(%r) = buffer(%t)`：将结果存储在已存在的 `buffer(%t)` 中。
    请注意，这并非总是可能的。例如，如果 `buffer(%t)` 的旧内容仍然需要。One-Shot Bufferize 的主要任务是检测这类情况，并在必要时回退到第二种选择。
2.  `buffer(%r)` 是一个新分配的缓冲区。

同一函数中可能还有其他缓冲区可以用于 `buffer(%r)`，但 One-Shot Bufferize 不考虑这些缓冲区，以保持缓冲化的简洁性。未来 One-Shot Bufferize 可以扩展为考虑此类缓冲区，以实现更好的缓冲化质量。

不采用目标传递风格的 tensor op 总是缓冲化为内存分配。例如：

```mlir
%0 = tensor.generate %sz {
^bb0(%i : index):
  %cst = arith.constant 0.0 : f32
  tensor.yield %cst : f32
} : tensor<?xf32>
```

`tensor.generate` 的结果没有"目标"操作数，因此缓冲化会分配一个新缓冲区。这可以通过改用 `linalg.generic` 这样的 op 来避免，它可以用"目标"操作数来表达相同的计算，如 outputs（`outs`）中所指定：

```mlir
#map = affine_map<(i) -> (i)>
%0 = linalg.generic {indexing_maps = [#map], iterator_types = ["parallel"]}
                    outs(%t : tensor<?xf32>) {
  ^bb0(%arg0 : f32):
    %cst = arith.constant 0.0 : f32
    linalg.yield %cst : f32
} -> tensor<?xf32>
```

乍一看，上面的 `linalg.generic` op 可能看起来不太有用，因为输出 tensor `%t` 被完全覆盖了。为什么还要将 tensor `%t` 作为操作数传入？举个例子，这在覆盖 tensor 的一个切片时非常有用：

```mlir
%t = tensor.extract_slice %s [%idx] [%sz] [1] : tensor<?xf32> to tensor<?xf32>
%0 = linalg.generic ... outs(%t) { ... } -> tensor<?xf32>
%1 = tensor.insert_slice %0 into %s [%idx] [%sz] [1]
    : tensor<?xf32> into tensor<?xf32>
```

假设切片 `%t` 没有其他用户，上述示例缓冲化为一个 `memref.subview`，然后是一个"对 memref 执行的 `linalg.generic`"，该操作会覆盖子视图的内存。然后 `tensor.insert_slice` 缓冲化为无操作（在没有 RaW 冲突的情况下，例如后续对 `%s` 的读取）。

RaW 冲突通过对 SSA use-def 链的分析来检测（详情见后文）。当存在单一的 SSA use-def 链时，One-Shot Bufferize 效果最佳，即一个 tensor op 的结果是下一个 tensor op 的操作数，例如：

```mlir
%0 = "my_dialect.some_op"(%t) : (tensor<?xf32>) -> (tensor<?xf32>)
%1 = "my_dialect.another_op"(%0) : (tensor<?xf32>) -> (tensor<?xf32>)
%2 = "my_dialect.yet_another_op"(%1) : (tensor<?xf32>) -> (tensor<?xf32>)
```

如果 SSA use-def 链在某个点分叉，则可能会插入缓冲区副本，例如：

```mlir
%0 = "my_dialect.some_op"(%t) : (tensor<?xf32>) -> (tensor<?xf32>)
%1 = "my_dialect.another_op"(%0) : (tensor<?xf32>) -> (tensor<?xf32>)

// "yet_another_op" 可能需要读取 %0 的数据，因此 "another_op" 无法
// 就地写入 buffer(%0)。
%2 = "my_dialect.yet_another_op"(%0) : (tensor<?xf32>) -> (tensor<?xf32>)
```

## Tensor / Buffer 边界

bufferization 方言提供了一些辅助 op，用于将 tensor IR（应被缓冲化）与现有缓冲区（可能由不同运行时/库/等分配/提供）连接起来。

`bufferization.to_buffer %t` 返回 tensor SSA 值的未来缓冲区。`bufferization.to_tensor %m` 为给定的 MemRef 缓冲区返回一个 tensor SSA 值。`bufferization.materialize_in_destination` 表示 tensor 值应在某个缓冲区中实体化。

考虑以下示例，其中 TOSA matmul 的结果应在现有缓冲区 `%C` 中实体化：

```mlir
// 批量 TOSA 矩阵乘法。%A 和 %B 是输入，%C 是输出。
func.func @test_matmul(%A: memref<1x17x19xf32>,
                       %B: memref<1x19x29xf32>,
                       %C: memref<1x17x29xf32>) {

  %A_tensor = bufferization.to_tensor %A restrict : memref<1x17x19xf32> to tensor<1x17x19xf32>
  %B_tensor = bufferization.to_tensor %B restrict : memref<1x19x29xf32> to tensor<1x19x29xf32>

  %0 = tosa.matmul %A_tensor, %B_tensor
      : (tensor<1x17x19xf32>, tensor<1x19x29xf32>) ->
         tensor<1x17x29xf32>

  bufferization.materialize_in_destination
    %0 in restrict writable %C
      : (tensor<1x17x29xf32>, memref<1x17x29xf32>) -> ()

  return
}
```

请注意，此示例中所有缓冲化 op 都设置了 `restrict` 单元属性。此属性类似于 C 语言的 restrict 关键字，表示不存在其他具有相同或别名 MemRef 操作数的 `to_tensor` 或 `materialize_in_destination` op。只有这样的 `to_tensor`/`materialize_in_destination` op 才被支持。`restrict` 属性为缓冲化分析提供了强有力的别名保证，允许我们只关注程序中的 tensor IR。（不对 tensor 进行操作的 op 被 One-Shot Bufferize 忽略。）

还请注意，`tosa.matmul` 无法按原样缓冲化：该 op 没有 `BufferizableOpInterface` 实现。但是，该 op 可以降级为 `tensor.empty` 和 `linalg.matmul` 的组合，后者可以被缓冲化。

## 使用 One-Shot Bufferize

MLIR 提供了一个 pass [`-one-shot-bufferize`](https://mlir.llvm.org/docs/Passes/#-one-shot-bufferize-one-shot-bufferize)，它执行分析并对所有具有 tensor 语义且实现了 `BufferizableOpInterface` 的 op 进行缓冲化。出于模块化原因，这些 op 接口实现通常是外部模型，位于方言的"Transforms"构建单元中。（外部模型是一种在不同构建单元中实现 op 接口的机制。）用户有责任确保在运行 One-Shot Bufferize 之前注册所有需要的外部模型。

默认情况下，当 One-Shot Bufferize 遇到具有 tensor 语义（即 tensor 结果或 tensor 操作数）但不可缓冲化（即未实现 `BufferizableOpInterface`）的 op 时，会失败。可以使用 `allow-unknown-ops` 来避免这种情况。在这种情况下，One-Shot Bufferize 会在缓冲化边界周围插入 `to_buffer`/`to_tensor` op。

One-Shot Bufferize 可以配置为仅使用 `dialect-filter` 对一组方言中的 op 进行缓冲化。

One-Shot Bufferize 也可以通过 [`bufferization::runOneShotBufferize`](https://github.com/llvm/llvm-project/blob/ae2764e835a26bad9774803eca0a6530df2a3e2d/mlir/include/mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h#L167) 以编程方式调用。或者，[`bufferization::bufferizeOp`](https://github.com/llvm/llvm-project/blob/ae2764e835a26bad9774803eca0a6530df2a3e2d/mlir/include/mlir/Dialect/Bufferization/Transforms/Bufferize.h#L78) 会跳过分析并在每次缓冲区写入时插入副本。

默认情况下，函数边界不进行缓冲化。这是因为函数图缓冲化目前存在一些限制：不支持递归调用。只要没有递归调用，可以使用 `bufferize-function-boundaries` 启用函数边界缓冲化。然后每个 tensor 函数参数和 tensor 函数结果将被转换为 memref。memref 类型的布局映射可以通过 `function-boundary-type-conversion` 控制。

## 内存布局

One-Shot Bufferize 自上而下地对 op 进行缓冲化。当所有 op 都可缓冲化时，这种方式效果很好。但是，当使用 `allow-unknown-ops` 遇到不可缓冲化的 tensor 时，One-Shot Bufferize 必须在缓冲化边界处插入 `to_buffer` op，并决定 memref 类型。默认情况下，One-Shot Bufferize 选择关于布局映射最动态的 memref 类型。例如：

```mlir
%0 = "my_dialect.unbufferizable_op(%t) : (tensor<?x?xf32>) -> (tensor<?x?xf32>)
%1 = tensor.extract %0[%idx1, %idx2] : tensor<?xf32>
```

在对上述 IR 进行缓冲化时，One-Shot Bufferize 插入一个带有动态偏移和步幅的 `to_buffer` op：

```mlir
%0 = "my_dialect.unbufferizable_op(%t) : (tensor<?x?xf32>) -> (tensor<?x?xf32>)
%0_m = bufferization.to_buffer %0 : memref<?x?xf32, strided<[?, ?], offset: ?>>
%1 = memref.load %0_m[%idx1, %idx2] : memref<?x?xf32, strided<[?, ?], offset: ?>>
```

`%0` 的所有用户都有完全动态的布局映射。这确保了缓冲化后的 IR 与 `unbufferizable_op` 未来的缓冲化（可能由另一个 pass 缓冲化）良好组合，无论未来缓冲化的确切 memref 类型如何。如果该 op 最终被缓冲化为具有更简单 memref 类型的 op（例如，identity 布局映射），我们预期规范化模式会清理不必要的动态布局映射。（其中一些规范化模式可能尚未实现。）

One-Shot Bufferize 在缓冲化 op 时尽量推断最精确的 memref 类型。如果整个 IR 都可缓冲化，我们不必保守地使用完全动态的布局映射。在这种情况下，我们也不必依赖规范化模式来清理缓冲化后的 IR。

注意：有些可缓冲化的 op 无法推断出精确的布局映射。例如，从 `tensor<*xf32>` 到 `tensor<?x?xf32>` 的 `tensor.cast` 必须缓冲化为带有完全动态布局映射的 memref 类型的 `memref.cast`。

One-Shot Bufferize 有一个 `unknown-type-conversion` 选项，用于控制在无法推断出精确布局时的布局映射生成：

*   `fully-dynamic-layout-map` 使用完全动态的布局映射，是默认行为。当 IR 被部分缓冲化时，这种方式组合性好。
*   `identity-layout-map` 使用静态的 identity 布局映射。此选项对于无法处理带布局映射的 memref 类型的遗留代码可能很有用。
    请注意，当折叠 `to_tensor`/`to_buffer` 对时，如果 memref 类型不兼容转换，此设置可能导致额外的缓冲区副本。

注意：`unknown-type-conversion` 选项不影响函数签名的布局映射。有一个单独的 `function-signature-type-conversion` 选项用于控制函数参数和函数结果的布局映射。

## 扩展 One-Shot Bufferize

如果自定义 op 实现了 `BufferizableOpInterface`，则可以对其进行缓冲化。用户至少必须实现以下接口方法：

*   `bufferizesToMemoryRead`：如果读取了给定 tensor OpOperand 的缓冲区，则返回 `true`。
*   `bufferizesToMemoryWrite`：如果写入了给定 tensor OpOperand 的缓冲区（就地缓冲化时），则返回 `true`。
*   `getAliasingOpResult`：返回可能与给定 OpOperand 共享同一缓冲区的 OpResult。此接口方法描述了关于目标传递风格的 OpOperand 到 OpResult 映射。
*   `bufferRelation`：如果给定的 OpResult 在缓冲化后（就地缓冲化时）与别名 OpOperand 是完全相同的 memref，则返回 `BufferRelation::Equivalent`。否则（例如，它们重叠但不一定是完全相同的 memref），应返回 `BufferRelation::Unknown`。未来将添加更多缓冲区关系，但 `BufferRelation::Unknown` 始终是安全的。
*   `bufferize`：使用给定的 rewriter 重写 op。Op 应使用 `bufferization::replaceOpWithBufferizedValues` 进行替换。

为了更好地理解接口方法，我们建议用户参考 MLIR 中的现有实现，例如 `tensor.insert` 或 `tensor.extract` 的实现。

DPS op（实现了 `DestinationStyleOpInterface`）的接口实现可以派生自 `DstBufferizableOpInterfaceExternalModel`，它提供了除 `bufferize` 之外所有必要的方法实现。

## 调试缓冲区副本

为了更好地理解 One-Shot Bufferize 为何引入缓冲区副本，用户可以使用 `test-analysis-only print-conflicts` 运行该 pass。然后每个 tensor op 都会被一个属性注释，该属性对每个 tensor OpOperand 都有一个布尔值。`true` 表示 OpOperand 就地缓冲化。`false` 表示 OpOperand 非就地缓冲化，将插入一个缓冲区副本。

缓冲区副本可能被插入的原因有两个：

1.  由于 RaW 冲突，就地缓冲化不安全。即，被覆盖的数据仍然需要。
2.  缓冲区不可写。例如，作为 `arith.constant` op 结果的 `memref.global` 缓冲区永远不会被修改。

在第一种情况下，`print-conflicts` 以 ("read", "conflicting write", "last write") 元组的形式说明冲突。

RaW 冲突由三部分组成，按 op 支配顺序如下：

1. **定义（Definition）：** 定义了一个 tensor `%t`。
2. **冲突写（Conflicting Write）：** 一个操作写入 `buffer(%t)`。
3. **读取（Read）：** 一个操作读取 `%t`。

当在分析阶段检测到此类 RaW 冲突时，One-Shot Bufferize 将为冲突写入插入一个缓冲区副本。

**示例**

```mlir
// RUN: mlir-opt %s -one-shot-bufferize="bufferize-function-boundaries test-analysis-only print-conflicts"
func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, tensor<3xf32>) {
  // 创建一个包含 [%arg0, %arg0, %arg0] 的新 tensor。
  %0 = tensor.from_elements %arg0, %arg0, %arg0 : tensor<3xf32>

  // 向新 tensor 中插入内容。
  %1 = tensor.insert %arg1 into %0[%arg2] : tensor<3xf32>

  // 从旧 tensor 中读取。
  %r = tensor.extract %0[%arg3] : tensor<3xf32>

  // 返回提取的值和插入的结果。
  func.return %r, %1 : f32, tensor<3xf32>
}
```

输出 IR 如下：

```mlir
func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, tensor<3xf32>) {
  %from_elements = tensor.from_elements %arg0, %arg0, %arg0 {"C_0[DEF: result 0]"} : tensor<3xf32>
  %inserted = tensor.insert %arg1 into %from_elements[%arg2] {"C_0[CONFL-WRITE: 1]", __inplace_operands_attr__ = ["none", "false", "none"]} : tensor<3xf32>
  %extracted = tensor.extract %from_elements[%arg3] {"C_0[READ: 0]", __inplace_operands_attr__ = ["true", "none"]} : tensor<3xf32>
  return {__inplace_operands_attr__ = ["none", "true"]} %extracted, %inserted : f32, tensor<3xf32>
}
```

请注意，IR 没有被缓冲化。它只是被缓冲化分析的结果所注释。每个具有 tensor 语义的操作都有一个 `__inplace_operands_attr__` 属性，每个操作数对应一个值。如果操作数不是 tensor，则相应的值为 `none`。否则，如果该操作数被决定就地缓冲化，则值为 `true`。值为 `false` 表示将插入缓冲区副本。在上述示例中，将为 `tensor.insert` 插入一个缓冲区副本，使其不会覆盖 `buffer(%from_elements)`（`tensor.extract` 仍然需要它）。

对于每个 RaW（示例中只有一个），添加了三个 `C_i` 属性：

* `C_0[DEF: result 0]`：定义了一个 tensor：`tensor.from_elements` 的第 0 个结果。
* `C_0[CONFL-WRITE: 1]`：一个操作（如果就地缓冲化）将写入所定义 tensor 的未来缓冲区：`tensor.insert` 的第 1 个操作数。
* `C_0[READ: 0]`：一个操作读取了 tensor 定义：`tensor.extract` 的第 0 个操作数。

完整缓冲化后的 IR（含插入的缓冲区副本）如下：

```mlir
func.func @test(%arg0: f32, %arg1: f32, %arg2: index, %arg3: index) -> (f32, memref<3xf32>) {
  %c2 = arith.constant 2 : index
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %alloc = memref.alloc() {alignment = 64 : i64} : memref<3xf32>
  memref.store %arg0, %alloc[%c0] : memref<3xf32>
  memref.store %arg0, %alloc[%c1] : memref<3xf32>
  memref.store %arg0, %alloc[%c2] : memref<3xf32>
  %alloc_0 = memref.alloc() {alignment = 64 : i64} : memref<3xf32>
  memref.copy %alloc, %alloc_0 : memref<3xf32> to memref<3xf32>
  memref.store %arg1, %alloc_0[%arg2] : memref<3xf32>
  %0 = memref.load %alloc[%arg3] : memref<3xf32>
  return %0, %alloc_0 : f32, memref<3xf32>
}
```

为了更好地理解 SSA Use-Def 链分析和 RaW 冲突检测算法，感兴趣的用户可以参考：

* [原始设计文档](https://discourse.llvm.org/uploads/short-url/5kckJ3DftYwQokG252teFgw3sYa.pdf)
* [ODM 演讲](https://youtu.be/TXEo59CYS9A)（[幻灯片](https://mlir.llvm.org/OpenMeetings/2022-01-13-One-Shot-Bufferization.pdf)）。
* [LLVM Dev Meeting 2023 教程幻灯片](https://m-sp.org/downloads/llvm_dev_2023.pdf)
