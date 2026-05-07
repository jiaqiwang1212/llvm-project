# 第 1 章：组合已有变换

## 简介

Transform 方言允许精确地将变换定向到 IR 中的特定操作，并将它们链接起来，即将一个变换应用于前一个变换产生的操作。为实现这一点，变换被表达为 IR 中的其他操作。我们将包含这些操作的 IR 称为变换 IR（transform IR），将正在被变换的 IR 称为载荷 IR（payload IR）。

变换 IR 操作在值上工作，这些值可以与载荷 IR 的操作、值或属性相关联。我们将前两类值分别称为操作句柄（operation handle）和值句柄（value handle），将最后一类称为参数（parameter）。

变换 IR 的应用始终从一个顶层操作开始。在 C++ API 中，该操作被传递给 `applyTransforms` 函数。这个顶层操作指定了应执行哪些变换以及如何执行。最常见的顶层操作 `transform.named_sequence` 只是按顺序逐一应用其主体中列出的其他变换操作，类似于一个函数或宏。

让我们用一个简单的变换序列来说明这一点，作用于常见的"全连接 + 偏置 + ReLU"机器学习层，其本质是执行一次矩阵乘法，然后进行（逐元素的）矩阵加法，并对每个元素取与 0 的最大值。这可以用以下 IR 来表达：

```mlir
func.func @fc_relu(%lhs: tensor<512x512xf32>, %rhs: tensor<512x512xf32>,
                   %bias: tensor<512x512xf32>, %output: tensor<512x512xf32>)
                   -> tensor<512x512xf32> {
  // Matrix-matrix multiplication.
  %matmul = linalg.matmul ins(%lhs, %rhs: tensor<512x512xf32>, tensor<512x512xf32>)
                          outs(%output: tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise addition.
  %biased = linalg.elementwise kind=#linalg.elementwise_kind<add>
    ins(%matmul, %bias : tensor<512x512xf32>, tensor<512x512xf32>)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise max with 0 (ReLU).
  %c0f = arith.constant 0.0 : f32
  %relued = linalg.elementwise kind=#linalg.elementwise_kind<max_signed>
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> ()>, affine_map<(d0, d1) -> (d0, d1)>]
    ins(%biased, %c0f : tensor<512x512xf32>, f32)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>
  func.return %relued : tensor<512x512xf32>
}
```

## 顶层序列操作

