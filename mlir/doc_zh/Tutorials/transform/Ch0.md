# 第 0 章："结构化" Linalg 操作入门

在开始 Transform 方言教程之前，让我们先简要了解一下结构化操作（Structured operations）的概念及其在 Linalg 方言中的实现。请注意，Transform 方言并不要求使用结构化操作，反之亦然。两者在 Transform 方言早期共同演进，使得针对结构化操作的变换子集最为成熟，也最适合用于本教程。如果你已经熟悉这一概念，可以直接跳至第 1 章。

结构化代码生成的目的是在必要时尽可能长时间地保留计算结构，以支持各种变换，包括设计支持特定变换的 IR 抽象。

## 均匀逐元素扩展

考虑 MLIR 中一个简单的标量算术加法操作，它可以直接映射到大多数支持浮点运算的架构上的机器指令：


```mlir
%2 = arith.addf %0, %1 : f32
```

该操作可以轻松扩展为均匀地应用于一维向量的每个元素，这在向量机上通常也有对应的指令：

```mlir
%2 = arith.addf %0, %1 : vector<8xf32>
```

只有少数现代指令集提供针对二维或更高维向量的指令。然而在 MLIR 中，可以透明地将均匀逐元素应用扩展到任意秩的向量。

```mlir
%2 = arith.addf %0, %1 : vector<8x4xf32>
%5 = arith.addf %3, %4 : vector<2x2x2x2x2x2x2xf32>
```

可以注意到，MLIR 对向量的算术操作保留了均匀逐元素应用的结构。编译器可以利用这一结构，例如生成目标平台上可用的较低秩操作，或者在存在融合乘加指令时进行乘法和加法的融合（当有一百个乘法紧跟一百个加法时，这将变得相当复杂）。

## 归约

有时需要将向量的元素相加以得到一个标量。某些平台为此提供了专用指令，另一些平台则提供可以组合使用的指令来达到相同效果，例如相邻元素相加和元素重排。

MLIR 中的 Vector 方言定义了一个操作来显式表示向量内归约：

```mlir
%1 = vector.reduction <add>, %0 : vector<8xf32> into f32
```

当不支持此类操作时，可以将其转换为一个循环：

```mlir
%c0 = arith.constant 0 : index
%c1 = arith.constant 1 : index
%c8 = arith.constant 8 : index
%init = arith.constant 0.0 : f32
%result = scf.for %i = %c0 to %c8 step %c1 iter_args(%partial = %init) -> (f32) {
  %element = vector.extract %0[%i] : f32 into vector<8xf32>
  %updated = arith.addf %partial, %element : f32
  scf.yield %updated : f32
}
```

即使存在专用指令，根据指令延迟和寄存器压力，使用循环形式（配合展开）有时仍然是更优的选择。将操作结构保留为单一归约，使编译器能够理解正在执行的是向量内归约，从而在实现方式上拥有更多选择。

## 收缩

收缩（Contraction）是归约的推广，它在相加之前将两个向量的元素相乘。简单的"加法"归约可以视为一种收缩，其中一个向量包含乘法的单位元 `1.0`。收缩为编译器提供了更大的灵活性，MLIR 中用专用操作来表示：

```mlir
// 加法的中性初始值。
%init  = arith.constant 0.0 : f32
// 乘法的单位元。
%ones = arith.constant dense<1.0> : vector<8xf32>
// 实际的收缩操作。
%result = vector.contract {
  indexing_maps = [affine_map<(i) -> (i)>,
                   affine_map<(i) -> (i)>,
                   affine_map<(i) -> ()>],
  iterator_types = ["reduction"]
} %0, %ones, %init : vector<8xf32>, vector<8xf32> into f32
```

注意 `affine_map` 表达式表示向量元素的索引方式。将此收缩写成等价的伪代码循环形式，其含义或许最为直观：

```mlir
for i in 0 to 8:
  init += p0[i] * ones[i]
```

其中 `%0` 和 `%ones` 都使用循环归纳变量 `i`，如对应 affine map 右侧所示：`(i) -> (i)`；而 `%init` 不使用，如其 affine map 右侧所示：`(i) -> ()`。

与均匀逐元素扩展类似，MLIR 向量收缩不限于一维情形。在二维及更高维的情况下，可以额外指定哪些向量维度被归约，哪些被保留。这通过 `iterator_types` 属性来实现，该属性为每个维度指定其是被归约（`"reduction"`）还是被保留（`"parallel"`）。考虑下面这个编码矩阵乘法的三维收缩：

