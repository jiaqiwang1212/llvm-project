# 'vector' 方言（Dialect）

**在向本方言添加任何操作之前，请先在[论坛](https://llvm.discourse.group/c/mlir/31)上发布 RFC。**

[TOC]

MLIR 支持多维 `vector` 类型以及对这些类型的自定义操作。一种通用的、可重定向的、高阶的 `vector` 类型（`n-D`，其中 `n > 1`）是一种结构化类型，携带对变换有用的语义信息。本文档讨论了 MLIR 中现有的可重定向抽象，它们作用于类型为 `vector` 的 SSA（静态单赋值）值，以及支持针对具体目标上的特定指令的模式重写和降低（lowering）。这些抽象用于分离 `memref`（即缓冲区）上的操作与 `vector` 值上的操作之间的关注点。这不是一个新提案，而是对现有 MLIR 组件的文字化文档，并附带设计原理说明。

## 在代码生成基础设施中的定位

以下图表（近期随
[StructuredOps abstractions](https://drive.google.com/corp/drive/u/0/folders/1sRAsgsd8Bvpm_IxREmZf2agsGU2KvrK-)
一同展示）展示了 MLIR 中在各种现有降低路径上实现的当前代码生成路径。
![](https://user-images.githubusercontent.com/10148468/71177417-f78e4d80-2239-11ea-92ef-700f42ea503f.png)

以下图表旨在将 `vector` 方言从代码生成路径的复杂性中隔离出来，专注于作用于 std 和 `vector` 类型的承载载荷的算子。此图表并非一成不变，也不代表当前存在的内容，而是展示 MLIR 中抽象层次的分层方式。

![MLIR 中的 `vector` 抽象](https://user-images.githubusercontent.com/10148468/71176949-e85ad000-2238-11ea-9806-200843bc4943.png)

这将以下关注点分离：(a) 在 `vector` 类型上定义高效操作；(b) 对 `memref`、循环和其他类型的结构化算子（无论是 `HLO`、`LHLO`、`Linalg` 还是其他）进行程序分析与变换。稍微展望未来，我们可以大胆预测：从代码生成（或某种用户/语言级别）构建和作为目标的 `vector` 级原语层级越高，我们的任务就越简单，可以表达更复杂的模式，性能也会越好。

## 通用可重定向向量级方言的组成部分

现有的 MLIR `vector` 级方言与以下自底向上的抽象相关：

1.  通过数据结构、指令和内置函数在 `LLVMIR` 中的表示。这被称为 `LLVM` 层。
2.  一组与硬件 ISA 几乎一一对应的、针对特定机器的操作和类型。这被称为硬件向量（Hardware Vector）层，简称 `HWV`。例如，(a) 用于 `CUDA` 的 `NVVM` 方言（含张量核心算子），(b) 特定加速器的方言（内部），一个潜在的（未来的）`CPU` 方言，用于更紧密地捕获 `LLVM` 内置函数，以及其他针对特定硬件的方言。理想情况下，这应尽可能从 `LLVM` 层自动生成。
3.  一组虚拟的、与机器无关的操作，这些操作根据 `HWV` 层的成本信息来定义。这被称为虚拟向量（Virtual Vector）层，简称 `VV`。这是更高层抽象（代码生成、自动向量化、潜在的向量语言等）所针对的层次。

现有的通用可重定向 `vector` 级方言与以下自顶向下的重写和转换相关：

1.  由 MLIR `PatternRewrite` 基础设施应用的 MLIR 重写模式，逐步降低到越来越接近 `HWV` 的实现。一些模式是"方言内"的 `VV -> VV`，另一些是转换 `VV -> HWV`。
2.  `Virtual Vector -> Hardware Vector` 的降低被指定为一组 MLIR 降低模式，目前是手动指定的。
3.  `Hardware Vector -> LLVM` 的降低是一个机械性过程，目前是手动编写的，应该实现自动化，尽可能紧密地遵循 `LLVM -> Hardware Vector` 算子生成的方式。

## 现有基础设施简介

### LLVM 层

在 CPU 上，`n-D` `vector` 类型当前降低到 `!llvm<array<vector>>`。
更具体地说：
* `vector<4x8x128xf32>` 降低到 `!llvm<[4 x [ 8 x < 128 x float >]]>`（固定宽度向量），
* `vector<4x8x[128]xf32>` 降低到 `!llvm<[4 x [ 8 x < vscale x 128 x float >]]>`（可伸缩向量）。

在如何访问子向量以及如何使用 `llvm.extractelement`、`llvm.insertelement` 和 `llvm.shufflevector` 方面存在权衡。[LLVM 降低权衡](#llvm-lowering-tradeoffs)一节对当前设计选择和权衡进行了深入探讨。

注意，虽然 LLVM 支持可伸缩向量的数组，但这些数组必须是一维可伸缩向量的固定宽度数组。这意味着具有非末尾可伸缩维度的可伸缩向量（例如 `vector<4x[8]x128xf32>`）无法转换为 LLVM。

最后，MLIR 与 LLVM 对可伸缩向量持相同的观点（参见
[VectorType](https://llvm.org/docs/LangRef.html#vector-type)）：
> 对于可伸缩向量，元素的总数是指定元素数量的常数倍（称为 vscale）；vscale 是一个在编译时未知的正二次幂整数，在运行时对所有可伸缩向量而言是相同的硬件相关常数。因此，特定可伸缩向量类型的大小在 IR 中是常数，即使字节的确切大小直到运行时才能确定。

### 硬件向量算子

硬件向量算子（Hardware Vector Ops）以每个目标一个方言的形式实现。对于内部硬件，我们正在自动生成特定的硬件方言。对于 `GPU`，`NVVM` 方言添加了诸如 `mma.sync`、`shfl` 等操作和测试。对于 `CPU`，情况有些不稳定，因为抽象接近 `LLVMIR`。目前尚无定论是否具体需要一个通用的 `CPU` 方言，但为所有目标设置相同的抽象层次，并在 MLIR 中为 `LLVM` 执行基于成本的降低决策，似乎是合理的。能够捕获 LLVM 窥孔优化或 MLIR 核心支持的不同类型（例如可伸缩向量）未能很好地捕获的特定特性的专用 `CPU` 方言是受欢迎的未来扩展。

### 虚拟向量算子

一些作用于 `n-D` `vector` 类型的现有 Arith 和 Vector 方言算子包括：

```mlir
// Produces a vector<3x7x8xf32>
%a = arith.addf %0, %1 : vector<3x7x8xf32>
// Produces a vector<3x7x8xf32>
%b = arith.mulf %0, %1 : vector<3x7x8xf32>
// Produces a vector<3x7x8xf32>
%c = vector.broadcast %1 : f32 to vector<3x7x8xf32>

%d = vector.extract %0[1]: vector<7x8xf32> from vector<3x7x8xf32>
%e = vector.extract %0[1, 5]: vector<8xf32> from vector<3x7x8xf32>
%f = vector.outerproduct %0, %1: vector<4xf32>, vector<8xf32>      // -> vector<4x8xf32>
%g = vector.outerproduct %0, %1, %2: vector<4xf32>, vector<8xf32>  // fma when adding %2

// Returns a slice of type vector<2x2x16xf32>
%h = vector.strided_slice %0
    {offsets = [2, 2], sizes = [2, 2], strides = [1, 1]}:
  vector<4x8x16xf32>

%i = vector.transfer_read %A[%0, %1]
    {permutation_map = (d0, d1) -> (d0)}:
  memref<7x?xf32>, vector<4xf32>

vector.transfer_write %f1, %A[%i0, %i1, %i2, %i3]
    {permutation_map = (d0, d1, d2, d3) -> (d3, d1, d0)} :
  vector<5x4x3xf32>, memref<?x?x?x?xf32>
```

Vector 列表目前正在演进，最好通过跟踪
[VectorOps.td](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/Vector/IR/VectorOps.td)
ODS 文件的演进来了解（Markdown 文档在本地构建时自动生成，并填充到
[Vector doc](https://github.com/llvm/llvm-project/blob/main/mlir/docs/Dialects/Vector.md) 中）。
近期的扩展由具体的感兴趣用例驱动。一个值得注意的用例是 `vector.contract` 算子，它将 StructuredOps 抽象的原则应用于 `vector` 类型。

### 虚拟向量重写模式

以下重写模式存在于 `VV->VV` 层：

1.  现已退役的 `MaterializeVector` 通道（pass）曾通过展开，将粗粒度虚拟 `vector` 上的算子合法化为细粒度虚拟 `vector`。这已被重写为作用于 `vector` 算子和 `vector` 类型的、可重定向的展开-并合（unroll-and-jam）模式。
2.  `vector_transfer` 算子的降低将 `vector` 加载/存储算子合法化为标量加载/存储上的置换循环。随着 `mask` 操作作为 `VV` 层的 `vector` 算子变得可用，这应该演进为 `vector` 加载/存储上的循环加上 `mask` 操作。

总体方向是添加更多虚拟向量层算子，并实现更多有用的 `VV -> VV` 重写，作为 PatternRewrite 基础设施可以迭代应用的可组合模式。

### 虚拟向量到硬件向量的降低

目前，`VV -> HWV` 用 C++ 指定（例如参见
[VectorOuterProductOp lowering](https://github.com/tensorflow/mlir/commit/957b1ca9680b4aacabb3a480fbc4ebd2506334b8)）。

从虚拟向量层开始，针对 `LLVM` 目标的简单
[转换测试](https://github.com/llvm/llvm-project/blob/main/mlir/test/Conversion/VectorToLLVM/vector-to-llvm.mlir)
已可用。

## 设计原理

### 硬件作为最小粒度的 `vector` 机器

高维 `vector` 在现代高性能计算硬件中无处不在。思考通用可重定向 `vector` 级方言的一种方式是：它作用于硬件向量大小整数倍的 `vector` 类型，以便硬件能够高效地实现一组高层原语（例如，当硬件 `vector` 大小为 `vector<4x8xf32>` 时，使用 `vector<8x8x8x16xf32>`）。

一些值得关注的 `vector` 大小包括：

1.  CPU：`vector<HW_vector_size * k>`、`vector<core_count * k' x HW_vector_size * k>` 和 `vector<socket_count x core_count * k' x HW_vector_size * k>`
2.  GPU：`vector<warp_size * k>`、`vector<warp_size * k x float4>` 和针对张量核心大小的 `vector<warp_size * k x 4 x 4 x 4>`，
3.  其他加速器：n-D `vector` 作为硬件中的一等公民。

根据目标的不同，非硬件 `vector` 大小整数倍的大小上的操作，可能产生慢速代码（例如通过 `LLVM` 合法化），或根本无法合法化（例如某些不支持的加速器 X 操作和类型组合）。

### 避免的变换问题

一个 `vector<16x32x64xf32>` 虚拟 `vector` 是一种粗粒度类型，可以被"展开"到特定于硬件的大小。多维展开因子由 IR 中的 `vector` 类型携带。展开后，可以运行传统的指令级调度。

通过在 `vector` `ssa-值`抽象上操作，以下关键变换（连同支持的分析和结构约束）被完全避免：

1.  循环展开与展开-并合。
2.  用于寄存器重用的循环和加载/存储重构。
3.  加载到存储的转发与内存到寄存器（Mem2reg）。
4.  从细粒度 `vector` 形式的粗化（提升）。

注意，在 `vector` 上下文中的"展开"对应于部分循环展开-并合，而非完全展开。因此，在适用情况下，这预期与软件流水线（SW pipelining）组合使用，不会导致指令缓存（ICache）膨胀。

### 超出范围的重要部分：自动向量化

这里未讨论的一个重要部分是自动向量化（自动从标量提升到 n-D `vector` 算子和类型）。简而言之，当第一个"超向量化"原型实现时，MLIR 远不如今天成熟。随着我们继续在 `VV -> HWV` 中构建更多抽象，有机会重新审视 MLIR 中的向量化。

由于此话题涉及代码生成抽象，从技术上讲超出了本调查文档的范围，但在结构化算子类型表示以及向量化变换如何跨方言重用的背景下，有很多值得讨论的内容。特别是，MLIR 允许以任意粒度定义方言，并且有利于渐进式降低。可以论证，循环+算子抽象上的自动向量化类似于提升已丢失的结构信息。相反，可以将向量化重新视为简单的模式重写，前提是 IR 处于合适的形式。例如，对语义匹配 `matmul` 的 `linalg.generic` 算子进行向量化，可以[通过模式相当容易地完成](https://github.com/tensorflow/mlir/commit/bff722d6b59ab99b998f0c2b9fccd0267d9f93b5)。事实上，在以 `vector.contract` 算子为目标时，这个模式对于任何类型的收缩都很容易推广，以及对任何域（`+/*`、`min/+`、`max/+`、`or/and`、`logsumexp/+` 等）。换言之，通过在比仿射循环更高层次的通用抽象上操作，非平凡的变换变得更加简单，并且可以以更细的粒度组合。

无论自动向量化器是否存在，都可以基于 VectorOps 方言构建一个名义上的向量语言，并通过直接在 IR 中表达 `vector` 以及简单的模式重写来构建端到端模型。
[EDSC](https://github.com/llvm/llvm-project/blob/main/mlir/docs/EDSC.md)
提供了一种在 C++ 中直接驱动这种名义语言的简单方式。

## 命名讨论

将 n-D 抽象层次命名为 `vector` 存在反对意见，因为大多数人将其与一维 `vector` 联系在一起。另一方面，在 MLIR 中 `vector` 是一等的 n-D 值。有人提出了替代名称 Tile，它传达了更高维的含义。但它也是编译器和硬件中最过载的术语之一。目前，我们通常使用 `n-D` `vector` 这一名称，并欢迎更好的建议。

## 零维向量（0D Vectors）

零维向量（即 *0-D 向量*或 *0D 向量*）在 MLIR 中是允许的。例如，一个包含一个标量的 `f32` 向量可以表示为 `vector<f32>`。这类似于 TensorFlow 中可用的 `tensor<f32>` 类型或 MLIR 中可用的 `memref<f32>` 类型。

通常，0D `vector` 可以解释为一个标量。0D `vector`、`tensor` 和 `memref` 的好处在于，它们使从各种前端（如 TensorFlow）降低代码变得更容易，也使处理边界情况（如将循环从一维展开到零维）变得更容易。

## LLVM 降低权衡

本节描述将 MLIR n-D 向量类型及其上的操作降低到 LLVM-IR 所涉及的权衡。暂时搁置
[LLVM Matrix](http://lists.llvm.org/pipermail/llvm-dev/2018-October/126871.html)
提案，这里假设 LLVM 仅内置支持一维向量。与 LLVM Matrix 提案的关系在本文档末尾讨论。

LLVM 指令以 `llvm.` 方言前缀标注（例如 `llvm.insertvalue`）。这些算子仅按照 [LLVM LangRef](https://llvm.org/docs/LangRef.html) 对一维向量和聚合类型进行操作。MLIR 操作以 `vector.` 方言前缀标注（例如 `vector.insert`）。这些算子仅对 MLIR `n-D` `vector` 类型进行操作。

### 将 n-D 向量类型降低到 LLVM 的替代方案

考虑一个秩为 n、静态大小为 `{s_0, ... s_{n-1}}` 的向量（即 MLIR `vector<s_0x...s_{n-1}xf32>`）。将这种 `n-D` MLIR 向量类型降低到 LLVM 描述符可以通过以下方式之一实现：

1.  一维向量的嵌套聚合类型：MLIR LLVM 方言中的 `!llvm."[s_0x[s_1x[...<s_{n-1}xf32>]]]">`（MLIR 中的当前降低方式）。
2.  展平为一维向量：MLIR LLVM 方言中的 `!llvm<"(s_0*...*s_{n-1})xfloat">`。
3.  两者的混合。

在选择其中一种或另一种时涉及多种权衡，我们将进行讨论。重要的是要注意，"两者的混合"立即归约为"一维向量的嵌套聚合类型"，再加上一个 `vector.cast %0: vector<4x8x16x32xf32> to vector<4x4096xf32>` 操作，该操作展平最"末尾的 k"个维度。

### 从 LLVM 继承的约束（参见 LangRef）

第一个约束已提及：LLVM 原生只支持一维 `vector` 类型。额外的约束与 LLVM 中向量和
[聚合类型](https://llvm.org/docs/LangRef.html#aggregate-types)
的区别有关：
> 聚合类型是派生类型的子集，可以包含多个成员类型。数组和结构是聚合类型。向量不被视为聚合类型。

这种区别也反映在一些操作中。对于一维向量，操作 `llvm.extractelement`、`llvm.insertelement` 和 `llvm.shufflevector` 适用，并且直接支持动态索引。对于 `n>1` 的 n-D 向量，因此在 LLVM 层是聚合类型，更受限制的操作 `llvm.extractvalue` 和 `llvm.insertvalue` 适用，它们只接受静态索引。聚合类型没有直接的混洗支持。

以下语句（参见 LangRef [结构体类型](https://llvm.org/docs/LangRef.html#structure-type)）说明了一个反复出现的权衡，在 MLIR 中也存在，即"值类型"（受 SSA 使用-定义链约束）和"内存类型"（受别名和副作用约束）之间的权衡：
> 内存中的结构通过使用 llvm.getelementptr 指令获取字段指针，用 'load' 和 'store' 进行访问。寄存器中的结构使用 llvm.extractvalue 和 llvm.insertvalue 指令进行访问。

将此转换到 MLIR 时，`llvm.getelementptr` 作用于内存中 n-D 向量的指针。对于存在于寄存器中的 n-D 向量值，我们可以使用 `vector.extract` 和 `vector.insert`，它们不接受动态索引。注意，这与下面讨论的硬件考量是一致的。

另一种方法是使用 LLVM 一维 `vector` 类型，对此可以使用 `llvm.extractelement`、`llvm.insertelement` 和 `llvm.shufflevector`。这些操作接受动态索引。其含义是必须使用将 MLIR n-D 向量展平为 LLVM 一维向量的降低方式。

涉及多种权衡，混合了对编程模型、在实际硬件上执行以及代码生成可见或隐藏的影响。这些将在以下各节中讨论。

### 嵌套聚合

优点：

1.  将 n-D 向量自然编码为一维向量上的 (n-1)-D 聚合。
2.  不需要到处插入线性化/反线性化逻辑。
3.  `llvm.insertvalue`、`llvm.extractvalue` 对 `(n-k)-D` 聚合是自然的。
4.  对一维向量类型的 `llvm.insertelement`、`llvm.extractelement`、`llvm.shufflevector` 是自然的。

缺点：

1.  `llvm.insertvalue` / `llvm.extractvalue` 不接受动态索引，只接受静态索引。
2.  在非最末尾维度上的动态索引需要来回经过内存。
3.  LLVM 中的特殊内置函数和原生指令作用于一维向量。由于有 `vector.cast %0: vector<4x8x16x32xf32> to vector<4x4096xf32>` 操作（展平最末尾的维度），这预计不会成为实际限制（见对代码生成影响中的大局）。

### 展平的一维向量类型

优点：

1.  可以在整个降低后的 n-D 向量类型上使用动态索引进行 `insertelement` / `extractelement` / `shufflevector`。
2.  支持特殊内置函数和原生操作。

缺点：

1.  需要到处使用线性化/反线性化逻辑，翻译复杂。
2.  通过动态索引将真实的硬件结构隐藏起来：归根结底，硬件向量大小通常是固定的，需要多个向量来保存大于硬件大小的向量。
3.  窥孔优化不太可能产生良好代码：任意动态访问，特别是在硬件向量边界处，不太可能产生规则模式。

### 讨论

#### 硬件向量及其对软件和编程模型的影响

截至今日，LLVM 模型只支持一维向量类型。这并不奇怪，因为从历史上看，绝大多数硬件只支持一维向量寄存器。我们注意到，多个硬件供应商正在向更高维的物理向量演进。

在以下讨论中，假设硬件向量大小为一维，软件向量大小为 n-D，其中 `n >= 1`。对于二维硬件 `vector` 大小和 `n >= 2`，同样的讨论也适用。在这种情况下，大多数硬件具有一个向量寄存器文件，此类向量的数量是固定的。根据软件向量抽象的秩和大小以及硬件向量大小和寄存器数量，一个 n-D 软件向量类型在给定时刻可能由多个一维硬件向量寄存器加内存位置的混合来实例化。

物理硬件约束对编程模型的影响是：不能跨硬件寄存器进行动态索引：寄存器文件通常不能被动态索引。这是因为寄存器号是固定的，要么需要显式展开以获得固定的寄存器号，要么需要经过内存。这是 CUDA 程序员熟悉的约束：当声明 `private float a[4]` 并随后使用*动态*值进行索引时，会产生所谓的**本地内存**使用（即来回经过内存）。

#### 对代码生成的影响

MLIR n-D 向量类型在降低到 LLVM 时，当前被表示为一维向量的 (n-1)-D 数组。这引入了前面讨论的静态与动态索引的后果：MLIR 中 n-D 向量上的 `extractelement`、`insertelement` 和 `shufflevector` 只支持静态索引。动态索引仅在最末尾的一维向量上受支持，而非外层的 (n-1)-D。对于其他情况，需要显式的加载/存储。

对代码生成的影响如下：

1.  围绕 `vector` 值的循环是向量值的间接寻址，它们必须对 n-D 向量类型进行显式的加载/存储操作。
2.  一旦 n-D `vector` 类型被加载到 SSA 值中（当最终降低时，该值可能住在 `n` 个寄存器中，也可能不住在，有或没有溢出），它可能被展开为与硬件对应的更小的 k-D `vector` 类型和操作。这个 MLIR 代码生成层次与寄存器分配和溢出相关，这些在 LLVM 流水线的后期才发生。
3.  硬件可能支持 >1-D 向量，并具有用于这些向量内间接寻址的内置函数。这可以通过从 MLIR k-D 向量类型和操作到 LLVM 一维向量及作用于一维 `vector<Kxf32>` 的内置函数的显式 `vector_cast` 操作来实现。

或者，我们认为直接降低到线性化抽象通过给人一种跨寄存器魔法动态索引的虚假印象，隐藏了与内存访问相关的代码生成复杂性。相反，我们更愿意在 MLIR 中使这些内容非常明确，并允许代码生成探索权衡。不同的硬件将在步骤 1、2 和 3 所涉及的大小上要求不同的权衡。

在 MLIR 层做出的决定将在 LLVM 的更晚阶段（寄存器分配之后）产生影响。我们不打算向 MLIR 明确暴露与寄存器分配和溢出建模相关的关注点。相反，每个目标将公开一组"良好的"目标操作和 n-D 向量类型，以及 MLIR 层的 `PatternRewriter` 可以作为目标的成本。MLIR 层的此类成本将是抽象的，用于排序，而非用于精确的性能建模。将来，此类成本将被学习。

#### 对降低到加速器的影响

要针对原生支持高维向量的加速器，我们可以从 MLIR 中的一维或 n-D 向量开始，并使用 `vector.cast` 将最末尾的维度展平为一维 `vector<Kxf32>`，其中 `K` 是适当的常数。然后，现有的到 LLVM-IR 的降低立即适用，并带有针对加速器特定内置函数的扩展。

降低 `vector.cast` 是特定于加速器的向量方言（见上图中的代码生成流程）的职责。加速器 -> LLVM 降低将由一系列 `Accelerator -> Accelerator` 重写（执行转换）与 `Accelerator -> LLVM` 转换加上作用于一维 `vector<Kxf32>` 的内置函数组成。

其中一些重写可能需要额外的处理，特别是在涉及归约的情况下。例如，当 `K != K1 * … * Kn` 时的 `vector.cast %0: vector<K1x...xKnxf32> to vector<Kxf32>`，以及某些任意不规则的 `vector.cast %0: vector<4x4x17xf32> to vector<Kxf32>`，可能会引入掩码和向量内混洗，这可能不值得甚至不可行，即具有无限代价。

然而，当 `K = K1 * … * Kn` 时，`vector.cast %0: vector<K1x...xKnxf32> to vector<Kxf32>` 应接近于空操作（noop）。

随着我们开始构建特定于加速器的抽象，我们希望实现可重定向的代码生成：相同的基础设施通过额外的 MLIR 模式和成本用于 CPU、GPU 和加速器。

#### 对调用作用于向量的外部函数的影响

在调用外部函数时，可能（很可能）还需要进行线性化。

### 与 LLVM matrix 类型提案的关系

LLVM matrix 提案在大约一年前提出，但似乎在最近才重新活跃起来。在其当前形式下，它仅限于二维矩阵类型，操作通过 LLVM 内置函数实现。相比之下，MLIR 处于更高的抽象层次，允许将 MLIR 中泛型 n-D 向量类型上的泛型操作降低到一维 LLVM 向量的聚合。将来，即使对于 CPU，也可能有意义降低到 LLVM matrix 抽象，尽管 MLIR 将继续需要更高层次的抽象。

另一方面，应该注意到，随着 MLIR 迁移到 LLVM，本文档可能成为人们应该针对一维向量的统一抽象，而 LLVM matrix 提案可以被视为这项工作的子集。

### 结论

LLVM matrix 提案中展平的一维向量设计在具有特殊内置函数的特定于硬件的世界中很好。对于寄存器层的寄存器分配、指令级并行性（ILP）和软件流水线/模数调度优化，这是一个良好的抽象。然而，MLIR 代码生成在更高的抽象层次上操作，在那里我们希望针对比硬件大小更粗粒度的向量上的操作，并对其应用展开-并合，且可以匹配跨多个硬件向量的模式。

这使得"一维向量的嵌套聚合类型"成为从 MLIR 降低的一种吸引人的抽象，因为：

1.  它不隐藏与缓冲区和值语义以及内存子系统相关的复杂性；
2.  它不依赖 LLVM 从过低层次的抽象神奇地使一切工作。

在一维 LLVM 世界中使用特殊内置函数仍然可以通过显式的 `vector.cast` 算子实现。

## 操作

[include "Dialects/VectorOps.md"]