出于性能考虑，我们希望对这些操作进行分块和融合以利用缓存局部性。这是一系列需要依次执行的变换，因此我们自然从相应的顶层变换操作开始。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
      %arg0: !transform.any_op,
      %arg1: !transform.op<"linalg.matmul">,
      %arg2: !transform.op<"linalg.elementwise">):
    transform.yield
  }
}
```

该操作有几个值得注意的方面。

其特殊名称 `@__transform_main` 和第一个参数是解释器 pass 所要求的，类似于 C 程序的入口点必须命名为 `main` 且可能具有 `int (int argc, char** argv)` 签名。该参数将与顶层载荷操作相关联，通常是 pass 所应用到的操作。注意，当通过 `applyTransforms` 或 `applyNamedSequence` _以编程方式_应用变换时，这些要求均不适用。

其余的入口块参数是可选的，可以与序列中有用的载荷属性、操作或值相关联。这些在调用 `applyTransforms` 时也会被指定。在我们的例子中，我们关注即将进行分块和融合的矩阵乘法和逐元素操作。

所有值句柄都具有 Transform 方言类型。这些类型指定了与其关联的载荷 IR 实体的某些属性。在本例中，`transform.any_op` 表示该句柄与任意载荷操作相关联。相反，`transform.op<"X">` 表示该句柄_仅_与 `X` 类型的载荷操作相关联。这些约束在创建句柄/载荷关联时会被验证。对于顶层变换操作的入口块参数，这发生在 `applyTransforms` 函数的早期阶段。如果约束不满足，变换应用将失败并向用户产生诊断信息。

最后，该操作被包装在一个带有 `transform.with_named_sequence` 属性的模块中，该属性在存在多个具名序列时会触发所有必要的验证。

## 失败传播

Transform 方言基础设施有一种特殊的诊断处理机制，支持可恢复错误。理解它最好从（无名）序列操作入手，该操作有一个必填属性，用于指定失败传播模式。有两种选项：

*   "propagate"（传播）：如果任何嵌套变换失败，则使序列变换失败；
*   "suppress"（抑制）：即使嵌套变换之一失败，序列也会成功，但不会尝试执行序列中失败变换之后的变换。

后者允许围绕序列的变换脚本在序列内部发生错误时继续执行，假设这些错误是可恢复的。由于我们只是在构建变换脚本，最好传播失败以便及时了解何时有变换未能应用。

要检查或调试变换序列，可以打印与变换 IR 值相关联的各种实体。例如，我们可以打印与句柄关联的操作：

```mlir
transform.sequence failures(propagate) {
^bb0(%arg0: !transform.any_op,
     %arg1: !transform.op<"linalg.matmul">,
     %arg2: !transform.op<"linalg.elementwise">):
  transform.debug.emit_remark_at %arg1, "matmul"
      : !transform.op<"linalg.matmul">
  transform.debug.emit_remark_at %arg2, "elemwise_binaries"
      : !transform.op<"linalg.elementwise">
  transform.yield
}
```

## Transform 方言解释器

由于我们不希望每次更改变换时都重新编译编译器，可以使用 Transform 方言解释器 pass 将此变换序列应用于载荷 IR。正如我们将在下一章中看到的，可以定义自定义 pass，甚至将变换解释器集成到更大的 pass 中。目前，我们可以使用现有的测试 pass：


```sh
$ mlir-opt sequence.mlir --pass-pipeline="
    builtin.module(transform-interpreter{
        debug-bind-trailing-args=linalg.matmul,linalg.elementwise})"
```

`sequence.mlir` 文件同时包含载荷 IR 函数和嵌套在同一模块中的变换 IR 序列。变换解释器 pass 将把 `@__transform_main` 具名序列应用于 pass 的锚操作。在我们的例子中，我们还通过相应的 pass 选项请求解释器 pass 将顶层序列的两个额外参数与所有 `linalg.matmul` 和 `linalg.elementwise` 载荷操作相关联。运行此 pass 会产生预期的备注信息：

```sh
sequence.mlir:5:13: remark: matmul
  %matmul = linalg.matmul ins(%lhs, %rhs: tensor<512x512xf32>, tensor<512x512xf32>)
            ^
sequence.mlir:5:13: note: see current operation: %0 = linalg.matmul ins(%arg0, %arg1 : tensor<512x512xf32>, tensor<512x512xf32>) outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32>
sequence.mlir:9:13: remark: elemwise_binaries
  %biased = linalg.elementwise kind=#linalg.elementwise_kind<add>
            ^
sequence.mlir:9:13: note: see current operation: %1 = linalg.elementwise kind=#linalg.elementwise_kind<add> ins(%0, %arg2 : tensor<512x512xf32>, tensor<512x512xf32>) outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32>
sequence.mlir:15:13: remark: elemwise_binaries
  %relued = linalg.elementwise kind=#linalg.elementwise_kind<max_signed>
            ^
sequence.mlir:15:13: note: see current operation: %2 = linalg.elementwise kind=#linalg.elementwise_kind<max_signed> indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> ()>, affine_map<(d0, d1) -> (d0, d1)>] ins(%1, %cst : tensor<512x512xf32>, f32) outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32>
```

注意 `%arg2` 与两个逐元素载荷操作都相关联。任何句柄都与一个实体列表相关联。各个变换可能关心也可能不关心列表中元素的顺序。


## 指定变换

现在我们已经获得了要变换的操作的句柄，可以开始应用变换了。让我们首先尝试对矩阵乘法操作本身进行分块。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
       %arg0: !transform.any_op,
       %arg1: !transform.op<"linalg.matmul">,
       %arg2: !transform.op<"linalg.elementwise">) {
    // The actual tiling transformation takes tile sizes as attributes.
    %loop, %tiled = transform.structured.tile_using_forall %arg1
                    tile_sizes [4, 32]
      : (!transform.op<"linalg.matmul">)
     -> (!transform.any_op, !transform.any_op)
    transform.yield
  }
}
```

