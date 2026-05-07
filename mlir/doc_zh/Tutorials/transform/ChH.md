# 第 H 章：复现 Halide 调度

本章演示如何使用 Transform 方言为结构化操作实现 [Halide DSL](http://halide-lang.org) 中的一个调度。

请注意，下面的 IR 是为了简洁而省略了类型的伪代码，可能与当前语法不同步。请始终以 [mlir/examples/transform/ChH](https://github.com/llvm/llvm-project/tree/main/mlir/test/Examples/transform/ChH) 中的源代码为准。

## 通道卷积

Transform 方言为在 MLIR 中实现"变换指令"领域特定语言（DSL）提供了基础设施。这样的 DSL，至少在其调度部分，可以以 Transform 方言中的操作为目标，这些操作随后由编译器应用。可以添加变换操作集合，甚至利用相同接口和基础设施的新方言，以支持针对特定调度模型的特定 DSL。在本章中，我们将重新审视 Halide DSL，该语言（重新）推广了最初用于图像处理程序的独立调度规格。

将 Halide 映射到 Transform 方言有两种可能的方法：

*   创建一个对应于 Halide DSL 计算部分的新方言，并定义一组封装在 Transform 方言操作中的变换，对应于 DSL 的调度部分。
*   将 Halide 抽象映射到现有的 MLIR 抽象，包括 DSL 的两个部分。

我们将考虑后一种方法，因为 DSL 的计算部分可以很容易地映射到 Linalg 方言中的结构化操作。这也让我们有机会讨论 Linalg 对所谓结构化操作的变换与现有变换的异同。

我们将考虑从 Halide [应用示例](https://github.com/halide/Halide/tree/294f80c49bf3bb8582446613c25fcce03b82bcd8/apps/conv_layer)中提取的二维通道卷积示例。

```cpp
// Sizes of the problem.
const int N = 5, CI = 128, CO = 128, W = 100, H = 80;

// Sized inputs. Note that the order of dimensions is
// inverted in Halide with respect to C++, so the last dimension
// in the list (N for input, CI for filter) is the least
// frequently varying. The C++ equivalent is input[N][H+2][W+2][CI].
Buffer<float, 4> input({CI, W+2, H+2, N}, "input");
Buffer<float, 4> filter({CO, 3, 3, CI}, "filter");
Buffer<float, 1> bias(std::vector<int>{CO}, "bias");

// ... data initialization happens here ...

// Declarations of "mathematical functions" for convolution and relu.
Func conv("conv"), relu("relu");

// Iterators/subscripts.
Var x("x"), y("y"), c("c"), n("n");

// 3D reduction domain (channels and 2 window dimensions),
// dimensions are later referred to as r.x, r.y, r.z.
RDom r(0, CI, 0, 3, 0, 3);

// Core convolution with the result initialized to the bias value.
// Note that the order of iterators is inverted in Halide DSL,
// i.e. `n` corresponds to the lest frequently-varying (outermost) dimension
// here and below.
conv(c, x, y, n) = bias(c);
conv(c, x, y, n) += filter(c, r.y, r.z, r.x) * input(r.x, x + r.y, y + r.z, n);

// ReLU rectification, an elementwise operation.
relu(c, x, y, n) = max(0, conv(c, x, y, n));
```

这几乎可以直接转换为操作张量的 Linalg 方言，后者在概念上更接近"数学函数"抽象，且大多数变换都在张量上可用。

```mlir
// Bias. Using a named Linalg operation for brevity.
%bias_init = tensor.empty() : !toutput
%biased = linalg.broadcast ins(%bias : !tbias)
                          outs(%bias_init : !toutput) dimensions = [0, 1, 2]

// Convolution proper. While Linalg has named operations for 2D convolutions,
// the one in the Halide example has an uncommon order of filter dimensions
// and is not supported. It also takes the filter as first argument. This
// code recreates it faithfully using the generic form.
%convolved = linalg.generic {
  iterator_types = ["parallel", "parallel", "parallel", "parallel",
                    "reduction", "reduction", "reduction"],
  indexing_maps = [
    affine_map<(n, y, x, c, rz, ry, rx) -> (rx, rz, ry, c)>,
    affine_map<(n, y, x, c, rz, ry, rx) -> (n, y+rz, x+ry, rx)>,
    affine_map<(n, y, x, c, rz, ry, rx) -> (n, y, x, c)>
  ]
} ins(%filter, %input: !tfilter, !tinput)
  outs(%biased : !toutput) {
^bb0(%in: f32, %f: f32, %b: f32):
  // Note the fastmath attributes that allow operations to be recombined into
  //   %0 = math.fma %in, %f, %b : f32
  // later on and to reorder reductions.
  %m1 = arith.mulf %in, %f  {fastmath = #arith.fastmath<fast>} : f32
  %0 = arith.addf %b, %m1  {fastmath = #arith.fastmath<fast>} : f32
  linalg.yield %0 : f32
} -> !toutput

// ReLU is just a max(0, x).
%c0 = arith.constant 0.0 : f32
%relued = linalg.generic {
  iterator_types = ["parallel", "parallel", "parallel", "parallel"],
  indexing_maps = [
    affine_map<(d0, d1, d2, d3) -> ()>,
    affine_map<(d0, d1, d2, d3) -> (d0, d1, d2, d3)>,
    affine_map<(d0, d1, d2, d3) -> (d0, d1, d2, d3)>
  ]
} ins(%c0, %convolved : f32, !toutput)
  outs(%output : !toutput) {
^bb0(%cst: f32, %in: f32, %out: f32):
  %0 = llvm.intr.maxnum(%cst, %in) : (f32, f32) -> f32
  linalg.yield %0 : f32
} -> !toutput
```

在 Halide 中，`conv` 这样的函数可能由两部分组成：一个"函数式"初始化计算和一个用于归约的原地更新。这在嵌入式 DSL 中表示为两条 C++ 语句，但在内部表示为单个对象。Linalg 没有这种能力，初始化和更新被表示为两个互不相连的独立 Linalg 操作。此外，Halide DSL 中的 `x`、`y`、`c`、`n` 变量对应于在相应对象上迭代的隐式循环，这意味着在其定义中共享这些变量的函数也共享相应的循环。换句话说，Halide 定义的循环等价形式从完全融合的形式开始。Linalg 模型则与之相反，每个结构化操作对应自己的循环嵌套，结果形成完全分布的形式。这将影响后续调度的构建方式。

Halide 计算的循环结构如下所示（改编自 `HL_DEBUG_CODEGEN=1` 的调试输出）：

```python
for n
  for y
    for x
      for c
        conv[n, y, x, c] = bias[c]
        for rz
          for ry
            for rx
              conv[n, y, x, c] += filter[rx, rz, ry, c] * input[n, y+rz, x+ry, rx]
        relu[n, y, x, c] = max(0, conv[n, y, x, c])
```

Linalg 计算的循环结构如下（通过 `mlir-opt --linalg-generalize-named-ops --empty-tensor-to-alloc-tensor --one-shot-bufferize --convert-linalg-to-loops` 得到）：

```python
for n
  for y
    for x
      for c
        init[n, y, x, c] = bias[c]
for n
  for y
    for x
      for c
        for rz
          for ry
            for rx
              conv[n, y, x, c] += filter[rx, rz, ry, c] * input[n, y+rz, x+ry, rx]
for n
  for y
    for x
      for c
        relu[n, y, x, c] = max(0, conv[n, y, x, c])

```

## 将 Halide 调度原语映射到 Linalg 结构化变换

示例中列出的完整 Halide 调度如下：

```cpp
Var co, ci, xo, xi;
relu.split(c, co, ci, vec * tile_w)
  .split(x, xo, xi, tile_h)
  .reorder(ci, xi, xo, y, n, co)
  .vectorize(ci, vec)
  .unroll(ci)
  .unroll(xi)
  .parallel(y)
  .parallel(n)
  .parallel(co);

conv.compute_at(relu, xo)
  .vectorize(c, vec)
  .unroll(c)
  .unroll(x)
  .unroll(y)
  .update()
  .reorder(c, x, y, r.x, r.y, r.z, n)
  .vectorize(c, vec)
  .unroll(c)
  .unroll(x)
  .unroll(y)
  .unroll(r.x, 2);
```

我们将只考虑不并行化的情况，以避免 Halide 和 MLIR 所生成并行运行时之间的差异。该调度对应于一系列循环操作、展开和向量化。以下指令均存在，可按如下方式映射到 Linalg 上的变换：

*   `split` 将一个循环维度分解为两个直接嵌套的循环，内层循环最多具有给定数量的迭代次数。这可以理解为循环_条带挖掘（strip-mining）_，或使用任何 `linalg.tile_` 变换操作对单个维度进行退化分块。我们将使用 `transform.structured.tile_using_forall`，因为这种循环在缓冲化方面得到最好的支持，也可以在之后转换为并行循环。与 Halide 不同，这不会向原始操作添加新维度，而是在其外围创建一个循环，并将操作本身重写为对原始数据子集的操作。
*   `reorder` 任意重新排列循环。在 Linalg 表示中，循环是隐式的，并且应尽量保持隐式以便目标微内核。`linalg.generic` 操作中隐式循环的顺序可以通过 `transform.structured.interchange` 来更改，但这不适用于首先需要通过调用 `transform.structured.generalize` 来"泛化"的具名操作。然而，这只能重新排序隐式维度，而不能重新排序由分块操作实体化的显式循环（这些循环不能再被"折叠"回原始操作）。作为替代，我们可以利用这种行为，通过"分块"为大小 1 来直接以期望的顺序实体化循环。
*   `vectorize` 表示给定维度应以给定因子进行向量化；如果循环范围大于因子，循环实际上被分割为两部分，内层部分被向量化。相反，结构化 Linalg 操作的向量化作为全局变换应用于所有适合的操作，例如通过 `transform.structured.vectorize_children_and_apply_patterns` 在函数范围内应用。它依赖 MLIR 对多维向量的支持，直接将多维张量映射到相应类型，这些类型随后在降级时被分解为硬件兼容的较小向量上的操作。
*   `unroll` 执行循环展开，完全展开或展开到给定因子。等价于 `transform.loop.unroll`。
*   `compute_at` 表示函数的值必须在为另一个函数生成的给定循环内计算；根据环绕函数的循环之间的关系，这对应于循环分布或生产者/消费者融合。鉴于 Linalg 表示从完全分布形式开始，可以将其表示为一系列 `transform.structured.fuse_into_containing_op`，作用于事先通过分块实体化的 `forall` 循环。


## 重建循环结构

Halide 调度中 `relu` 的前三个变换指令旨在产生如下循环结构：

```python
for co
  for n
    for y
      for xo
        for xi
          for ci
            relu[n, y, xo*tile_h + xi, co*tile_w*vec + ci] = ...
```

注意 `c` 的外层部分从所有周围循环中被提升出来。操作的隐式循环顺序为 `n, y, x, c`，因此需要首先实体化 `co` 循环，以实现期望的重新排序。其余维度可以在一次变换中作为循环实体化。

```mlir
    //                                                             [n  y  x  c]
    %co, %relu2 = transform.structured.tile_using_forall %relu
                                                        tile_sizes [0, 0, 0, 64]
    %n_y_xo, %relu3 = transform.structured.tile_using_forall %relu2
                                                        tile_sizes [1, 1, 5, 0]
```

这将在 IR 中创建以下循环，嵌套的逐元素操作通过隐式循环对原始数据的较小子集进行操作：

```mlir
scf.forall (%co) in (2) {
  scf.forall (%n, %y, %xo) in (5, 80, 20) {
    tensor.extract_slice
    // Implicit dimensions [ni=0:1, y=0:1, xi=0:5, ci=0:64]
    %relued = linalg.elementwise kind=#linalg.elementwise_kind<max_signed> // ...
    scf.forall.in_parallel {
      tensor.parallel_insert_slice // ...
    }
  }
}
```

接下来是对 `conv` 函数的 `compute_at` 和 `reorder` 变换，这些变换需要在循环被展开和向量化破坏之前进行。它们旨在产生最终期望的循环结构：

```python
for co
  for n
    for y
      for xo
        for xi
          for ci
            conv[n, y, x*tile_h + xi, co*tile_w*vec + ci] = ...
        for rz
          for ry
            for rx
              for xi
                for ci
                  conv[n, y, x*tile_h + xi, co*tile_w*vec + ci] += ...
        for xi
          for ci
            relu[n, y, xo*tile_h + xi, co*tile_w*vec + ci] = ...
```

实际上，这对应于将卷积初始化和更新融合到之前通过分块实体化的 `co, n, y, xo` 循环中。结构化操作变换集支持将值的生产者融合到其消费者中，因此融合分两个阶段进行：

*   首先，将主卷积更新融合到使用它的 ReLU 中，后者已有实体化的循环；
*   然后，将偏置初始化融合到卷积 + ReLU 循环嵌套中。

每个阶段由两个变换组成，先将计算操作融合到外层循环，再融合到内层循环。

```mlir
%conv2, %co2 = transform.structured.fuse_into_containing_op %conv into %co
%conv3, %n_y_xo2 = transform.structured.fuse_into_containing_op %conv2
  into %n_y_xo

%bias2, %co3 = transform.structured.fuse_into_containing_op %bias into %co2
%bias3, %n_y_xo3 = transform.structured.fuse_into_containing_op %bias2
  into %n_y_xo2
```

为完成结构，我们需要将 `rz, ry, rx` 循环放在"分块"循环 `xi, ci` 的外侧。这可以通过从卷积操作实体化相应的循环来实现。然而，这些是归约循环，将其实体化为本质上并行的"forall"循环是无效的。因此，我们使用专用的"归约分块"变换并生成顺序的 `scf.for` 循环。（`scf.forall` 循环也可以表达并行归约，但对应的变换在撰写本文时尚不支持沿多个维度同时归约。）

```mlir
%rz_ry_rx, %red_fill, %conv4, %comb
  = transform.structured.tile_reduction_using_for %conv3
//               n  y  x  c  rz ry rx
  by tile_sizes=[0, 0, 0, 0, 1, 1, 1]
```

该变换在卷积操作外围实体化了期望的循环。它的能力不止于产生（归约）循环：变换后的代码执行 `N / tile_size` 个元素的 `tile_size` 次部分归约（通过改变循环内结构化操作的维度类型，这些归约可能并行进行），然后通过在循环之后产生一个新的"合并器"结构化操作来执行这些部分结果的最终归约。在我们的例子中，所有维度的 `tile_size = 1`，因此归约完全由生成的循环执行。合并器结构化操作仍然被产生，并将归约结果与初始值相加。这改变了浮点运算的顺序（非单位大小的归约分块也会如此），可能由于这些运算不满足交换律而影响最终结果，但 `fastmath` 标志明确允许这样做。Halide 也以完整的 `fastmath` 标志生成 LLVM IR。

最后，我们需要产生仍未显式存在的最内层循环 `xi` 和 `ci`。由于下一步将沿 `ci` 方向进行向量化，我们需要考虑它在 MLIR 结构化操作上的工作方式：它不是选择特定的向量大小和循环/维度进行向量化，而是直接将多维向量类型替换张量类型并相应地更新操作。因此，张量类型不应变为平凡（即大小为 1），而应在期望轴 `ci` 上保留一个 `vector_size` 大小的维度。这可以通过以 `vector_size` 为该维度的分块大小来实现：

```mlir
//                                                                  n  y  xi ci
%1, %c5 = transform.structured.tile_using_forall %conv4 tile_sizes [0, 0, 1, 16]
%2, %b4 = transform.structured.tile_using_forall %bias3 tile_sizes [0, 0, 1, 16]
%3, %r4 = transform.structured.tile_using_forall %relu3 tile_sizes [0, 0, 1, 16]
%4, %c2 = transform.structured.tile_using_forall %comb  tile_sizes [0, 0, 1, 16]
```

注意，归约分块产生的合并器操作在此也被分块。


## 显式循环展开

剩余未处理的循环变换是展开。具体而言，对构成 16 元素向量操作的 4x5 分块的最内层循环请求展开，以确保使用 20 个 512 位向量寄存器作为累加器的连续 `vfma` 指令序列。对 `unroll(y)` 和 `unroll(r.x, 2)` 的额外展开请求在调度中出现，但_没有实际效果_。也就是说，Halide 在删除这些指令后产生的代码及所有中间表示，与使用完整调度产生的代码_完全相同_。因此，我们只展开对应于 `xi` 和 `ci` 维度的循环，这些循环确实被 Halide 展开了。

由于 Transform 方言中的分块会生成指向由分块实体化的循环的句柄，展开这些循环只需链接相应的变换即可。注意，内层循环必须先被展开，因为展开外层循环会使指向内层循环的句柄失效。

```mlir
transform.loop.unroll %bias_ci {factor = 4}
transform.loop.unroll %bias_xi {factor = 5}
transform.loop.unroll %conv_ci {factor = 4}
transform.loop.unroll %conv_xi {factor = 5}
transform.loop.unroll %relu_ci {factor = 4}
transform.loop.unroll %relu_xi {factor = 5}
transform.loop.unroll %comb_ci {factor = 4}
transform.loop.unroll %comb_xi {factor = 5}
```

## 向量化

这些变换产生了期望的循环结构，我们现在准备进行向量化。在进行之前，最好先简化代码，因为分块和融合可能产生了大量用于计算张量子集和循环范围的操作，其中一些可能是重复的或过于复杂的。涉及规范化、公共子表达式消除、循环不变代码外提以及各种重写模式的简化可以直接从 transform 方言中应用。此外，任意组合的重写模式可以_在一次扫描中_应用于给定范围，这是一种_传统编译器 pass 无法实现的功能_（至少在不为每种模式组合创建新 pass 的情况下），因为传统 pass 分别应用每组模式。

```mlir
%f00 = transform.structured.match ops{["func.func"]} in %arg0
transform.apply_patterns to %f00 {
  transform.apply_patterns.canonicalization
  transform.apply_patterns.linalg.tiling_canonicalization
}
transform.apply_cse to %f00

%all_loops = transform.structured.match interface{LoopLikeInterface} in %arg0
transform.apply_licm to %all_loops
```

向量化前还需要一个最终的简化步骤，以生成良好的向量化代码。通过"分块为 1"来实体化循环产生了处理 4D 类型的结构化（`linalg`）操作，其中只有一个维度不是单位大小，例如 `tensor<1x1x1x16xf32>`，其中 16 是对应 AVX512 的向量大小。这是因为结构化分块不修改操作的秩以保留原始结构。尽管核心计算是相同的，生成的代码可能比必要的更复杂，尤其是在将多维向量分解为硬件支持的单维向量时。这些单位维度可以在向量化之前使用相应的模式集显式折叠掉：

```mlir
transform.apply_patterns to %f00 {
  transform.apply_patterns.linalg.fold_unit_extent_dims_via_reshapes
}

%fv = transform.structured.vectorize_children_and_apply_patterns %f00
```

这产生了在 `vector<16xf32>` 类型上执行算术操作的期望代码，下游编译器可以轻松将其降级为 AVX512 指令。向量化可能创造了新的代码简化机会，特别是将张量子集操作和向量切片操作结合起来。可以在向量化后应用另一轮简化：

```mlir
transform.apply_patterns to %fv {
  transform.apply_patterns.canonicalization
  transform.apply_patterns.tensor.fold_tensor_subset_ops_into_vector_transfers
}
transform.apply_cse to %fv
transform.structured.hoist_redundant_vector_transfers %fv
```

## 降级至 LLVM 与缓冲化障碍

完成循环重构后，程序现在需要被转换为可执行形式。第一步是_缓冲化（bufferization）_，即将载荷 IR 中的每个张量与一个内存缓冲区相关联的过程。MLIR 的单次缓冲化可直接作为变换操作使用：

```mlir
%arg1 = transform.bufferization.one_shot_bufferize %arg0 {
  bufferize_function_boundaries = true,
  function_boundary_type_conversion = 1 : i32 }
```

单次缓冲化本身不产生缓冲区释放，这可能导致内存泄漏。因此我们必须运行缓冲区释放 pass 流水线来避免泄漏。注意，Transform 方言可以无缝地运行具名 pass 和 pass 流水线：如果需要，可以用操作替换复杂的 `--pass-pipeline` 表达式。注意我们将流水线应用于函数而非整个模块，以避免在包含在模块中的变换 IR 上运行它：

```mlir
%f = transform.structured.match ops{["func.func"]} in %arg1
  : (!transform.any_op) -> !transform.any_op
transform.apply_registered_pass "buffer-deallocation-pipeline" to %f
  : (!transform.any_op) -> !transform.any_op
```

在这种特定情况下，变换后的 IR 可以直接缓冲化。但通常情况并非如此，某些操作（特别是 `tensor.empty`）可能无法缓冲化。在运行缓冲化之前，通常需要先消除这类操作，这通常可以通过充分的融合（如我们的情况）来实现，或者运行专用变换：`transform.bufferization.eliminate_empty_tensors` 用于删除仅为定义计算大小而存在的 `tensor.empty` 操作，或 `transform.bufferization.empty_tensor_to_alloc_tensor` 用于为空张量实体化新的临时缓冲区，作为本地缓存使用。

```mlir
// Apply general canonicalization and CSE to each function after
// bufferization as new simplification opportunities may have appeared.
%fb = transform.structured.match ops{["func.func"]} in %arg1
transform.apply_patterns to %fb {
  transform.apply_patterns.canonicalization
}
transform.apply_cse to %fb

// Lower complex, multidimensional vector operations into simpler
// primitives. This particular selection of the pattern groups corresponds
// to vector dialect operations present in the payload IR at this stage.
// Many of these groups can be parameterized to use different strategies or
// lower-level primitives offering performance trade-offs. In this case, we
// are selecting the simplest strategies.
transform.apply_patterns to %fb {
  transform.apply_patterns.vector.lower_contraction
    lowering_strategy = parallelarith
  transform.apply_patterns.vector.lower_transfer
    max_transfer_rank = 1
  transform.apply_patterns.vector.lower_transpose
    lowering_strategy = eltwise
  transform.apply_patterns.vector.lower_shape_cast
}

// These patterns apply in a separate sweep to avoid transfer-to-scf
// patterns overlap with lower-transfer patterns as they apply to the same
// kind of operations. These patterns may produce local allocations to act
// as temporary caches deep inside loops, which could lead to catastrophic
// performance. Such allocations are moved onto the stack and hoisted from
// all the surrounding loops.
transform.apply_patterns to %fb {
  transform.apply_patterns.vector.transfer_to_scf
  transform.apply_patterns.memref.alloc_to_alloca
  }
transform.bufferization.buffer_loop_hoisting %fb

// A final round of cleanups additionally includes patterns to simplify
// buffer aliasing operations that may have been introduced during
// bufferization and could result in excessively complex address
// computation.
transform.apply_patterns to %fb {
  transform.apply_patterns.memref.fold_memref_alias_ops
  transform.apply_patterns.canonicalization
}
transform.apply_cse to %fb
```

由于其过程间特性，单次缓冲化处理整个载荷模块，从而使所有之前创建的句柄失效。因此，它通常是变换序列中的一个较晚步骤，此时不再需要对变换进行精确定向。后续变换通常是模块级或函数级的重写，通常是基于模式的降级。这部分序列可以看作是直接在 transform 方言中指定的 pass 流水线，其中基于模式的降级 pass 从具名模式组_按需构建_。

生成的 IR 可以进一步完全降级到 LLVM 方言，再到 LLVM IR，并由 LLVM 编译器处理以生成可执行文件或进行 JIT 编译。

生成的代码在主频 2.0GHz 的 Intel Skylake 微架构处理器上运行约 420ms。鉴于该计算执行约 $`5 \cdot 80 \cdot 100 \cdot 128 \cdot (2 \cdot 3 \cdot 3 \cdot 128 + 2) \approx 5.9 * 10^9`$ 次浮点运算，达到约 14 GFlops。在 1 个 FMA 单元可用的情况下，测试处理器的单核性能为 64 GFlops（$`16 \cdot 2 \cdot 2 \cdot 10^9`$，其中 16 为向量宽度），因此仅达到理论峰值的 22%。

Halide 生成的代码在同一处理器上运行约 120ms，提升了 3.5 倍，达到峰值的 77%。让我们分析生成的汇编代码以了解性能差距的来源。主要计算工作预计发生在卷积中的浮点乘法和加法处。在两种情况下，汇编代码都以操作 `%zmm` 512 位向量寄存器的 AVX512 `vfma231ps` 指令为特征。在 MLIR 生成的代码中，每次操作前都穿插着加载两个 `fma` 操作数的内存访问，导致延迟增加：

```asm
vmovups       -192(%r10), %zmm0
vbroadcastss  -1536(%rdi,%r9), %zmm1
vmovups       112(%rsp), %zmm2
vfmadd231ps   %zmm1, %zmm0, %zmm2     # zmm2 = (zmm0 * zmm1) + zmm2
vmovups       %ymm2, 112(%rsp)
vextractf64x4 $1, %zmm2, 144(%rsp)
// 19 more blocks of either
//  (a) vmovups,vbroadcast,vfma(z,z),vextract,
//  (b) vbroadcast,vfma(z,mem),vextract
```

然而，Halide 生成的代码具有紧凑的 `vfma231ps` 和 `vbroadcastss` 块，其中一个操作数在加载，而另外两个操作数驻留在寄存器中，在 `fma` 之前加载：

```asm
vbroadcastss    -1536(%rsi,%rbx), %zmm25
vmovups         -192(%rdi), %zmm26
vmovups         -128(%rdi), %zmm27
vmovups         -64(%rdi), %zmm28
vmovups         (%rdi), %zmm29
vfmadd231ps     %zmm25, %zmm26, %zmm24  # zmm24 = (zmm26 * zmm25) + zmm24
vfmadd231ps     %zmm25, %zmm27, %zmm23  # zmm23 = (zmm27 * zmm25) + zmm23
vfmadd231ps     %zmm25, %zmm28, %zmm22  # zmm22 = (zmm28 * zmm25) + zmm22
vfmadd231ps     %zmm25, %zmm29, %zmm21  # zmm21 = (zmm29 * zmm25) + zmm21
vbroadcastss    -1024(%rsi,%rbx), %zmm25
vfmadd231ps     %zmm25, %zmm26, %zmm20  # zmm20 = (zmm26 * zmm25) + zmm20
vfmadd231ps     %zmm25, %zmm27, %zmm19  # zmm19 = (zmm27 * zmm25) + zmm19
vfmadd231ps     %zmm25, %zmm28, %zmm18  # zmm18 = (zmm28 * zmm25) + zmm18
vfmadd231ps     %zmm25, %zmm29, %zmm17  # zmm17 = (zmm29 * zmm25) + zmm17
vbroadcastss    -512(%rsi,%rbx), %zmm25

// 3 more blocks of 4 vfmadd231 followed by a vbroadcast
```

检查 MLIR 产生的逐步中间表示，可以观察到加载（传输）/fma 交错出现在调度应用后的所有层级。重复的张量子集操作（随后转换为向量传输操作）和向量内存加载，是由调度中显式请求的循环展开产生的！问题在于张量（和向量）的单赋值模型，导致访问和更新操作形成冗长而复杂的链，最终变得如此之长，以至于底层变换和下游编译器无法再对其进行简化。实际上，在变换序列早期展开循环可能导致各种与编译器性能相关的问题（包括由于代码过长而导致编译器无法执行某些优化）。

因此，最好在较晚阶段（特别是在缓冲化和相关简化之后）执行循环展开。然而，缓冲化会使所有循环句柄（包括我们希望展开的循环）失效。这个障碍可以通过在缓冲化之后匹配载荷 IR 操作来生成新句柄来克服。我们首先通过在大小为 `[0, 0, 1, 16]` 的分块时使用 `transform.structured.tile_using_forall` 而非 `transform.structured.tile`，将调度中产生的循环类型从 `scf.for` 改为 `scf.forall`，以减少需要匹配的操作数量。然后我们可以匹配载荷 IR 中所有 `scf.forall` 操作，并在_缓冲化之后_将其转换为单迭代器 `scf.for` 循环：

```mlir
%foralls = transform.structured.match ops{["scf.forall"]} in %arg1
%xi_bias, %ci_bias = transform.loop.forall_to_for %xi_ci_bias
%xi_conv, %ci_conv = transform.loop.forall_to_for %xi_ci_conv
%xi_relu, %ci_relu = transform.loop.forall_to_for %xi_ci_relu
%xi_comb, %ci_comb = transform.loop.forall_to_for %xi_ci_comb
```

然后我们可以按需将循环展开变换移到变换序列的后面。将这个新版本编译为汇编代码，产生了与 Halide 版本完全相同的围绕 `vfmadd231ps` 的核心计算，两者仅在分配的寄存器上略有不同。毫不意外，这个版本在同一台机器上运行约 120ms。


## 多维向量的妙用

虽然我们在上一节设法产生了与 Halide 相似的代码，但我们通过在缓冲化之后重新匹配生成的循环来实现这一点，这在一定程度上违背了在 Transform 方言中使用句柄来链接变换的目的。幸运的是，这一步实际上并非必要，它仅作为一个练习来产生期望的循环结构。

多维结构化向量操作通过展开和拆分降级为目标特定的向量。例如，`vector<5x64xf32>` 上的逐元素算术操作被替换为 5 个 `vector<64xf32>` 上的操作，以及用于在 MLIR 层面重建所需类型的额外向量值操作。这些操作中的每一个随后在 LLVM 层面（目标向量宽度信息变得可用的地方）被拆分为 4 个 `vector<16xf32>` 上的操作。总体而言，这与先实体化 5x4 循环嵌套然后完全展开这些循环具有完全相同的效果。因此，调度中的最后一阶段（分块、重匹配和展开）可以从调度中去除。

生成的汇编代码将所有 `vbroadcast` 在 `vfmadd231` 之前分组，但其余结构类似。这种分组是因为每个多维向量操作分别被"展开"。运行时约为 ~110ms，比之前的版本和 Halide 均略微提升了约 8%，达到约 53.7 GFlop/s，即单核峰值性能的 84%。这种改进主要是由于在存在大向量操作的情况下中间表示更短更简洁，从而允许更激进的地址计算和加载放置优化。

最终变换策略已提交到代码库：[mlir/examples/transform/ChH/full.mlir](
https://github.com/llvm/llvm-project/tree/main/mlir/test/Examples/transform/ChH/full.mlir)。
