# MLIR 设计原理

本文旨在记录 MLIR 设计过程中所考虑的若干备选方案、尚存的争议，以及某些决策背后的设计原理（rationale）。本文并非精心雕琢的正式文档——我们更倾向于随时记录有价值的细节，而不必过于在意其一致性或可读性。

[TOC]

## 摘要

MLIR 是一种编译器中间表示（Intermediate Representation，IR），与传统的三地址静态单赋值（SSA）表示（如 [LLVM IR](http://llvm.org/docs/LangRef.html) 或 [SIL](https://github.com/apple/swift/blob/main/docs/SIL.rst)）有诸多相似之处，但它将多面体（polyhedral）循环优化领域的概念作为一等公民引入其中。这种混合设计专为表示、分析和变换高层次数据流图（dataflow graph）以及针对高性能数据并行系统生成的目标相关代码而优化。除表示能力之外，其统一连贯的设计提供了一套从数据流图一路降级（lower）到高性能目标相关代码的框架。

MLIR 是以下名称之一的缩写："Multi-Level IR"（多层级中间表示）、"Multi-dimensional Loop IR"（多维循环中间表示）、"Machine Learning IR"（机器学习中间表示）或"Mid Level IR"（中间层中间表示），我们更倾向于第一种解释。本文仅阐述 MLIR 的设计原理——其正式的[规范文档](../LangRef.md)及其他内容另行托管。

## 介绍与动机

多层级中间表示（Multi-Level Intermediate Representation，MLIR）旨在方便地表达和优化涉及深层循环嵌套（loop nest）与高维稠密矩阵的计算。因此，它尤其适合深度学习计算。但它同样足够通用，能够表示任意顺序计算。该表示支持针对各类并行架构（包括具有深层内存层次结构的通用多核处理器、GPU 和专用神经网络加速器）的高层次优化与并行化。

MLIR 借鉴了 LLVM 和 Swift 的 IR 设计思路用于低层次构造，同时将其与多面体抽象的思想相结合，以便在 IR 中将循环嵌套、多维数据（张量）及对这些实体的变换作为一等公民概念来表示。

MLIR 是一种多层级 IR，即它可以表示从领域特定表示（如 HLO 或 TensorFlow 计算图）直至机器级别的各层代码。MLIR 能够表示任意控制流和任意数据访问，足够通用，几乎可以表示所有顺序计算。这是它与现有多面体表示实现（如 LLVM 的 [Polly](https://polly.llvm.org/)）的重要区别——后者只能在与 LLVM IR 相隔离的环境中使用多面体抽象，且仅限于仿射（affine）循环嵌套，即数组访问、循环边界和条件均为规则的（涉及循环迭代变量和常量符号的线性函数）代码段。静态不可预测的数据访问或控制流并不妨碍在 MLIR 中进行表示，但在一定程度上会限制使用多面体抽象进行推理和变换的能力。

带有仿射约束的映射（map）、集合（set）和关系（relation）是用多面体表示高维循环嵌套和多维数组的核心结构。这些结构以接近其数学形式的文本表达式来表示，用于捕获循环嵌套、张量数据结构，以及它们在目标架构上的重排序和映射方式。所有结构化或"合规"的循环均作为多面体信息的一部分被捕获，张量变量、它们的布局（layout）以及对这些张量的下标访问也同样如此。

IR 中捕获的信息能够紧凑地表达所有循环变换、数据重映射、加速器显式寻址内存所需的显式复制、到预调优专家原语的映射，以及到专用向量指令的映射。可轻松实现的循环变换包括仿射变换的主体：这些变换涵盖了所有传统循环变换（单模和非单模），如循环分块（tiling）、交换（interchange）、置换（permutation）、倾斜（skewing）、缩放（scaling）、相对移位（relative shifting）、翻转（reversal）、融合（fusion）和分布/分裂（distribution/fission）。数据布局变换（如填充（padding）和变换为分块布局（blocked layout））也可通过仿射布局映射（affine layout map）得到良好表示。

MLIR 的设计支持渐进式降级（progressive lowering）到目标相关形式。除了典型的中层优化器所需处理的循环嵌套和数据布局高层变换外，MLIR 还被设计为可执行某些通常由后端 IR 负责的低层调度和映射决策：包括映射到专用向量指令、自动向量化（auto-vectorization）和软件流水线（software pipelining）。需要支持这些变换的原因在于，神经网络加速器拥有处理大块数据的专用单元，这些数据的计算可以映射回更接近原始规约层次的程序视角下的多个循环层次的数据块。这类专用单元或指令从程序员的视角处理多维数据块，这使得在非常低层（接近汇编）的 IR 上工作的后端难以或无法提升并重构循环以执行此类映射。这与当今编译器中主要只处理最内层循环体的经典指令选择和调度形成鲜明对比。MLIR 还有助于将计算自动映射到在内存层次结构更高层（或最高层）操作数据的专家预调优原语或供应商库。

综上所述，MLIR 便于且在变换上封闭（closed）于将计算降级到通用处理器和专用加速器所需的各类变换。它还支持构建模块化且可复用的目标无关和目标相关的遍（pass）。

## 设计决策

本节阐明若干设计决策——其中一些在规范文档中已有间接体现。

### 加载与存储

`load` 和 `store` 指令被专门设计为能够完全解析到 memref 的某个元素。这些指令接受 n 个索引加 1 的参数（共 n+1 个）用于 n 秩张量。这不允许等价的指针算术或以其他方式索引同一 memref（例如 C 数组所允许的方式）。此外，对于仿射构造，编译器可以跟踪定义-使用链（use-def chain，例如通过 [affine.apply 操作](../Dialects/Affine.md/#affineapply-affineapplyop) 或 [仿射操作](../Dialects/Affine.md/#operations) 的映射属性）来在编译时使用多面体技术精确分析引用。这之所以可行，是因为对[维度和符号的限制](../Dialects/Affine.md/#restrictions-on-dimensions-and-symbols)。

存储在内存中的元素类型标量（原始类型或向量类型）被建模为 0 维 memref。这对于函数中在 for 循环和 if 条件之外存活的标量（live-out scalar）也是必要的，因为我们目前还没有针对这类标量的 SSA 表示——本文稍后描述了[一个扩展方案](#affineif-and-affinefor-extensions-for-escaping-scalars)以支持该情况。

### 符号与类型

当前 MLIR 不允许在类型中使用符号。例如，当张量或 memref 的某个维度在静态时未知，它在类型中用 `?` 表示。当 memref 被创建时，会将一个 SSA 符号绑定到该维度上。未知维度的实际值可通过 "dim" 内置操作查询，如下所示。

示例：

```mlir
func.func foo(...) {
  %A = memref.alloc <8x?xf32, #lmap> (%N)
  ...
  call bar(%A) : (memref<8x?xf32, #lmap>)
}

func.func bar(%A : memref<8x?xf32, #lmap>) {
  // Type of %A indicates that %A has dynamic shape with 8 rows
  // and unknown number of columns. The number of columns is queried
  // dynamically using dim instruction.
  %N = memref.dim %A, 1 : memref<8x?xf32, #lmap>

  affine.for %i = 0 to 8 {
    affine.for %j = 0 to %N {
      // A[i,j] += 1
      %s1 = affine.load %A[%i, %j] : memref<8x?xf32, #lmap>
      %s2 = add %s1, 1
      affine.store %s2, %A[%i, %j] : memref<8x?xf32, #lmap>
    }
  }
  return
}

```

另一种设计是将符号引用直接嵌入类型中，例如 memref<8x%Nxf32>。MLIR 选择了当前方案，因为它简化了设计——当符号值改变时，类型保持不变（immutable）。

### 块参数与 PHI 节点

MLIR 的区域（Region）使用"[块参数（block argument）](../LangRef.md/#blocks)"来表示 SSA，而不是 LLVM 中使用的 [PHI 指令](http://llvm.org/docs/LangRef.html#i-phi)。这两种选择在表示上是等价的（相同的构造可以用任一形式表示），但块参数具有若干优势：

1.  LLVM 的 PHI 节点必须始终保持在块的顶部，变换通常需要手动跳过它们。使用基本块（BB）参数则消除了这一问题。
1.  LLVM 有单独的函数参数（Argument）节点。使用 BB 参数同样消除了这一问题，因为入口块的参数就扮演了这一角色。
1.  LLVM 中 PHI 节点块的执行是原子性的，这令编译器工程师困惑且极易引入错误（与 SSA 降级文献中的"[丢失复制（lost copy）](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.524.5461&rep=rep1&type=pdf)"问题密切相关）。使用 BB 参数表示则消除了这种混淆。
1.  LLVM 中 PHI 节点的前驱列表是无序的，某些块可能有数千个前驱（例如 unwind 块），这会导致变换需要线性扫描该列表从而引起较长的编译时间。使用 BB 参数表示则消除了这一问题。
1.  LLVM 无法表示仅在某一后继中可用而在另一后继中不可用的值，例如其 invoke 指令无法仅在异常边上产生异常值。为此，[landingpad 指令](http://llvm.org/docs/LangRef.html#landingpad-instruction) 被用作一种 hack 来表示这一点。MLIR 并未使用这种能力，但 SIL 大量使用了它，例如在 [switch_enum 指令](https://github.com/apple/swift/blob/main/docs/SIL.rst#switch-enum) 中。

更多背景信息：块参数此前曾用于 Swift 的 [SIL 中间表示](https://github.com/apple/swift/blob/main/docs/SIL.rst)，并在 [YouTube 的一次演讲](https://www.youtube.com/watch?v=Ntj8ab-5cvE) 中有所介绍。相关部分[从此处开始](https://www.youtube.com/watch?v=Ntj8ab-5cvE&t=596s)。

### index 类型的用法与限制

index 类型旨在用于平台特定的"大小"值，可出现在下标（subscript）、聚合类型的大小以及仿射表达式中。它们也与 `affine.apply` 及 affine.load/store 操作紧密耦合；具有 `index` 类型是一个值被这些操作接受的必要前提条件。

我们允许在张量、向量和 memref 中使用 `index` 类型，因为代码生成策略必须将 `index` 映射到某个实现类型，因此需要能够具体化（materialize）相应的值。但是，目标架构可能不支持使用目标特定的 `index` 类型等价物作为元素的 `vector` 值。

### 非原始类型的数据布局

数据布局信息（如类型的位宽或对齐方式）可能因目标架构和 ABI 而异，因此应当是可配置的，而非由编译器强制规定。复合类型或 `index` 类型的布局尤为如此。MLIR 为某些原始*类型*（特别是整数和浮点数）规定了默认位宽，等于类型定义中出现的数字，例如 `i32` 的位宽为 `32`，`f32` 的位宽同样为 `32`。位宽与存储该类型值所需的内存量（字节数）或寄存器大小（位数）不一定相关。例如，`vector<3xi57>` 很可能被降级为四个 64 位整数的向量，其存储需求为 `4 x 64 / 8 = 32` 字节，而不是从位宽简单计算得出的 `(3 x 57) ceildiv 8 = 22` 字节。MLIR 通过属性使此类[数据布局信息](../DataLayout.md)可配置，这些属性可在降级时查询，例如在分配复合类型时。

方言（dialect）特定类型的数据布局在 MLIR 层面是未定义的。但方言可以自由定义自己的量，并通过数据布局基础设施使其可用。

### 整数符号语义

MLIR 内置类型系统中的整数具有位宽（注意 `index` 类型的符号宽度等于机器字长），并且*可以*额外具有符号（signedness）语义。这是为了满足不同方言的需求——方言可以对不同层次的抽象建模。某些抽象（尤其是更接近源语言的抽象）可能希望区分整数类型的符号性；而另一些（尤其是更接近机器指令的抽象）可能希望使用无符号语义的整数。为了避免强制每种抽象采用相同的整数建模方式或各自重新实现，Integer 类型将此作为一个选项，以促进代码复用和一致性。

对于标准方言（standard dialect），选择是使用无符号语义（signless）的整数类型。整数值本身不带有内在的符号，具体的解释由特定的操作决定。例如，`arith.addi` 和 `arith.muli` 执行二进制补码算术，而某些其他操作带有符号，例如 `arith.divsi`（有符号除法）与 `arith.divui`（无符号除法）。

LLVM 采用了[相同的设计](http://llvm.org/docs/LangRef.html#integer-type)，该设计随 [LLVM 2.0 整数类型的改版](http://releases.llvm.org/2.0/docs/LangRef.html#t_derived)而引入。在此之前，从 [LLVM 1.0](http://releases.llvm.org/1.0/docs/LangRef.html#t_classifications) 到 [1.9](http://releases.llvm.org/1.9/docs/LangRef.html#t_classifications)，LLVM 使用带符号类型如 "sbyte" 和 "ubyte"。这一转变非常重要，多年来也确实使 LLVM 受益匪浅。其重要性在于：中间表示对同一计算使用同一条指令是一件好事。带符号类型带来了麻烦，例如 "sbyte 的加法" 与 "ubyte 的加法" 执行的是相同的计算，但类型系统却使它们看起来人为不同。这种分裂还需要诸如 "从 sbyte 强制转换到 ubyte" 这样在机器层面毫无意义的类型转换。从类型系统中去除符号消除了这些问题，使编译器更加简洁。

关于这一转变的更多信息，可参见一段讨论 LLVM 2.0 的[旧 YouTube 演讲](https://www.youtube.com/watch?v=VeRaLPupGks)。

注意，此设计原理仅适用于我们可以表达设计意见的"标准操作（standard ops）"方言。其他方言通常旨在对外部系统建模，应尽可能忠实地反映该系统的设计。

### 浮点运算与整数运算的分离

MLIR 的"Arith"方言将许多整数和浮点操作分为不同类别，例如 `arith.addf` 与 `arith.addi`，以及 `arith.cmpf` 与 `arith.cmpi`（[遵循 LLVM 的设计](http://llvm.org/docs/LangRef.html#binary-operations)）。但这些指令在类型的元素数量上是多态（polymorphic）的，例如 `addf` 可用于标量浮点数、浮点数向量和浮点数张量（LLVM 对其标量/向量类型也做了同样的事情）。

这种分离非常重要，因为浮点和整数操作在实践中差异显著：例如，浮点值包含 NaN，因此[整数比较](http://llvm.org/docs/LangRef.html#icmp-instruction)和[浮点比较](http://llvm.org/docs/LangRef.html#fcmp-instruction)应使用不同的比较操作码。在算术层面，浮点操作支持舍入模式、浮点收缩（contraction）和["快速数学（fast math）"](http://llvm.org/docs/LangRef.html#fadd-instruction)，而整数可能需要二进制补码溢出行为，或在[各种环绕（wrapping）形式](http://llvm.org/docs/LangRef.html#add-instruction)下具有未定义行为以实现性能优化。

在 MLIR 中，这类问题目前还远未成为优先考虑事项，但由于我们有相关经验且知道正确的做法，我们宁愿从一开始就将其纳入设计。

注意，此设计原理仅适用于我们可以表达设计意见的"标准操作"方言。其他方言通常旨在对外部系统建模，应尽可能忠实地反映该系统的设计。

### 在整数比较操作中指定符号

由于整数是[无符号语义](#整数符号语义)的，有必要为整数比较操作指定符号。该符号指示如何处理整数的最高位：作为符号位还是最高有效位。例如，比较两个 `i4` 值 `0b1000` 和 `0b0010`，在无符号（`8 > 3`）和有符号（`-8 < 3`）解释下结果不同。这种差异仅对*顺序*比较有意义，对*相等性*比较则无影响。事实上，对于相等性比较，所有位必须具有相同的值，与符号无关。由于两个参数的位宽完全相同且该操作不会进行填充，因此不可能出现两个值的位表示不同但被解释为相等的情况。

### 将比较类型指定为属性

与算术操作不同，比较操作共享若干公共属性，例如不可被视为结合律。在实践中，比较操作有时由同一条指令或其变体实现，因此在 IR 层面将它们归为一组是合理的。

另一种方案是为当前支持的所有整数比较种类引入十个不同的操作符。这些操作符会增加标准操作使用的"保留"名称数量以及 C++ API 的规模，而其实现在很大程度上是相同的。

比较类型在内部是一个整数属性。但为了人类可读性，自定义汇编（assembly）形式接受字符串字面量并将其映射到底层整数值：`cmpi "eq", %lhs, %rhs` 比 `cmpi 0, %lhs, %rhs` 更清晰地表达整数相等比较的含义（后者中什么与什么比较并不明确）。这种语法糖得益于对非内置操作自定义汇编形式的解析器逻辑的重新定义。若要在完整符号中支持此特性，则需要修改主解析算法的工作方式，可能带来意想不到的影响。虽然可以将谓词存储为字符串属性，但这会使基于比较类型的 switch 逻辑无法实现，并使属性合法性检查（十种可能类型之一）更加复杂。

### 区域（Region）

#### `Block` 类型的属性

我们曾考虑通过 `ArrayAttr` 来表示区域，其中包含特殊类型 `IRBlockAttr` 的列表，而 `IRBlockAttr` 又包含操作列表。由于 MLIR 中所有属性在上下文中都是唯一化（unique'd）的，这会使区域内部的 IR 无故成为"不朽"的（immortal）。

#### 使用"内联"函数作为区域

我们考虑过在函数上附加 "force-inline" 属性或在函数 `call` 操作上附加类似属性。即使是最小的区域支持（在引入区域之前 affine.for 和 affine.if 的用例），也需要访问主导块（dominating block）中定义的值，而函数不支持这一点。从概念上讲，函数体是区域的一个实例，而非相反；区域还可以是设备内核（device kernel）、备选代码段等。

#### 专用的 `region` 操作

这意味着我们需要一种允许拥有区域的特殊操作，而其他操作则不行。这种区分类似于我们已经有过并选择取消的 Stmt/Op 区别，目的是让 IR 更简洁、更灵活。这还需要分析和遍在操作交互时加以考虑（例如，`affine.for` 操作后面必须跟随一个 region 操作）。最终，使用当前实现完全可以引入 region 操作，它与其他操作并无本质区别。

#### 显式捕获区域中使用的值

能够使用在区域外定义的值意味着定义-使用链（use-def chain）中可能包含来自不同嵌套区域的使用。因此，IR 变换和分析可以将定义某值的指令跨区域边界拉取，例如在 TableGen 定义的规范化（canonicalization）模式中。如果所有使用的值都作为区域参数传入，则不会出现这种情况。在 IR 中引入区域的动机之一，正是希望实现比过程间变换更简单的跨区域分析和变换。与维护一个额外数据结构来建立函数调用参数（作为原始定义的使用）与形式参数（作为新定义）之间对应关系的方式相比，在同一定义-使用链中出现来自不同区域的使用更有利于这种简化。由于每个操作现在归属于块，块归属于区域，因此始终可以检查某个值的定义是否属于其使用所在的同一区域。风险在于任何 IR 遍历都需要显式处理这种情况，而很容易遗漏检查（反之，在 tablegen 模式中设计正确的检查也并不容易）：遍历定义-使用链可能会隐式地跨越语义边界，使得在不知情的情况下破坏区域语义成为可能。这预期会在变换后的验证器（verifier）中被捕获。

另一方面，也可以选择将某些或全部值作为区域参数传入，以显式断开当前提案中的定义-使用链。这可以与属性强制要求区域体不得引用其外部值的语义约束相结合。

### 方言类型扩展

本节描述影响 MLIR 中方言可扩展类型系统设计的若干决策。

#### 方言间的交互

方言间有两种重要的交互方式。当一个方言的类型：

*   出现在其他方言的操作中时

    -   对于标准/内置操作，只允许内置类型。这一限制使操作能够清楚地了解其所工作的不变量。
    -   在标准/内置操作之外，方言应按操作验证其允许的操作类型。

*   出现在其他方言的类型中时

    -   对于内置类型，这些类型允许包含来自其他方言的类型。这简化了类型系统，消除了方言重新定义所有内置聚合类型（如 tensor 和 memref 类型）的需要。方言应验证特定类型在内置类型中是否有效，例如某类型是否可以作为张量的元素。
    -   对于方言类型，该方言应验证任何类型不变量，例如张量类型是否可以包含该方言的某种特定类型。

#### 分离内置类型与标准类型

遵循内置与标准方言的分离原则，将内置类型与标准方言类型分离是合理的。内置类型对 IR 本身的合法性是必需的，例如函数类型（出现在函数签名和操作的通用汇编形式中）。整数、浮点数、向量、memref 和张量类型虽然重要，但对 IR 的合法性并非必需。

#### 未注册类型

MLIR 支持以通用汇编形式表示的未注册操作，对类型也支持类似的概念。解析时，如果方言类型所属的方言未被注册，该类型将被建模为 `OpaqueType`（不透明类型）。这允许类型在不需要链接定义它们的方言库的情况下进行往返（round-trip）。不透明类型在解析/打印之外不提供任何额外信息。

#### 方言类型语法

方言扩展类型以包裹在方言命名空间内的字符串字面量形式表示。这意味着解析器将特定类型实例的解析工作委托给方言。这与方言定义操作的表示方式不同，后者使用标识符名称，解析器通过该名称识别并解析操作。

选择这种表示方式有以下几个原因：

##### 方言必须提供自定义类型解析器

方言类型解析无法像操作那样插入现有的解析器基础设施（OpAsmParser/Printer）。操作具有所有方言共同遵循的已定义语法结构。而类型则可能有许多不同且有时相互冲突的解析约束，很难在单一接口中维护。

这也带来了鼓励方言复用现有外部类型解析器的额外好处。例如，LLVM 方言可以提供一个 MLIR LLVM 类型，它只是对 LLVM 类型的封装，并复用现有的 LLVM 类型解析基础设施。

示例：

```mlir
%s = "foo"() : () -> !llvm<"i32*">
```

##### 类型并不总是有规范名称

与操作不同，类型通常没有正式的规范名称。例如，函数类型没有定义的关键字，整数类型则通过正则表达式定义以支持任意位宽。具有现有类型系统的方言（如 LLVM）很可能会为其现有类型系统提供封装。对于这些封装类型，没有简单的规范名称，将这些类型视为存在于方言命名空间内是合乎逻辑的。如果方言希望为某类型指定规范名称，可以通过[类型别名（type alias）](../LangRef.md/#type-aliases)实现。

### 元组类型

MLIR 类型系统为定义[元组类型（tuple type）](../Dialects/Builtin/#tupletype)提供了一等支持。这是因为 `Tuple` 表示一个普遍概念，它很可能且已经开始在许多不同方言中出现。尽管该类型在类型系统中是一等公民，但它仅用于提供一种在 MLIR 中表示这一概念的公共机制。因此，MLIR 不为与 `tuple` 类型交互提供标准操作。方言作者需要自行提供操作（例如 extract_tuple_element）来解释和操作元组。在可能的情况下，操作应优先使用多个结果来代替元组。多个结果具有诸多好处，例如消除了仅用于拦截分析和变换路径的 tuple-extract 操作的需要。

### 汇编形式

MLIR 在以下考量下决定同时支持通用（generic）和自定义（custom）汇编形式：

MLIR 是一个开放系统；它被设计为支持模块化且可插拔的方言。根据对应方言是否存在以及是否已被插入，操作可能在 MLIR 系统中注册，也可能未注册。但我们仍然需要一种方式来查看这些操作。因此，通用汇编形式由 MLIR 系统的这一特性所要求，为操作提供默认的文本形式。

另一方面，汇编形式的目的是帮助开发人员查看 IR。通用形式作为安全的回退，但对于某些操作来说可能过于冗长。因此，MLIR 赋予每个方言根据操作的语义和具体需求为每个操作定义自定义汇编形式的选择权。自定义汇编形式可以从操作中去除冗余信息以得出更简洁的形式，从而更好地促进对 IR 的理解。

## 示例

本节描述若干非常简单的示例，帮助理解 MLIR 如何表示计算。

### 非仿射控制流

```mlir
// A simple linear search in every row of a matrix
for (i = 0; i < N; i++) {
  for (j = 0; j < N; j++) {
    // dynamic control flow
    if (a[i][j] == key) {
      s[i] = j;
      break;
    }
  }
}
```

动态控制流的存在导致内层非仿射函数嵌套在使用仿射循环的外层函数中。

```mlir
func.func @search(%A: memref<?x?xi32>, %S: <?xi32>, %key : i32) {
  %ni = memref.dim %A, 0 : memref<?x?xi32>
  // This loop can be parallelized
  affine.for %i = 0 to %ni {
    call @search_body (%A, %S, %key, %i) : (memref<?x?xi32>, memref<?xi32>, i32, i32)
  }
  return
}

func.func @search_body(%A: memref<?x?xi32>, %S: memref<?xi32>, %key: i32, %i : i32) {
  %nj = memref.dim %A, 1 : memref<?x?xi32>
  cf.br ^bb1(0)

^bb1(%j: i32)
  %p1 = arith.cmpi "lt", %j, %nj : i32
  cf.cond_br %p1, ^bb2, ^bb5

^bb2:
  %v = affine.load %A[%i, %j] : memref<?x?xi32>
  %p2 = arith.cmpi "eq", %v, %key : i32
  cf.cond_br %p2, ^bb3(%j), ^bb4

^bb3(%j: i32)
  affine.store %j, %S[%i] : memref<?xi32>
  cf.br ^bb5

^bb4:
  %jinc = arith.addi %j, 1 : i32
  cf.br ^bb1(%jinc)

^bb5:
  return
}
```

根据 [MLIR 规范](../LangRef.md)，用于 affine.apply 操作的维度和符号标识符的限制仅适用于 `affine.for` 和 `affine.if` 操作内部的访问。但是，为了确定 `%i` 循环是否可以并行化，需要分析被调用函数（`@search_body`）内部的访问：此类函数访问分析依赖调用上下文。

### 非仿射循环边界

循环边界非仿射的情况会导致如下所示的函数嵌套。

```c
for (i = 0; i < N; i++)
  for (j = 0; j < N; j++)
    // Non-affine loop bound for k loop.
    for (k = 0; k < pow(2, j); k++)
       for (l = 0; l < N; l++) {
        // block loop body
        ...
       }
```

```mlir
func.func @outer_nest(%n : index) {
  affine.for %i = 0 to %n {
    affine.for %j = 0 to %n {
      %pow = call @pow(2, %j) : (index, index) ->  index
      call @inner_nest(%pow, %n) : ...
    }
  }
  return
}

func.func @inner_nest(%m : index, %n : index) {
  affine.for %k = 0 to %m {
    affine.for %l = 0 to %n {
      ...
    }
  }
  return
}
```

### 参考 2D 卷积

以下示例展示了一个 2D 卷积的参考实现，其中使用整数集 `#domain` 表示扩张卷积（dilated convolution）中有效的输入数据。

```mlir
// Dilation factors S0 and S1 can be constant folded if constant at compile time.
#domain = (d0, d1)[S0,S1,S2,S3]: (d0 % S0 == 0, d1 % S1 == 0, d0 >= 0, d1 >= 0,
                                   S3 - d0 - 1 >= 0, S4 - d1 - 1 >= 0)
// Identity map (shown here for illustration).
#map0 = (d0, d1, d2, d3, d4, d5, d6) -> (d0, d1, d2, d3, d4, d5, d6)

// Affine map from output to input coordinate space.
// d0 = output_h, d1 = output_w, d2 = kernel_h, d3 = kernel_w
// S0 = h_stride, S1 = w_stride, S2 = h_kernel_dilation, S3 = w_kernel_dilation
// S4 = h_pad_low, S5 = w_pad_low
//     %out0 =  %0#1 * %h_stride + %0#4 * %h_kernel_dilation - %h_pad_low
//     %out1=  %0#2 * %w_stride + %0#5 * %w_kernel_dilation - %w_pad_low
#map1_0 = (d0, d1, d2, d3) [S0, S1, S2, S3, S4, S5] -> (d0 * S0 + d2 * S2 - %S4)
#map1_1 = (d0, d1, d2, d3) [S0, S1, S2, S3, S4, S5] -> (d1 * S1 + d3 * S3 - %S5)

// Semi-affine map to undilated input coordinate space.
// d0 = input_h, d1 = input_w, S0 = h_base_dilation, S1 = w_base_dilation.
#map2_0 = (d0, d1) [S0, S1] -> (d0 / S0)
#map2_1 = (d0, d1) [S0, S1] -> (d1 / S1)

// Conv2D shapes:
// input:   [batch, input_height, input_width, input_feature]
// kernel: [kernel_height, kernel_width, input_feature, output_feature]
// output: [batch, output_height, output_width, output_feature]
func.func @conv2d(%input: memref<16x1024x1024x3xf32, #lm0, /*scratchpad=*/1>,
             %kernel: memref<5x5x3x32xf32, #lm0, /*scratchpad=*/1>,
             %output: memref<16x512x512x32xf32, #lm0, /*scratchpad=*/1>) {
  affine.for %b = 0 to %batch {
    affine.for %oh = 0 to %output_height {
      affine.for %ow = 0 to %output_width {
        affine.for %of = 0 to %output_feature {
          affine.for %kh = 0 to %kernel_height {
            affine.for %kw = 0 to %kernel_width {
              affine.for %if = 0 to %input_feature {
                // Calculate input indices.
                %1_0 = affine.apply #map1_0 (%0#1, %0#2, %0#4, %0#5)
                  [%h_stride, %w_stride, %h_kernel_dilation, %w_kernel_dilation,
                   %h_pad_low, %w_pad_low]
                %1_1 = affine.apply #map1_1 (%0#1, %0#2, %0#4, %0#5)
                  [%h_stride, %w_stride, %h_kernel_dilation, %w_kernel_dilation,
                   %h_pad_low, %w_pad_low]

                // Check if access is not in padding.
                affine.if #domain(%1_0, %1_1)
                                       [%h_base_dilation, %w_kernel_dilation, %h_bound, %w_bound] {
                  %2_0 = affine.apply #map2 (%1_0, %1_1)
                  %2_1 = affine.apply #map2 (%1_0, %1_1)
                  // Compute: output[output_indices] += input[input_indices] * kernel[kernel_indices]
                  call @multiply_accumulate(%input, %kernel, %output, %b, %oh, %ow, %of, %kh, %kw, %if, %2_0, %2_1)
                }
              }
            }
          }
        }
      }
    }
  }
  return
}
```

TODO：（添加更多示例，展示各种有趣场景下的 IR）

## 设计备选方案与扩展

以下是我们详细讨论但未纳入规范或推迟到未来按需考虑的若干设计备选方案和扩展。当我们积累更多实现经验并更深入地了解当前设计在实践中的挑战和局限性后，将重新审视这些讨论。

### 多面体代码表示备选方案：调度列表 vs 调度树 vs 仿射循环/if 形式

当前 MLIR 使用由 if/for 循环树构成的多面体调度表示方式。我们对典型的无序多面体指令表示（每条指令具有多维调度信息）的权衡进行了广泛辩论，讨论了调度树形式的优点，并最终决定采用仿射 if/else 条件和仿射 for 循环的语法树。对该权衡的讨论记录在以下文档中：[MLIR：简化多面体形式的理由](RationaleSimplifiedPolyhedralForm.md)。

从高层来看，我们有两种备选方案：

1.  调度树表示，而非仿射循环 AST 形式：当前提案使用仿射循环和条件树形式，这是语法性的，没有将域（domain）和调度（schedule）作为集合和多维仿射函数分离。而调度树形式将多面体域和调度作为 IR 中的一等概念，允许通过调度树紧凑地表达变换，而无需更改指令的域。这种表示还隐藏了序言（prologue）、尾声（epilogue）、部分分块（partial tile）、复杂循环边界和条件，使循环嵌套不含"语法"。代价模型转而关注域和调度。此外，如有必要，这种域-调度表示可以被规范化，以显式地将调度传播到域中并对所有清理代码建模。调度树形式的示例和更多细节见下一节。
1.  拥有两种不同的"仿射区域"形式：仿射循环树形式和多面体调度树形式。在后者中，操作可以携带属性以捕获域、调度和其他多面体代码生成选项，使用 IntegerSet、AffineMap 和其他属性。

#### 仿射区域的调度树表示

该表示基于多面体编译器社区使用的域/调度表示的简化形式。域（domain）表示需要执行的内容，而调度（schedule）表示域元素的交错顺序。我们将域建模为非分片凸整数集，将调度建模为仿射函数；但前者可以是析取的，后者可以是分片仿射关系。在调度树表示中，指令的域和调度以树状结构表示，称为调度树。树的每个非叶节点是一个抽象的多面体维度，对应于出现在该分支中每条 ML 指令的一个抽象融合循环。每个叶节点是一条 ML 指令。

```mlir
// A tiled matmul code (128x128x128) represented in schedule tree form

// #map0 = (d0, d1, d2, d3, d4, d5) -> (128*d0 + d3, 128*d1 + d4, 128*d2 + d5)
#intset_ij = (i, j) [M, N, K]  : i >= 0, -i + N - 1 >= 0, j >= 0, -j + N-1 >= 0
#intset_ijk = (i, j, k) [M, N, K] : i >= 0, -i + N - 1 >= 0, j >= 0,
                                     -j +  M-1 >= 0, k >= 0, -k + N - 1 >= 0)
func.func @matmul(%A, %B, %C, %M, %N, %K) : (...)  { // %M, N, K are symbols
  // t1, t2, t3, t4, t5, t6  are abstract polyhedral loops
  mldim %t1 : {S1,S2,S3,S4,S5}  floordiv (i, 128) {
    mldim %t2 : {S1,S2,S3,S4,S5}  floordiv (j, 128) {
      // (%i, %j) = affine.apply (d0, d1) -> (128*d0, 128*d1) (%t1, %t2)
      call dma_mem_to_scratchpad(%C, %i, %j, %M, %N, %K)
          with @intset_ij(%i, %j) [%M, %N, %K]
      mldim %t3 :   {S2,S3,S4,S5} floordiv (k, 128) {
        // (%i, %j, %k) = affine.apply (d0, d1, d2)
        //                          -> (128*d0, 128*d1, 128*d2)  (%t1, %t2, %t3)
        call dma_mem_to_scratchpad(%A, ...) with #inset_ijk (%i, %j, %k) [%M, %N, %K]
        // (%i, %j, %k) = affine.apply (d0, d1, d2)
        //                          -> (128*d0, 128*d1, 128*d2)  (%t1, %t2, %t3)
        call dma_mem_to_scratchpad(%B, ...) with #inset_ijk (%i, %j, %k) [%M, %N, %K]
        mldim %t4 : {S4} i mod 128 {
          mldim %t5 : {S4} j mod 128 {
            mldim %t6 : {S4} k mod 128 {
              // (%i, %j, %k) = affine.apply #map0 (%t1, %t2, %t3, %t4, %t5, %t6)
              call matmul_body(A, B, C, %i, %j, %k, %M, %N, %K)
                  with #inset_ijk(%i, %j, %k) [%M, %N, %K]
            } // end mld4im t6
          } // end mldim t5
        } // end mldim t4
      } // end mldim t3
      // (%i, %j) = affine.apply (d0, d1) -> (128*d0, 128*d1) (%t1, %t2)
      call $dma_scratchpad_to_mem_C ... with #intset(%i, %j) [%M, %N, %K]
    }  // end mldim t2
  } // end mldim t1
  return
}

```

### 仿射关系

当前 MLIR 规范包含仿射映射（affine map）和整数集（integer set），但不包含仿射关系（affine relation）。仿射关系是对读写访问信息建模的自然方式，对于在没有实现的情况下捕获外部库调用的行为（如高性能供应商库或用户提供/调优的例程）非常有用。

仿射关系是输入和输出维度标识符之间的关系，对符号标识符列表是符号性的，并且具有关于这些标识符的仿射约束。

语法：

```
// Affine relation definition at the top of file
affine-rel-def ::= affine-rel-id `=` affine-relation-inline

affine-rel-id ::= `##` prefixed-id

affine-relation-inline ::=
       `(` input-dims `)` (`[` symbols `]`)? `->`
       `(` output-dims `)` :  affine-constraint-conjunction

input-dims ::= bare-id-list
output-dims ::= bare-id-list
symbols ::= bare-id-list

affine-rel ::= affine-rel-id | affine-relation-inline

// Usage
affine-rel-spec ::= affine-rel dim-and-symbol-use-list
```

出现在 input-dims、output-dims 和 symbol-dims 中的所有标识符两两不同。上述语法中所有 affine-constraint 非终结符只允许包含来自 input-dims、output-dims 和 symbol-dims 的标识符。

仿射关系用于在 IR 中对函数的 read、write、may_read 和 may_write 集合建模。输出维度标识符对应数据维度。

示例：

```mlir
// read relation: two elements ( d0 <= r0 <= d0+1 )
##aff_rel9 = (d0) -> (r0) : r0 - d0 >= 0, d0 - r0 + 1 >= 0

func.func @count (%A : memref<128xf32>, %pos : i32) -> f32
  reads: {%A ##aff_rel9 (%pos)}
  writes: /* empty */
  may_reads: /* empty */
  may_writes: /* empty */ {
bb0 (%0, %1: memref<128xf32>, i64):
  %val = affine.load %A [%pos]
  %val = affine.load %A [%pos + 1]
  %p = arith.mulf %val, %val : f32
  return %p : f32
}
```

### 区域（Region）

#### 将函数定义作为操作

MLIR 支持函数类型的值。与其将函数作为一等 IR 概念，可以定义一个带有体区域的操作，用以定义函数值。函数的特殊之处在于，其名称是全局可见的，并且可以在定义之前被引用，这与必须先定义的 SSA 值不同。实现"函数定义"操作需要放宽区域中的部分 SSA 约束，并将 IR 模块（Module）也设为区域，仅仅为了概念统一而影响核心基础设施（例如函数遍）。

#### 为区域设置类型

与其检查第一个块的参数类型，可以给区域本身一个类型。这个类型会与块参数类型冗余（后者必须有值），并为类型不匹配留下空间。虽然函数确实有与第一个块参数部分冗余的类型，但这对于支持没有函数体（因而无法通过函数体获取参数类型）的函数声明是必要的。区域总是包含在操作或函数中，必要时可以查询这些实体来获取区域的"类型"。

若区域被认为是独立于其封闭实体（操作或函数）的、具有应被检查的自身语义，则给区域设置类型是有理由的。

#### 为区域附加属性

区域可以用方言属性注解，以使用属性验证钩子（attribute verification hook）。一个操作可以接受多个区域作为参数，每个区域可能需要不同的属性。然而，目前实际上很少有需要这样做的场景。作为替代，可以用附加在包含区域的实体（操作或函数）上的数组属性来模拟每个区域的属性。这降低了 IR 的整体复杂性，并支持更简洁的、操作特定的形式，例如当某个操作的所有区域都具有相同属性时，只需提及一次即可。由于区域的语义完全由其封闭实体定义，将属性附加在该实体而非区域本身上也更为合理。

如果将来发现大量实际使用场景，可以重新考虑此决定。

### 外部函数的 Read/Write/May_Read/May_Write 集合

为外部函数（包括不透明的函数、高性能供应商库如 CuDNN、CuB、MKL、FFT 库、用户提供/优化的函数，或 DMA 等数据移动运行时）提供 read、write、may_read 和 may_write 集合是一个强大的特性。它允许编译器在存在此类调用以及在子张量上围绕此类调用的循环时执行分析、组合/变换。对于用户提供或自定义手工调优的函数，read/write/may_read/may_write 集合可由用户作为外部函数签名的一部分先验地提供，也可以是数据库的一部分。

TODO：设计此特性，并更新以使用函数属性语法。

示例：

```mlir
##rel9 ( ) [s0] -> (r0, r1) : 0 <= r0 <= 1023, 0 <= r1 <= s0 - 1

func.func @cblas_reduce_ffi(%M: memref<1024 x ? x f32, #layout_map0, /*mem=*/0>)
  -> f32 [
  reads: {%M, ##rel9() }
  writes: /* empty */
  may_reads: /* empty */
  may_writes: /* empty */
]

func.func @dma_mem_to_scratchpad(%a : memref<1024 x f32, #layout_map0, /*mem=*/0>,
    %b : memref<1024 x f32, #layout_map0, 1>, %c : memref<1024 x f32,
    #layout_map0>) [
  reads: {%M, ##rel9() }
  writes: /* empty */
  may_reads: /* empty */
  may_writes: /* empty */
 ]

```

### Memref 扩展

1.  张量的任意多面体形状：例如，当张量维度具有对称性时采用三角形状：使用整数集（仿射约束）对张量数据空间建模（而不仅仅是范围）。需要对 IR 和内存中的形式进行一些修改。
1.  布局映射（Layout map）

    1.  允许分片仿射映射（piece-wise affine map）用于布局：允许通过填充（padding）、环绕（wrapping）、镜像（mirroring）以及填充值为计算结果（而非数据）和内部填充（而非仅边界填充）来清晰地对图像/张量的边界情况建模。
    1.  允许多对一布局映射：当前提案中的索引和布局映射是双射的。将其扩展为多对一布局映射，可以在复用内存的同时更清晰地（？）对广播/规约计算建模。

    方案 2(a) 需要对 IR 和内存中的表示进行重大修改。2(b) 无需修改，但会影响代价模型查看索引和布局映射的方式。

### `affine.if` 和 `affine.for` 对"逃逸标量"的扩展

我们曾考虑为在 `if/else` 条件体中存活（live-out）以及在 `affine.for` 循环中携带（loop carried）的 SSA 值提供表示。由于其复杂性，我们最终放弃了这一方案。在 MLIR 的当前设计中，标量变量无法逃逸 for 循环或 if 指令。在需要逃逸的情况下，我们使用零维张量和 memref 来代替标量。

**TODO**：本节已过时，应更新以使用块参数和 for/if 指令中类似 yield 的终结符。

已放弃的支持逃逸标量的设计如下：

#### affine.for 指令

语法：

```
[<out-var-list> =]
for %<index-variable-name> = <lower-bound> ... <upper-bound> step <step>
   [with <in-var-list>] { <loop-instruction-list> }
```

out-var-list 是在循环体中定义并在循环体外使用的 SSA 值的逗号分隔列表。in-var-list 是在循环体内使用的 SSA 值及其初始化器的逗号分隔列表。loop-instruction-list 是可能包含 yield 指令的指令列表。

示例：

```mlir
// Return sum of elements in 1-dimensional mref A
func.func i32 @sum(%A : memref<?xi32>, %N : i32) -> (i32) {
   %init = 0
   %result = affine.for %i = 0 to N with %tmp(%init) {
      %value = affine.load %A[%i]
      %sum = %value + %tmp
      yield %sum
   }
   return %result : i32
}
```

#### affine.if/else 指令

语法：

```
<out-var-list> = affine.if (<cond-list>) {...} [else {...}]
```

out-var-list 是由 if 指令定义的 SSA 值列表。当存在 else 子句时，这些值是出现在 then 和 else 子句中的 yield 指令的参数。当 if 指令只包含 if 子句时，then 子句中定义的逃逸值应与 if 指令之前该变量所持有的值合并。此处记录的设计未处理这种情况。

示例：

```mlir
// Compute sum of half of the array
func.func i32 @sum_half(%A : memref<?xi32>, %N : i32) -> (i32) {
   %s0 = 0
   %s1 = affine.for %i = 1 ... N step 1 with %s2 (%s0) {
       %s3 = if (%i >= %N / 2) {
          %v0 = affine.load %A[%i]
          %s4 = %s2 + %v0
          yield %s4
       }
       yield %s3
   }
   return %s1 : i32
}
```

### 编译器的多线程化

人们希望编译器运行得更快，一种简单的方式是对其进行多线程化。实现方式有多种，其中一种简单的方式是并行地优化和编译独立的函数。LLVM 最初的遍（pass）管理器就预见了这一需求，CallGraphSCCPass 管理器甚至也被设计为支持此特性，但遗憾的是，LLVM 早期的几个设计决策使这一目标始终无法实现。为此，ThinLTO 等方案不得不将程序拆分为独立的 LLVM 模块/上下文并分别独立地优化这些块。

问题在于 LLVM 的 IR 中存在若干全局唯一化（globally uniqued）且可变的对象，最典型的是 `i32 0` 这样的常量。在 LLVM 中，这些常量是 `Value`，可以作为指令的操作数，并且具有 SSA 使用列表（use list）。由于这些对象是唯一化的，任何函数中所有的 `i32 0` 共享同一个使用列表。这意味着并行地优化多个函数是行不通的（至少在没有某种使用列表同步机制的情况下如此，而这种同步开销将是无法接受的）。

MLIR 目前支持多线程化的遍管理器，通过以下几项设计选择实现：

1.  MLIR 大量使用唯一化的不可变数据结构（仿射表达式、类型等均是不可变的、唯一化的且不会被释放的）。
2.  常量在每个操作的池中定义，而非全局唯一化。
3.  函数及其他类全局操作本身也不是 SSA 值，因此不存在与常量相同的问题。
4.  遍通过拷贝构造函数（copy ctor）被复制为每个线程一个实例，避免了线程间共享局部状态。

这使得 MLIR 的遍能够支持高效的多线程编译和代码生成。