该变换返回两个句柄，如其[文档](https://mlir.llvm.org/docs/Dialects/Transform/#transformstructuredtile_using_forall-transformtileusingforallop)所示：

*   一个指向在原始数据子集上操作的 `linalg.generic` 的句柄。
*   一个指向围绕张量的 `scf.forall` "多重循环"的句柄。

使用上述相同命令运行此变换，将如预期那样产生分块后的代码。

```mlir
#map = affine_map<(d0) -> (d0 * 4)>
#map1 = affine_map<(d0) -> (d0 * 32)>
#map2 = affine_map<(d0, d1) -> (d0, d1)>
#map3 = affine_map<(d0, d1) -> ()>

func.func @fc_relu(%arg0: tensor<512x512xf32>,
                   %arg1: tensor<512x512xf32>,
                   %arg2: tensor<512x512xf32>,
                   %arg3: tensor<512x512xf32>) -> tensor<512x512xf32> {
  %0 = scf.forall (%arg4, %arg5) in (128, 16) shared_outs(%arg6 = %arg3) -> (tensor<512x512xf32>) {
    %3 = affine.apply #map(%arg4)
    %4 = affine.apply #map1(%arg5)
    %extracted_slice = tensor.extract_slice %arg0[%3, 0] [4, 512] [1, 1]
                     : tensor<512x512xf32> to tensor<4x512xf32>
    %extracted_slice_0 = tensor.extract_slice %arg1[0, %4] [512, 32] [1, 1]
                     : tensor<512x512xf32> to tensor<512x32xf32>
    %extracted_slice_1 = tensor.extract_slice %arg6[%3, %4] [4, 32] [1, 1]
                     : tensor<512x512xf32> to tensor<4x32xf32>
    %5 = linalg.matmul
         ins(%extracted_slice, %extracted_slice_0
            : tensor<4x512xf32>, tensor<512x32xf32>)
         outs(%extracted_slice_1 : tensor<4x32xf32>) -> tensor<4x32xf32>
    scf.forall.in_parallel {
      tensor.parallel_insert_slice %5 into %arg6[%3, %4] [4, 32] [1, 1]
           : tensor<4x32xf32> into tensor<512x512xf32>
    }
  }
  %1 = linalg.elementwise kind=#linalg.elementwise_kind<add>
     ins(%0, %arg2 : tensor<512x512xf32>, tensor<512x512xf32>)
     outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32>
  %cst = arith.constant 0.000000e+00 : f32
  %2 = linalg.elementwise kind=#linalg.elementwise_kind<max_signed>
    indexing_maps = [#map2, #map3, #map2]
    ins(%1, %cst : tensor<512x512xf32>, f32)
    outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32>
  return %2 : tensor<512x512xf32>
}
```

除了产生新的句柄外，分块变换操作还会_消耗_操作数句柄。这意味着该句柄在此操作之后_失效_，不应再被使用。变换操作需要将其所有操作数标记为已消耗或只读。如果关联的载荷操作被删除或重新创建（即删除后以相似的结构重新创建），变换操作通常会消耗操作数。由于句柄本质上是对载荷操作的引用，如果载荷不再存在，它们就会成为悬空引用。


## 句柄失效与昂贵检查模式

未定义行为在发生时难以处理，因此 Transform 方言解释器默认执行一组额外的、可能代价较高的检查，以检测变换 IR 中的大多数未定义行为。例如，如果我们试图在 `%arg1` 句柄被消耗后使用它，在调试构建中会触发断言，在发布模式下可能导致段错误。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
       %arg0: !transform.any_op,
       %arg1: !transform.op<"linalg.matmul">,
       %arg2: !transform.op<"linalg.elementwise">) {
    // The actual tiling transformation takes tile sizes as attributes.
    %loop, %tiled = transform.structured.tile_using_forall %arg1 tile_sizes [4, 32]
        : (!transform.op<"linalg.matmul">) -> (!transform.any_op, !transform.any_op)

    // This is trying to use an invalidated handle leading to undefined behavior.
    transform.debug.emit_remark_at %arg1, "remark" : !transform.op<"linalg.matmul">
    transform.yield
  }
}
```

然而，启用解释器中的昂贵检查后，会产生友好的诊断信息：

```sh
sequence.mlir:28:3: error: op uses a handle invalidated by a previously executed transform op
  transform.debug.emit_remark_at %mm, "elemwise_binaries" : !transform.any_op
  ^