```mlir
%result = vector.contract {
  indexing_maps = [affine_map<(i, j, k) -> (i, k)>,
                   affine_map<(i, j, k) -> (k, j)>,
                   affine_map<(i, j, k) -> (i, j)>],
  iterator_types = ["parallel", "parallel", "reduction"]
} %lhs, %rhs, %init: vector<8x10xf32>, vector<10x16xf32> into vector<8x16xf32>
```

查看索引映射，很容易识别出对应的循环形式：

```mlir
for i in 0 to 8:
  for j in 0 to 16:
    for k in 0 to 10:
      init[i, j] += lhs[i, k] * rhs[k, j]
```

保留收缩的这种高层结构，使编译器更容易识别矩阵乘法和点积等操作，并可以自由生成利用最先进指令甚至预生成微内核的底层操作。

## 内存上的通用操作

到目前为止，我们一直在考虑存储于虚拟寄存器中的向量操作。类似的收缩抽象也可以定义在内存上：

```mlir
linalg.generic {
  indexing_maps = [affine_map<(i, j, k) -> (i, k)>,
                   affine_map<(i, j, k) -> (k, j)>,
                   affine_map<(i, j, k) -> (i, j)>],
  iterator_types = ["parallel", "parallel", "reduction"]
} ins(%lhs, %rhs : memref<8x10xf32>, memref<10x16xf32>)
  outs(%init : memref<8x16xf32>) {
^bb0(%lhs_one: f32, %rhs_one: f32, %init_one: f32):
  %0 = arith.mulf %lhs_one, %rhs_one : f32
  %1 = arith.addf %init_one, %0 : f32
  linalg.yield %1 : f32
}
```

这看起来更复杂，让我们逐一解析。`indexing_maps` 和 `iterator_types` 与上面向量收缩中所见的_完全_相同。操作数现在被分成两个列表：


*   `in` 操作数：包含只被该操作读取的缓冲区；
*   `out` 操作数：被该操作读取并更新。

这种区分在向量上是不必要的，因为在 MLIR 中，向量是只读的（SSA 或函数式形式），修改向量的操作实际上是产生一个新的向量。

此外，该操作现在包含一个 region，显式指定了在收缩中隐式包含的乘法和加法操作。region 中的块参数对应于从缓冲区读取的各个元素：前两个对应于 `in` 操作数，最后一个对应于 `out` 操作数。从 region 中 yield 出来的值被"写入" `out` 操作数，并作为最后一个块参数在 region 的后续执行中可用。请注意，region 针对各元素元组的执行顺序未作规定，对 `out` 缓冲区的写入作为整体在操作结束时完成。

## "循环"融合

由于 `linalg.generic` 操作的 region 可以包含任意多个操作，我们可以通过在 region 中链接更多操作来表达隐式循环的"融合"。例如，常见的机器学习修正线性单元层（ReLU），可定义为 `relu(x) = max(0, x)`，可以用一个 `linalg.generic` 操作中的"比较-选择"惯用方式来表达，无需为比较结果创建临时缓冲区，也无需重复外部操作：

```mlir
linalg.generic {
  indexing_maps [affine_map<(i) -> (i)>, affine_map<(i) -> (i)>],
  iterator_types = ["parallel"]
} ins(%in : memref<?xf32>) outs(%out : memref<?xf32>) {
^bb0(%in_one : f32, %out_one : f32):
  %c0 = arith.constant 0.0 : f32
  %0 = arith.cmpf ogt %in_one, %c0 : f32
  %1 = arith.select %0, %in_one, %c0 : f32
  linalg.yield %1 : f32
}
```

此类操作可以转换为循环，也可以在拆分为多个操作（每个操作映射到一个 Vector 方言原语）后降级为向量形式。这种建模方式再次赋予编译器在选择代码生成策略时更多自由。

## 张量上的通用操作

让我们在抽象层次上再迈一步。MLIR 提供了张量（tensor）抽象，使编译器更容易对多维但规则的数据进行推理，而无需解决多维缓冲区上所必需的别名分析和依赖满足等复杂问题。张量抽象与向量抽象非常相似（主要区别包括：支持无秩张量、张量布局，以及向量可以作为张量的元素类型但不能作为其他向量的元素类型）。张量是只读的，更新张量的操作会产生一个新张量。

上面的 `linalg.generic` 操作可以提升为操作张量而非缓冲区：

```mlir
%result = linalg.generic {
  indexing_maps = [affine_map<(i, j, k) -> (i, k)>,
                   affine_map<(i, j, k) -> (k, j)>,
                   affine_map<(i, j, k) -> (i, j)>],
  iterator_types = ["parallel", "parallel", "reduction"]
} ins(%lhs, %rhs : tensor<8x10xf32>,tensor<10x16xf32>)
  outs(%init :tensor<8x16xf32>) {
^bb0(%lhs_one: f32, %rhs_one: f32, %init_one: f32):
  %0 = arith.mulf %lhs_one, %rhs_one : f32
  %1 = arith.addf %init_one, %0 : f32
  linalg.yield %1 : f32
} -> tensor<8x16xf32>
```

