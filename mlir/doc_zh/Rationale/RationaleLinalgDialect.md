# Linalg 方言设计理由：编译器友好自定义操作的案例

[TOC]

## 简介<a name="introduction"></a>

### 定位

<img width="180" align="left" alt="MLIR Codegen Flow" src="https://user-images.githubusercontent.com/10148468/73613629-c5586580-45c5-11ea-94b7-074aeea94c7b.png">

本文档描述了促成 Linalg 现有实现的关键设计原则，旨在揭示构建更高层次中间表示（IR，Intermediate Representation）和方言（dialect）以促进代码生成时所涉及的权衡取舍。请参考描述 MLIR 代码生成流程的简化模式图。Linalg 的设计目标是解决高层次分层优化（HHO 框）问题，并在*混合专家编译器*环境（即 *CGSel* 框）中良好互操作。这项工作受到领域内大量[先验技术（prior art）](#prior-art)的启发，并从中汲取关键经验教训。本文档和反思工作也发生在一个关于[高层张量计算原语方言及变换](https://llvm.discourse.group/t/development-of-high-level-tensor-compute-primitives-dialect-s-and-transformations/388/3)开发的工作组提案背景下。我们希望先验技术的经验、本文档阐述的设计原则以及 Linalg 的架构，能够帮助社区找到定义这些高层张量计算原语的路径。

### 起源

Linalg 最初作为一个务实的方言启动，以引导 MLIR 中的代码生成。其做法是*通过定义绕开*复杂的代码生成问题（如精确依赖分析或多面体代码生成），并在快速库实现可用时引入调用它们的能力。Linalg **以声明方式定义操作（op）和变换**，最初限于具有*类线性代数*语义的操作（`pointwise`、`matmul`、`conv` 等）。这种方法使得构建高层生产力优先的代码生成解决方案成为可能，该方案同时利用*编译器优化*和*高效库实现*，从而不会错过简单的性能收益。例如，如果某人最喜欢的 HPC 库或 ISA 在某些内存中存储的操作数上拥有运行在 95% 可达峰值性能的 `matmul` 原语，就应该能够在可能时**使用该原语**，*否则*生成代码。

然而，随着 Linalg 的设计与 MLIR 设计共同演进，越来越明显的是，它可以扩展到比仅密集张量机器学习更广泛的应用领域。

Linalg 的设计和演进遵循*代码生成友好*的方法，其中 IR 和变换协同演进。核心思想是操作语义*声明*并传递传统上通过编译器分析获得的信息。这些信息捕获变换的合法性和适用性，且**不会因过早降级到循环或 CFG（控制流图）形式而丢失**。关键变换被设计为**尽可能长地保留这些信息**。例如，`linalg.matmul` 在分块（tiling）和融合（fusion）后仍然是 `linalg.matmul`。

此外，Linalg 将变换的有效性与盈利能力考虑解耦，并在第一次迭代中主动搁置后者（参见[搜索适用性](#suitability_for_search)指导原则）。

这些想法的第一个化身在 EuroLLVM 2019 开发者会议上作为示例呈现，作为第一届 [MLIR 教程](https://www.youtube.com/watch?v=cyICUIZ56wQ)的 [Linalg 部分](https://llvm.org/devmtg/2019-04/slides/Tutorial-AminiVasilacheZinenko-MLIR.pdf)的一部分。

### 演进

自初始实现以来，设计随着 MLIR 核心基础设施的演进而发展，并部分推动了其演进，使用了[区域（Regions）](../LangRef.md/#regions)、[操作接口（OpInterfaces）](../Interfaces.md)、[ODS](../DefiningDialects/Operations.md)和[声明式重写规则（Declarative Rewrite Rules）](../DeclarativeRewrites.md)等。Linalg 采用的方法被扩展为[StructuredOps 抽象](https://drive.google.com/drive/u/0/folders/1sRAsgsd8Bvpm_IxREmZf2agsGU2KvrK-)，Linalg 成为其在张量和缓冲区上的具体化。它由[向量方言（Vector dialect）](../Dialects/Vector.md)补充，该方言在向量上定义结构化操作，遵循与 Linalg 相同的设计理由和原则。（向量方言包括多维向量上的高层操作，并抽象化了到一维向量的降级。）

Linalg 方言本身超越了类线性代数操作，变得更具表达力，特别是通过提供围绕任意 MLIR [区域](../LangRef.md/#regions)支持并行、归约和滑动窗口的循环嵌套抽象。它还有超越*密集*线性代数以支持更丰富数据类型（如稀疏和锯齿张量及缓冲区）的潜力。

Linalg 设计对演进和与其他方言及方法的交叉融合保持开放。它已被成功用作代码生成相关抽象的试验场，衍生出以下泛化：
- `!linalg.view` 类型折叠进*"步幅 MemRef"*类型，同时保留结构以允许使用不出人意料的 ABI 约定调用外部 C++ 库；
- `linalg.view` 和 `linalg.subview` 操作演变进入标准方言；
- `linalg.for`、`linalg.load` 和 `linalg.store` 操作演变成为*结构化控制流*方言（名为 `LoopOps`）的前身。
当出现新用途或需求时，可以提取、重新设计和泛化更多组件。

Linalg 中仍有若干[设计问题](../Dialects/Linalg/_index.md/#open_issues)悬而未决，Linalg 并不声称是解决所有编译问题的通用方案。它的目标是推动思考和实现领域特定抽象，这些抽象可以直接在 IR 中以极高层次捕获程序员的意图。

鉴于范围的演变，越来越明显的是，比"Linalg"更好的名称可以消除与该方言（及其底层方法）、目标和局限性相关的一些混淆。

## 先验技术

Linalg 从数十年的先验技术中汲取灵感，设计出一个现代务实的解决方案。以下非详尽列表提及了影响 Linalg 设计的部分项目：

- [ONNX](https://onnx.ai/)，
- [LIFT](https://lift-project.github.io/)，
- [XLA](https://www.tensorflow.org/xla/architecture)，
- [Halide](https://halide-lang.org/) 和 [TVM](https://tvm.apache.org/)，
- [TACO](http://tensor-compiler.org/)，
- [Darkroom](http://darkroom-lang.org/) 和 [Terra](http://terralang.org/)，
- [Sigma-LL](http://spiral.ece.cmu.edu:8080/pub-spiral/pubfile/cgo16-preprint_248.pdf)，
- [Tensor Comprehensions](https://arxiv.org/abs/1802.04730)，
- [多面体编译器（Polyhedral Compilers）](https://en.wikipedia.org/wiki/Polytope_model)，
- MLIR 中的 [Affine 方言](https://mlir.llvm.org/docs/Dialects/Affine/)，
- 通用循环变换（参见 Ken Kennedy 的[面向现代架构的优化编译器](https://www.elsevier.com/books/optimizing-compilers-for-modern-architectures/allen/978-0-08-051324-9)）
- 传统编译器 CFG 与 SSA 形式。

此外，在整体思考这些组件如何从用户到硬件全程相互作用时，以下工具的经验被证明极为宝贵：

- [Torch](http://torch.ch/) 机器学习框架，
- LLVM 编译器（特别是 JIT 模式），
- 高性能库（MKL、CUBLAS、FBFFT），
- [PeachPy](https://www.cs.utexas.edu/users/flame/BLISRetreat/BLISRetreatTalks/PeachPy.pdf) 汇编器，
- 当前和潜在的未来硬件 ISA。

MLIR 代码库的新颖性及其对定义和混合抽象的前所未有的支持，使我们能够反思并整合先验技术成功的关键要素，同时避免代码生成领域的常见陷阱。因此，Linalg 没有分散到关于采用任何现有解决方案影响的讨论中，而是有机会在所有这些方案的基础上构建，从它们的经验中学习，同时利用后见之明的优势。

以下对先验技术的反思影响了 Linalg 的设计。这些讨论并非详尽无遗，但应能捕捉到 Linalg 背后的关键动机。

### ONNX 的经验<a name="lessonsonnx"></a>
ONNX 是机器学习工作负载中出现的操作规范。因此，它主要受 ML 表达性需求驱动，对 HPC 代码生成的 IR 设计考虑较少。

与 ONNX 类似，Linalg 定义了*"语义丰富的"命名操作*。但它也将*这些操作上的变换*视为关键组件，并设计 IR 以支持这些变换，必要时优先考虑变换而非表达性。

Linalg 还希望额外解决以下问题：
- 通过在操作定义中考虑编译器变换和降级，促进前端-编译器协同设计；
- 通过使操作相互不重叠来最小化可用操作集，从而简化中间表示。

### LIFT 的经验<a name="lessonslift"></a>
[LIFT](https://lift-project.github.io/) 是一个基于函数式抽象编写计算核的系统。变换由 IR 中的附加节点表示，其语义处于算法层次（例如 `partialReduce`）。LIFT 通过使用[局部重写规则](https://lift-project.github.io/publications/2015/steuwer15generating.pdf)应用和组合变换，将这些附加节点直接嵌入函数抽象中。

与 LIFT 类似，Linalg 使用 MLIR [声明式重写规则](../DeclarativeRewrites.md)机制实现的局部重写规则。

Linalg 在 LIFT 方法的基础上构建，并如下分离关注点：
- 变换要么与表示分离，要么表示为独立于实际计算的可组合属性，避免对性能产生复杂影响；
- 抽象被分解为更小的组件（例如控制流和数据结构抽象），可能在 MLIR 开放生态系统中的不同方言之间复用。

LIFT 预计将随着其演进继续影响 Linalg 的设计。特别是，将数据结构抽象扩展到支持非密集张量，可以利用 LIFT 对[稀疏](https://lift-project.github.io/publications/2016/harries16sparse.pdf)和[位置相关数组](https://lift-project.github.io/publications/2019/pizzuti19positiondependentarrays.pdf)的抽象经验。

### XLA 的经验<a name="lessonsxla"></a>
[XLA](https://www.tensorflow.org/xla/architecture) 是后 Theano 时代第一批 ML 编译器之一，作为 TensorFlow 的务实编译解决方案引入。它在 Google 的 xPU 硬件上表现出色，是整个拼图的重要一环。它特别擅长：(1) 在标量世界和向量世界之间来回变换代码，(2) 跨函数边界处理主机和设备代码，(3) 遵守节能 xPU 施加的严格要求。XLA 遵循务实的设计流程，编译器对每个操作的语义拥有完美的了解，一切都从强大的 `conv` 和 `matmul` 操作开始。XLA 变换包括编写以 C++ 函数形式组合的发射器。完美的操作语义知识有两大好处：(1) 变换按构建即正确，(2) 在困难的 xPU 目标上性能非常强劲。

类似地，Linalg 操作*"了解自身语义"*并*"了解如何变换和降级自身"*。然而，在 MLIR 中提供和使用此信息的方式却大相径庭。

Linalg 还希望额外解决以下问题：
- HLO 作为整体具有表达力，但每个操作的语义非常有限且固定：操作不可配置。因此，HLO 演变成了一个过大的操作集，其语义相互交叉。这与 ONNX 表现出的操作激增问题相呼应。
- 依赖完美的操作知识会导致变换和操作最终需要了解彼此的语义（例如在融合期间）。由于变换本身不是简单的局部重写模式（与 LIFT 不同），代码复杂性迅速增长。
- XLA 缺乏可以独立检查、单元测试和使用的独立 IR。这种单体设计使系统不可移植：xPU 流水线和 GPU 流水线共享的代码很少。

### Halide 和 TVM 的经验<a name="lessonshalide"></a>
[Halide](https://halide-lang.org/) 是一种嵌入 C++ 中的 DSL，提供了一种对 HalideIR 进行元编程并以声明方式应用变换的方法，让专家用户能够以定制方式变换和优化程序。Halide 最初面向 SIGGRAPH 社区，但现在更具普遍适用性。[TVM](https://tvm.apache.org/) 是 Halide 在机器学习和深度神经网络领域的演进，基于 HalideIR。

Halide 的变换方法论遵循与 [URUK](http://icps.u-strasbg.fr/~bastoul/research/papers/GVBCPST06-IJPP.pdf) 和 [CHiLL](https://pdfs.semanticscholar.org/6a46/20589f63f3385707d2d590f7b7dc8ee4d74f.pdf) 编译器变换框架类似的原则，但没有多面体模型的优势（尤其是复杂性）。

Halide 特别擅长使 HPC 变换方法论对 $\Omega$(10-100) 量级的用户可访问，而多面体工具仍然只对 $\Omega$(1-10) 量级的用户可访问。Halide 大量使用规范化规则，这在 MLIR 中也非常普遍。

Linalg 还希望额外解决以下问题：
- Halide 调度功能强大，探索了大量可能的变换。但对于新手来说仍然太难使用或扩展。从 Halide 获得的性能水平因用户是经验丰富的老手还是新手而大相径庭。随着变换数量的增加，这一点尤为突出。
- Halide 从两个方面提升而非降低，这与我们为 MLIR 中高层代码生成抽象设定的设计目标相悖。首先，规范 Halide 前端代码使用显式索引和标量值上的数学，因此要面向 BLAS/DNN 库，需要添加模式匹配，这与仿射情况一样脆弱。虽然 Halide 在可编程目标（CPU/GPU）上的性能与库相当，但这种方法在移动加速器或 xPU 上行不通，因为这些框架接收的是整张量操作。其次，归约和扫描使用串行迭代表达，再次需要模式匹配才能对其进行变换（例如使用原子操作进行归约，或分层归约）。应吸取的教训是我们应该从比 Halide 更高层次的原语开始。

### Tensor Comprehensions 的经验<a name="lessonstc"></a>
[Tensor Comprehensions](https://arxiv.org/abs/1802.04730) 是一种使用推广爱因斯坦记法语法表达张量计算的高级语言，结合了能够降级到高效 GPU 代码的端到端编译流程。它与两个 ML 框架集成：Caffe2 和 PyTorch。

<img width="600" alt="MLIR Codegen Flow"
src="https://user-images.githubusercontent.com/10148468/73613272-df904480-45c1-11ea-88f9-214dee7464cf.png">

编译流程结合了 [Halide](#lessonshalide) 和从 [ISL](https://en.wikipedia.org/wiki/Integer_set_library) 派生的多面体编译器，并同时使用 HalideIR 和 ISL *调度树* IR。编译器提供了一系列多面体编译算法，以执行融合并支持多层并行和提升到更深层次的内存层次结构。Tensor Comprehensions 表明，固定几种带有参数变换和调优旋钮的预定义策略，已经可以提供很好的结果。在之前的工作中，简单的遗传搜索结合自动调优框架足以在***非计算密集区域***找到良好的实现。这需要各种变换可获得的代码版本能够涵盖接近屋顶线限制的版本。Tensor Comprehensions 的终极目标是将 Halide 高层变换与多面体中层变换具体混合，构建一个能够利用两种编译风格优势的务实系统。

Linalg 还希望额外解决以下问题：
- Halide 在 Tensor Comprehensions 中从未被适当地用于形状推断之外。大部分投资都用于简化多面体变换和构建可用的端到端系统。MLIR 被认为是混合这类编译的更好基础设施。
- 重用已建立基础设施（HalideIR 和 ISL 调度树）带来的早期收益变成了比小的战术投资所能解决的更多的阻抗不匹配问题。
- Tensor Comprehensions 发出 CUDA 代码，然后通过文本表示由 NVCC 即时编译。虽然这是一个短期务实的解决方案，但它使执行低层重写变得困难，而这些重写本可以帮助在***计算密集区域***实现寄存器复用。
- 同样对发出 CUDA 代码的依赖使得在需要时难以创建代价模型，这使得剪枝不良解决方案比必要的更加困难，导致过度的运行时评估，正如论文[机器学习系统陷入瓶颈](https://dl.acm.org/doi/10.1145/3317550.3321441)所报告的。

在 MLIR 基础设施中实现这些思想可以自然地解决许多这些问题。

### 多面体编译器的经验<a name="lessonspolyhedral"></a>
多面体模型在循环级优化领域处于前沿数十年，在生产编译器中有多种具体化形式，如 GCC 的 [GRAPHITE](https://gcc.gnu.org/wiki/Graphite) 和 LLVM 的 [Polly](https://polly.llvm.org)。尽管它对于从 [PolyMage](http://mcl.csa.iisc.ac.in/polymage.html) 和 [Tensor Comprehensions](https://dl.acm.org/doi/abs/10.1145/3355606) 等领域特定语言生成高效代码已被证明至关重要，但它从未完全融入主流通用优化流水线。关于多面体变换角色的详细分析在 [MLIR 诞生之初的简化多面体形式](RationaleSimplifiedPolyhedralForm.md)文档中提供。

特别是，由于以下原因，多面体抽象被证明难以与更传统的编译器集成：
- 变换后的代码（或 IR）很快变得复杂，难以分析和理解；
- 从多面体模型中使用的数学形式生成代码依赖于非平凡的指数复杂度算法；
- 数学形式很少与 SSA 表示及相关算法可组合，而大多数主流编译器今天都构建在这些之上；
- 表达性限制，尽管在科学文献中通过例如摘要函数等方法加以解决，但在实际实现中通常仍然存在。

MLIR 中的 Affine 方言专门设计用于解决上述集成问题。特别是，它在整个变换过程中以相同的形式（带有关于边界如何表达的附加约束的循环）维护 IR，减少了在截然不同的表示之间进行一次性转换的需要。它还通过使用 MLIR 区域将多面体表示嵌入 SSA 形式，从而允许组合多面体和基于 SSA 的变换。

### Affine 方言的经验<a name="lessonsaffine"></a>
MLIR 中的 Affine 方言使多面体抽象更接近传统的 SSA 表示。它解决了上述若干长期存在的集成挑战，在从 C 语言级别抽象编译时可能更合适。

MLIR 使得从比 C 更高层次的抽象开始成为可能，例如在机器学习工作负载中。在这种情况下，通过利用更高抽象层次上可用的信息（类似于 DSL 编译器），可能可以避免多面体变换所需的复杂分析（跨循环迭代的数据流分析是指数复杂度的）。Linalg 打算在信息可用时使用这些信息，并通过将合法性前提条件集成到操作语义中，*通过构建确保变换的合法性*（例如，平铺可以应用于计算矩阵乘法的循环嵌套，无需额外依赖仿射依赖分析来检查这一点）。这些信息在 Affine 方言中不是现成可用的，只能使用潜在昂贵的模式匹配算法来推导。

基于多面体编译和特别是 Affine 方言的实践经验，Linalg 做出以下决策：
- **不鼓励循环倾斜（loop skewing）**：循环倾斜变换有时用于启用并行化，但通常对性能有出人意料的（负面）影响。特别是，多面体自动变换可以以更简单的方式表达而无需循环倾斜；倾斜通常会导致复杂的控制流，阻碍 GPU 等加速器上的性能。此外，循环倾斜解决的问题可以通过其他方法更好地解决，例如菱形平铺（diamond tiling）。在 ML 工作负载更受限制的情况下，感应变量相互独立的多重 for 循环（在文献中称为超矩形迭代域），如提议的 [affine.parallel](https://llvm.discourse.group/t/rfc-add-affine-parallel/350)，在大多数情况下已经足够。
- **声明式平铺（Declarative Tiling）**：*平铺*变换在 HPC 代码生成中无处不在。它可以看作是将迭代空间或数据空间分解为更小的规则部分（称为瓦片）。多面体方法（包括 Affine 方言）主要选择迭代空间平铺，这会引入额外的控制流和复杂的地址表达式。如果在变换期间瓦片大小未知（所谓参数化平铺），地址表达式和条件很快变为非仿射，或需要指数复杂度算法来推理。Linalg 转而专注于数据空间的平铺，创建利用 MLIR 步幅 `memref` 抽象的缓冲区视图。这些视图可以组合，访问表达式的复杂度保持可预测。
- **保留高层信息**：Linalg 在变换所需的时间内尽可能长地维护操作语义提供的信息。例如，对矩阵乘法进行平铺的结果是围绕较小矩阵乘法的循环。即使在 Affine 方言上进行模式匹配，这也需要变换后的另一步模式匹配。

鉴于这些选择，Linalg 打算在输入表示中有显著更多信息可用且应在降级到其他抽象之前利用的**高层编译**中发挥更好的作用。Affine 仍然是中层变换的强抽象，被用作 Linalg 的降级目标，实现进一步变换以及语义丰富和较低层次输入的组合。因此，Linalg 旨在补充 Affine 而非替代它。

## 核心指导原则<a name="guiding_principles"></a>

### 变换与简洁性优先<a name="transformations_first"></a>
Linalg IR 及其操作的目的主要是：
- 开发一套关键变换，
- 通过仔细策划驱动适用性的通用操作属性，使它们按构建即正确，
- 使它们的实现、应用、验证，特别是维护非常简单。

当前问题从根本上是由面向高性能和并行硬件架构的领域特定工作负载编译驱动的：**这是一个 HPC 编译问题**。

相关变换的选择遵循协同设计方法，涉及以下考量：
- 应用领域当前和未来的具体需求，
- 硬件当前和未来的具体属性和 ISA，
- 对[现有方法](#prior-art)优势和局限性的理解，
- 利用 MLIR 中多层 IR 共存的优势。

需要有条不紊地避免激增和冗余。某个变换可能存在于多个抽象层次，但**仅仅因为可以在 Y 层写 X 变换，绝对不意味着应该这样做**。这正是评估现有系统并承认其优势和弱点至关重要的地方：简洁性和可维护性必须是首要考量。没有这种额外的内省努力，设计将经不起时间的考验。同时，复杂性很难抵御。似乎需要经历复杂性，才能被促使退后一步重新思考抽象。

这不仅仅是在系统 Y 中重新实现思想 X：简洁性**必须是**这种内省努力的结果。

### 信息保留<a name="information_preservation"></a>
过去二十年见证了领域特定语言（DSL）的激增，这些语言在有限的应用领域非常成功。这些系统的主要共同点是它们使用比 CFG 或循环丰富得多的结构信息。然而，现有系统的另一个共同点是很快降级到 LLVM，并在一步之内跨越巨大的抽象鸿沟。这个过程通常会丢弃语义信息，而这些信息后来需要重建，如果没有不可挽回地丢失的话。

这些观察，加上 MLIR 适合在多个抽象层次定义 IR，导致了以下两个原则。

#### 声明式规范：避免提升<a name="declarative_specification"></a>

编译器变换需要静态结构信息（例如循环嵌套、基本块图、纯函数等）。当这些结构信息丢失时，需要重建。

这种现象的一个很好说明是多面体编译器中的*提升（raising）*概念：多种多面体工具首先从简化的 C 形式或 SSA IR 提升到更适合循环变换的高层表示。

在高级多面体编译器中，通常存在第二种类型的提升，用于检测特定模式（通常是 BLAS 的变体）。这些模式可能被变换破坏，使其检测非常脆弱甚至不可能（不正确）。

MLIR 通过使用区域和属性，使声明式定义操作语义变得容易。这是定义新抽象以将用户意图直接传达到适当抽象的理想机会。

#### 渐进式降级：不要过快丢失信息<a name="#progressive_lowering"></a>

过快降级到仿射、通用循环或 CFG 形式会减少可用于派生变换的结构量。虽然对于某类变换，操作循环比操作 CFG 形式净收益，但重要信息仍然丢失（例如并行循环，或循环嵌套到外部实现的映射）。

这会产生非平凡的阶段排序问题。例如，循环融合可能轻易破坏检测 BLAS 模式的能力。一种可能的替代方案是执行循环融合、平铺、瓦片内循环分布，然后希望检测 BLAS 模式。这种方案呈现出困难的阶段排序约束，这些约束可能会干扰其他决策和流水线。相反，某些 Linalg 操作被设计为在平铺和融合等变换中维护高层信息。

MLIR 被设计为***渐进式降级***的基础设施。Linalg 完全拥抱这一概念，从*降低势函数*的角度思考代码生成。该势函数宽泛地定义为特定 Linalg 操作中低层指令的数量（即 Linalg 操作有多"重"或多"轻"）。基于 Linalg 的代码生成和变换从更高层的 IR 操作和方言开始，然后每次变换应用通过引入更低层的 IR 操作和*更小*的 Linalg 操作来降低势，逐渐降低势，一直降到循环 + 向量操作（VectorOps）和 LLVMIR。

### 形式的可互换性<a name="forms"></a>

#### Linalg 形式

Linalg 核心操作集有四种形式：
* **通用（Generic）**：由 `linalg.generic` 表示，可以编码所有完美嵌套的循环操作。
* **类别（Category）**：例如 `linalg.contract` 和 `linalg.elementwise`，编码 `linalg.generic` 的更高层语义，同时仍通过属性和语法表示多个_命名_操作。未来计划添加其他类别操作（例如：`linalg.convolution` 和 `linalg.pooling`）。
* **命名（Named）**：例如 `linalg.matmul`、`linalg.add` 等。所有可以转换为单个_类别_或_通用_形式（即_完美嵌套_）的_命名_形式。
* **复合（Composite）**：例如 `linalg.softmax` 和 `winograd` 变体。这些操作不是完美嵌套的，被转换为其他操作的列表（来自各种方言）。

这些形式的关联如下：
```
+ generic
 \__ + category
      \__ + named
+ composite
```

`category` 和 `named` 形式派生自 `linalg.generic` 且是*等价的*。应该总是可以将 `named` 操作转换为 `category`，再转换为 `generic`，再回到 `named`。然而，如果不存在这样的 `named` 形式，可能无法将 `generic` 转换为 `named`。

`Composite` 操作不能转换为其他三类，自成一个子集。但在展开时可以使用其他 Linalg 形式。可以有模式匹配变换来检测操作图并转换为 `composite` 操作。

Linalg 方言中的各种形式旨在促进模式匹配（单个操作或 DAG），并能够将不同形式视为不同变换的*规范形式*。

Linalg 的各种形式也携带信息，在渐进式降级过程中应尽可能保留这些信息。`matmul` 操作是 `contract` 操作的特例，而 `contract` 操作又是 `generic` 操作的特例。Linalg 操作（任何形式）上的变换应该避免分解为循环 + 算术，如果它们仍然可以表示为 Linalg 操作（最好是其原始形式）。

#### 规范形式<a name="canonical_forms"></a>

有了多种（通常可互换的）形式，以及牢记变换简洁性，编译器应尽量减少匹配和替换的复杂度。当用复杂模式匹配单个操作时，在 `generic` 操作中拥有所有信息有助于依次迭代匹配不同的模式。然而，当组装操作 DAG 来形成模式时，匹配命名操作（如 `max` + `div` + `reduce` + `broadcast`）比匹配其通用对应物要简单得多。

这正是形式可互换性大显身手的地方。Linalg 具有特化和泛化的能力，以便将 IR 转换为特定类型变换更容易处理的形式。由于形式在语义上等价，可以在各种变换中来回转换，以匹配每个变换的需求。对于那个特定的变换，这种形式可以被认为是_规范的_，因此对于模式_匹配_来说是"预期的"。这降低了模式匹配器的复杂性，简化了编译器流水线。

### 可组合的声明式变换<a name="declarative_transformations"></a>
复杂且有影响力的变换不必难以操作、编写或维护。将 XLA 风格的高层操作语义知识与描述这些语义的通用属性直接混合在 MLIR 中，是一种有前途的方式：
- 设计按构建即正确、易于编写、易于验证和易于维护的变换；
- 提供一种方式以声明方式指定变换及其操纵的 IR 单元，从而允许在 MLIR 中使用局部模式重写规则（即 [DRR](../DeclarativeRewrites.md)）；
- 允许通过简单地选择重写规则以声明方式创建可定制的流水线，允许在单个流水线中混合变换、规范化、常量折叠和其他启用重写。结果是一个流水线融合非常容易获得的系统，并为解决某些[阶段排序问题](https://dl.acm.org/doi/10.1145/201059.201061)带来希望。

### 搜索和机器学习的适用性<a name="ml"></a>
编译器启发式是手工制作的人工工程特征：它已成熟到可以被机器学习技术颠覆的地步。为了启用搜索，编译器变换应该是细粒度的、[可组合的](#declarative_transformations)，并暴露可以修改其行为的调优参数，这一点受到之前 [Tensor Comprehensions](#lessonstc) 经验的指导。

当然，我们并不主张立即在堆栈中的所有地方使用 ML：低层编译和机器模型在 LLVM 中仍然相当高效。然而，对于高层和中层优化问题，模型需要（概率地）以充当黑盒的低层编译器为条件。出于这些原因，我们优先设计具有搜索友好属性的 IR 和变换，而不是构建代价模型。尽管如此，这并不意味着 Linalg 拒绝代价模型：相反，我们更愿意投资于能够[使 ML 技术自动构建代价模型](http://homepages.inf.ed.ac.uk/hleather/publications/2009_autofeatures_cgo.pdf)的基础设施。

### 可扩展性和面向未来<a name="future"></a>
MLIR 允许为结构化控制流和结构化数据类型定义 IR。出于上述原因，我们选择利用这些属性。特别是，`MemRefType` 表示密集的非连续内存区域。这种结构应该超越简单的密集数据类型，泛化到锯齿、稀疏和混合密集/稀疏张量，以及树、哈希表、记录表，甚至图。

对于这些更高级的数据类型，遍历数据结构所需的控制流、终止条件等静态分析和表征起来要复杂得多。因此，我们还需要设计有机会演进为运行时自适应计算的解决方案（例如巡查-执行器，其中*巡查器*对数据进行廉价的运行时分析以配置*执行器*的运行方式）。虽然今天在 MLIR 中还没有解决这些问题的具体解决方案，但很明显，完美的静态知识和分析不会是这些问题的有力竞争者。

## 关键观察<a name="keyobservation"></a>
以下关键观察影响了 Linalg 的设计，并帮助将[核心指导原则](#guiding_principles)与基于 MLIR 实现时的现实需求相协调。

### 算法 + 数据结构 = 程序<a name="data_and_compute"></a>

这是对 Niklaus Wirth 表述的变体，但捕捉了 Linalg 设计的本质：控制流不是独立于数据存在的。相反，控制流和数据结构之间有非常强的关系：没有对方就无法存在。这对 [Linalg 操作的语义](../Dialects/Linalg/_index.md/#linalg_ops)及其变换有多重影响。特别是，这一观察影响某些变换是否更好地以控制流或数据结构操作的方式进行，在 Linalg 操作属性上还是在部分降级发生后的循环上，以及作为新操作或属性对 Linalg 方言的扩展。

### 方言无需在变换下封闭<a name="dialect_not_closed"></a>
这可能是最令人惊讶和违反直觉的观察。在为变换设计 IR 时，封闭性通常是不可谈判的属性。这是多面体 IR 的关键设计原则，如 [URUK](http://icps.u-strasbg.fr/~bastoul/research/papers/GVBCPST06-IJPP.pdf) 和 [基于 ISL 的 IR](https://en.wikipedia.org/wiki/Integer_set_library)：它们在仿射变换下是封闭的。在 MLIR 中，多个方言共存并形成一个连贯的整体。在尝试了不同的替代方案后，很明显严格的方言封闭性不是必要的，可以放宽。之前的系统没有构建新 IR 的简单和有原则的方法，可能因这种限制而受损。我们推测这是它们需要 IR 在变换下封闭的关键原因。

尽管 Linalg 操作只允许完美嵌套的语义，一旦平铺和融合开始，不完美嵌套的循环逐渐被引入。换句话说，不完美嵌套的控制流作为***应用关键变换的结果***出现。

考虑到[渐进式降级](#progressive_lowering)讨论中描述的*势*，在变换下的封闭性将要求势保持不变。相反，Linalg 倡导变换下的***单调性***。

### 现有替代方案总结<a name="observationssummary"></a>
最后，我们通过以下图示总结从[先验技术](#prior-art)汲取的经验教训观察——在我们[核心指导原则](#guiding_principles)的视角下。

<img width="1200" alt="MLIR Codegen Flow"
src="https://user-images.githubusercontent.com/10148468/73613904-2f720a00-45c8-11ea-8265-1c856c02525b.png">

此图并非旨在完全准确，而是从代码生成友好角度粗略描绘我们如何看待现有系统中结构信息的分布。不出所料，[Linalg 方言](../Dialects/Linalg/_index.md)及其未来演进的目标是在此图的右上角。