sequence.mlir:26:9: note: handle to invalidated ops
  %mm = transform.cast %matmul : !transform.op<"linalg.matmul"> to !transform.any_op
        ^
sequence.mlir:27:19: note: invalidated by this transform op that consumes its operand #0 and invalidates all handles to payload IR entities associated with this operand and entities nested in them
  %loop, %tiled = transform.structured.tile_using_forall %mm tile_sizes [4, 32]
```

当编译时性能是关注点，且变换序列足够稳定时，可以通过向 pass 提供 `disable-expensive-checks` 选项或在传入 `applyTransforms` 的 `TransformOptions` 中设置相应标志来禁用解释器中的昂贵检查以提升性能。

有人可能会注意到，某些操作（如 `transform.cast`）不会消耗操作数（因为它们不会删除对应的操作）。那么如果我们尝试使用该操作数会怎样呢？

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
       %arg0: !transform.any_op,
       %arg1: !transform.op<"linalg.matmul">,
       %arg2: !transform.op<"linalg.elementwise">) {
    // We can cast one type to another as long as operations are compatible
    // with both types. This creates "aliasing" handles.
    %casted = transform.cast %arg1 : !transform.op<"linalg.matmul">
        to !transform.any_op

    // The actual tiling transformation takes tile sizes as attributes.
    %loop, %tiled = transform.structured.tile_using_forall %arg1
                    tile_sizes [4, 32]
      : (!transform.op<"linalg.matmul">)
     -> (!transform.any_op, !transform.any_op)

    // Consuming an operand invalidates the consumed handle and any other handle
    // that is associated with the same payload operations, or payload
    // operations nested in them.
    transform.debug.emit_remark_at %casted, "remark"
      : !transform.any_op
    transform.yield
  }
}
```

`%arg1` 和 `%casted` 都引用同一个载荷操作。沿用引用类比，这两个引用互为别名。自然地，当载荷操作被删除时，所有对它的引用都会成为悬空引用。句柄也是如此。实际上，消耗一个操作数会使被消耗的句柄以及与相同载荷操作关联的任何其他句柄失效。载荷 IR 的考量是递归的：与被删除操作中_嵌套_的载荷操作关联的句柄也会失效（因为删除该操作也会删除其 region 及所有包含的操作）。昂贵检查模式也能处理这种情况。

```sh
sequence.mlir:28:3: error: op uses a handle invalidated by a previously executed transform op
  transform.debug.emit_remark_at %matmul, "elemwise_binaries" : !transform.op<"linalg.matmul">
  ^
sequence.mlir:21:29: note: handle to invalidated ops
^bb0(%root: !transform.any_op, %matmul: !transform.op<"linalg.matmul">, %elemwise: !transform.op<"linalg.elementwise">):
                            ^
sequence.mlir:27:19: note: invalidated by this transform op that consumes its operand #0 and invalidates all handles to payload IR entities associated with this operand and entities nested in them
  %loop, %tiled = transform.structured.tile_using_forall %mm tile_sizes [4, 32]
```