可以注意到，该操作的大多数组件与其缓冲区版本保持一致，这是经过专门设计的。除操作数类型外，主要区别在于该操作现在产生一个新结果，而不是更新 `out` 缓冲区。`out` 操作数仅用作初始化值。

如果 `linalg.generic` 操作存在向量版本，它将具有完全相同的结构。

## 分块与循环实体化

在这一抽象层次上，编译器可以轻松地执行高性能代码生成通常所需的更高级变换，例如[分块（tiling）](https://en.wikipedia.org/wiki/Loop_nest_optimization)。分块通常可以理解为将迭代空间划分为更小的部分（即分块），使每个部分所需的数据能够放入某级缓存中。各分块的执行顺序必须保持原始数据依赖关系。

对于 `linalg.generic` 操作，迭代空间是隐式的，由操作数的形状定义。因此，一个分块可以通过对原始数据的一个子集（切片）执行_相同_的操作来表达。由于 `linalg.generic` 的主体应用于不同输入元素元组的顺序未作规定，各分块可以以任意顺序执行，无需依赖分析。为了控制不同分块的执行，分块的实现会产生循环。因此，对 `linalg.generic` 操作进行分块也可以理解为将迄今为止一直隐式存在的循环实体化。

例如，以分块大小 `(2, 8)` 对上面的矩阵乘法进行分块，我们将在一个 `linalg.generic` 外围得到一个循环嵌套，该操作在一个 `2x8` 张量上表达相同的运算。

```mlir
// 一种特殊的"多重循环"，支持张量插入语义
// 而不是隐式更新。最终的 8x16 张量由
// 这个循环产生。
// 迭代器的行程计数通过将原始张量大小
// 8x16 除以分块大小 2x8 来计算，得到 4x2。
// 当张量大小是动态的时，行程计数计算会作为 IR 生成
// 并在运行时计算。
%0 = scf.forall (%i, %j) in (4, 2)
     shared_outs(%shared = %init) -> (tensor<8x16xf32>) {

  // 将循环归纳变量乘以分块大小。
  %3 = affine.apply affine_map<(d0) -> (d0 * 2)>(%i)
  %4 = affine.apply affine_map<(d0) -> (d0 * 8)>(%j)

  // 提取输入和输出的切片。只有 "i" 和 "j" 维度被切片。
  %lhs_slice = tensor.extract_slice %lhs[%3, 0] [2, 10] [1, 1]
             : tensor<8x10xf32> to tensor<2x10xf32>
  %rhs_slice = tensor.extract_slice %rhs[0, %4] [10, 8] [1, 1]
             : tensor<10x16xf32> to tensor<10x8xf32>
  %result_slice = tensor.extract_slice %shared[%3, %4] [2, 8] [1, 1]
                : tensor<8x16xf32> to tensor<2x8xf32>

  // 这与之前完全相同的操作，但现在对较小的
  // 数据切片进行操作。
  %partial =  linalg.generic {
  indexing_maps = [affine_map<(i, j, k) -> (i, k)>,
                   affine_map<(i, j, k) -> (k, j)>,
                   affine_map<(i, j, k) -> (i, j)>],
  iterator_types = ["parallel", "parallel", "reduction"]
  } ins(%lhs_slice, %rhs_slice : tensor<2x10xf32>, tensor<10x8xf32>)
    outs(%result_slice : tensor<2x8xf32>) -> tensor<2x8xf32> {
  ^bb0(%lhs_one: f32, %rhs_one: f32, %init_one: f32):
    %0 = arith.mulf %lhs_one, %rhs_one : f32
    %1 = arith.addf %init_one, %0 : f32
    linalg.yield %1 : f32
  } : tensor<2x8xf32>

  // 具有张量插入语义的循环终止符。将切片插入
  // 更大的张量中，可能是并行进行的。
  scf.forall.in_parallel {
    tensor.parallel_insert_slice %partial into %shared[%3, %4] [2, 8] [1, 1]
        : tensor<2x8xf32> into tensor<8x16xf32>
  }
}
```

## 生产者/消费者融合与重新物化

在通过分块实体化循环之后，另一个关键的代码生成变换变得简单——融合。与循环融合不同，结构化操作方法允许即使在操作的（隐式）迭代空间不匹配的情况下也能进行生产者/消费者融合。给定张量上的高层结构化操作（如 `linalg.generic`），可以沿着使用-定义链来识别：

1. 该分块所使用的操作数子集（切片），以及
2. 产生被切片的整个张量的张量级结构化操作。

通过对 `indexing_map` 求逆并将其应用于通过切片访问的元素集合，我们可以计算出定义完整张量的操作的迭代空间中计算该分块所必需的部分。因此，融合就归结为用产生原始操作数的 `linalg.generic` 的分块来替换 `tensor.extract_slice` 操作。

假设矩阵乘法操作之后还有另一个操作，将结果矩阵的每个元素与自身相乘。这个后续的逐元素操作具有二维迭代空间，而矩阵乘法是三维的。尽管如此，仍然可以对该后续操作进行分块，然后将其操作数的生产者（矩阵乘法）融合到由分块生成的循环中。未被分块的维度将被完整使用。


```mlir
// 与之前相同的循环。
%0 = scf.forall (%i, %j) in (4, 2)
     shared_outs(%shared = %init)
     -> (tensor<8x16xf32>, tensor<8x16xf32>) {
  // 将循环归纳变量乘以分块大小。
  %1 = affine.apply affine_map<(d0) -> (d0 * 2)>(%i)
  %2 = affine.apply affine_map<(d0) -> (d0 * 8)>(%j)

  // 提取输入和输出的切片。只有 "i" 和 "j" 维度被切片。
  %lhs_slice = tensor.extract_slice %lhs[%1, 0] [2, 10] [1, 1]
             : tensor<8x10xf32> to tensor<2x10xf32>
  %rhs_slice = tensor.extract_slice %rhs[0, %2] [10, 8] [1, 1]
             : tensor<10x16xf32> to tensor<10x8xf32>
  %result_slice = tensor.extract_slice %result[%1, %2] [2, 8] [1, 1]
                : tensor<8x16xf32> to tensor<2x8xf32>

  // 这与之前完全相同的矩阵乘法切片。它替代了
  // 下面 generic 操作的切片提取。
  %partial = linalg.generic {
    indexing_maps = [affine_map<(i, j, k) -> (i, k)>,
                     affine_map<(i, j, k) -> (k, j)>,
                     affine_map<(i, j, k) -> (i, j)>],
    iterator_types = ["parallel", "parallel", "reduction"]
  } ins(%lhs_slice, %rhs_slice : tensor<2x10xf32>, tensor<10x8xf32>)
   outs(%result_slice : tensor<2x8xf32>) {
  ^bb0(%lhs_one: f32, %rhs_one: f32, %init_one: f32):
    %5 = arith.mulf %lhs_one, %rhs_one : f32
    %6 = arith.addf %init_one, %5 : f32
    linalg.yield %6 : f32
  } -> tensor<2x8xf32>

  // 提取最终结果的切片。注意我们不需要提取
  // 操作数的切片，因为上面的矩阵乘法操作已经
  // 就地计算了它。
  %shared_slice = tensor.extract_slice %shared[%1, %2] [2, 8] [1, 1]
                : tensor<8x16xf32> to tensor<2x8xf32>

  // 我们分块的逐元素操作。
  %elemwise = linalg.generic {
    indexing_maps = [affine_map<(i, j) -> (i, j)>,
                     affine_map<(i, j) -> (i, j)>],
    iterator_types = ["parallel", "parallel"]
  } ins(%partial : tensor<2x8xf32>)
   outs(%shared_slice : tensor<2x8xf32>) {
  ^bb0(%in: f32, %out: f32):
    %5 = arith.mulf %in, %in : f32
    linalg.yield %5 : f32
  } -> tensor<2x8xf32>

  // 具有张量插入语义的循环终止符。将切片插入
  // 更大的张量中，可能是并行进行的。
  scf.forall.in_parallel {
    tensor.parallel_insert_slice %elemwise into %shared[%1, %2] [2, 8] [1, 1]
        : tensor<2x8xf32> into tensor<8x16xf32>
  }
}
```

这个过程可能导致操作数张量中的某些元素在循环的每次迭代中被（重新）计算。这也称为_重新物化（rematerialization）_，表达了执行冗余计算与将其结果存储在（较慢的）内存中之间的权衡。

## Linalg 操作的简写"具名"形式

Linalg 为矩阵乘法、点积、卷积等常见情况提供了一组预定义操作。这些操作等价于 `generic` 操作，但省去了拼写访问模式和主体的需要。例如，矩阵乘法可以简单地表示为：

```mlir
%matmul = linalg.matmul ins(%lhs, %rhs: tensor<8x10xf32>, tensor<10x16xf32>)
                        outs(%init: tensor<8x10xf32xf32>) -> tensor<8x16xf32>
```