## 通过句柄链接变换

回到变换序列，我们已经对矩阵乘法进行了分块，但我们还想对逐元素操作进行分块和融合。在结构化操作范式中，典型的做法是对某个无环数据流图中的最后一个操作进行分块，然后逐步将产生其操作数的操作融合进来。这样就无需显式地对所有操作进行分块，因为融合可以自适应地调整大小并在需要时注入重新计算。因此，我们不对矩阵乘法操作进行分块，而是对链中的最后一个操作进行分块，然后将前面的操作融合到分块产生的循环中。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
       %arg0: !transform.any_op,
       %arg1: !transform.op<"linalg.matmul">,
       %arg2: !transform.op<"linalg.elementwise">) {
    // Since the %arg2 handle is associated with both elementwise operations,
    // we need to split it into two handles so we can target only the second
    // elementwise operation.
    %add, %max = transform.split_handle %arg2
        : (!transform.op<"linalg.elementwise">)
        -> (!transform.any_op, !transform.any_op)

    // The actual tiling transformation takes tile sizes as attributes. It
    // produces a handle to the loop generated during tiling.
    %tiled_max, %loop =
        transform.structured.tile_using_forall %max tile_sizes [8, 32]
          : (!transform.any_op) -> (!transform.any_op, !transform.any_op)

    // We can now fuse the other operations into the loop. Here, we fuse
    // operations one by one. This requires the operation that is being fused to
    // define the value used within the loop, so the order of such fusions is
    // important. We could also use "transform.merge_handles" to obtain a single
    // handle to all operations and give it to `fuse_into_containing_op` that
    // would take care of the ordering in this case.
    %add_fused, %loop_0 =
        transform.structured.fuse_into_containing_op %add into %loop
          : (!transform.any_op, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)
    %matmul_fused, %loop_1 =
        transform.structured.fuse_into_containing_op %arg1 into %loop_0
          : (!transform.op<"linalg.matmul">, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)

    transform.yield
  }
}
```

这实现了期望的分块和融合。

## 更多句柄失效

最后，假设存在一个高效的微内核或表达为内置函数的硬件指令，用于 4x4 矩阵乘法。为此，我们需要将融合后的操作分块到所需的大小，然后将其提取成独立函数（outline）。得到的函数调用可以替换为对微内核的调用。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
       %arg0: !transform.any_op,
       %arg1: !transform.op<"linalg.matmul">,
       %arg2: !transform.op<"linalg.elementwise">) {
    // Since the %arg2 handle is associated with both elementwise operations,
    // we need to split it into two handles so we can target only the second
    // elementwise operation.
    %add, %max = transform.split_handle %arg2
        : (!transform.op<"linalg.elementwise">)
          -> (!transform.any_op, !transform.any_op)

    // The actual tiling transformation takes tile sizes as attributes. It
    // produces a handle to the loop generated during tiling.
    %tiled, %loop = transform.structured.tile_using_forall %max
                    tile_sizes [8, 32]
        : (!transform.any_op) -> (!transform.any_op, !transform.any_op)

    // We can now fuse the other operations into the loop. Here, we fuse
    // operations one by one. This requires the operation that is being fused to
    // define the value used within the loop, so the order of such fusions is
    // important. We could also use "transform.merge_handles" to obtain a single
    // handle to all operations and give it to `fuse_into_containing_op` that
    // would take care of the ordering in this case.
    %add_fused, %loop_0 =
        transform.structured.fuse_into_containing_op %add into %loop
          : (!transform.any_op, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)
    %matmul_fused, %loop_1 =
        transform.structured.fuse_into_containing_op %arg1 into %loop_0
          : (!transform.op<"linalg.matmul">, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)

    // Tile again to get the desired size. Note that this time this tiles the
    // "add" operation and fuses matmul into the loop, but doesn't affect the
    // "max" operation. This illustrates the precise targeting with the
    // transform dialect. Otherwise, it is difficult to differentiate "add" and
    // "max", both of which having the same kind.
    %tiled_2, %loop_2 =
        transform.structured.tile_using_forall %add_fused tile_sizes [4, 4]
          : (!transform.any_op) -> (!transform.any_op, !transform.any_op)
    %matmul_fused_2, %loop_3 =
        transform.structured.fuse_into_containing_op %matmul_fused into %loop_2
          : (!transform.any_op, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)

    // Since outlining is currently only implemented for region-holding
    // operations such as loops, use tiling to size 1 to materialize the outer
    // loop that is going to be outlined.
    %_, %outline_target =
        transform.structured.tile_using_forall %tiled_2 tile_sizes [1]
          : (!transform.any_op) -> (!transform.any_op, !transform.any_op)
    transform.structured.fuse_into_containing_op %matmul_fused_2
        into %outline_target
          : (!transform.any_op, !transform.any_op)
            -> (!transform.any_op, !transform.any_op)
    %func, %call = transform.loop.outline %outline_target
                   {func_name = "outlined"}
        : (!transform.any_op) -> (!transform.any_op, !transform.op<"func.call">)

    transform.yield
  }
}
```

这个额外的变换也说明了嵌套操作的句柄失效问题。`transform.loop.outline` 操作消耗了指向循环的句柄，这会使其失效，以及所有指向其中任何嵌套操作（如 `%2`）的句柄。尝试使用这个句柄将导致未定义行为。（请注意，对于这种特定形式的提取，消耗操作数并非严格必要，因为该实现只是_移动_了 region 而没有重新创建操作，但变换的作者选择了无论如何都使该句柄失效。）

尝试在提取后访问融合结果会产生以下错误：

```sh
test/Examples/transform/Ch1/invalidation-2.mlir:109:3: error: op uses a handle invalidated by a previously executed transform op
  transform.debug.emit_remark_at %outline_target, "outlined loop" : !transform.any_op
  ^
test/Examples/transform/Ch1/invalidation-2.mlir:102:25: note: handle to invalidated ops
  %outline_target, %_ = transform.structured.tile_using_forall %tiled_2 tile_sizes [1]
                        ^
test/Examples/transform/Ch1/invalidation-2.mlir:106:18: note: invalidated by this transform op that consumes its operand #0 and invalidates all handles to payload IR entities associated with this operand and entities nested in them
  %func, %call = transform.loop.outline %outline_target {func_name = "outlined"}
                 ^
test/Examples/transform/Ch1/invalidation-2.mlir:24:13: note: ancestor payload op
  %biased = linalg.elementwise kind=#linalg.elementwise_kind<add>
            ^
test/Examples/transform/Ch1/invalidation-2.mlir:24:13: note: nested payload op
  %matmul = linalg.matmul ins(%lhs, %rhs: tensor<512x512xf32>, tensor<512x512xf32>)
```

注意，"add" 逐元素操作被指出为载荷祖先，因为它被用于产生分块循环，因此该循环具有其位置信息。

最后，我们希望将对已提取函数的调用替换为对微内核的调用。遗憾的是，Transform 方言没有对这种变换的支持（如果调用被重写为自定义的树外操作，则也无法提供支持）。因此，我们需要定义新的变换操作。后续章节将介绍如何实现这一点。

## 追踪 IR 修改

Transform 方言会自动追踪作为变换操作一部分所做的所有 IR 更改（实现必须使用所提供的重写器来修改 IR）。如果一个载荷操作被删除，它会自动从所有当前与之关联的句柄中移除。如果一个载荷操作被替换，Transform 方言会尝试找到替换操作并相应地更新所有句柄。如果一个多结果操作被替换为由多个操作定义的值，或者一个操作被替换为不同类型的操作，则会产生错误。这是因为直接替换是否真正代表原始操作的计算并不清晰。有办法可以自定义这种行为。更多详情请参阅 `transform::TrackingListener` 的文档。
