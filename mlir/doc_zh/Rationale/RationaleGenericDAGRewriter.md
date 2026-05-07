# 通用 DAG 重写基础设施设计原理

本文详述 MLIR 通用 DAG 到 DAG 重写基础设施的设计依据。有关面向用户的最新 API 文档，请参阅主要的[模式重写文档](../PatternRewriter.md)。

## 介绍与动机

编译器 IR 的目标是表示代码——在各种抽象层次上，这些层次在表示能力和变换便捷性方面各有不同的权衡取舍。然而，仅仅能够表示代码本身并没有多大用处——你还需要能够实现那些变换。

编译器变换有许多不同类型，但本文聚焦于一类在规模化场景中反复出现的特别重要的变换，对于 MLIR 的目标也至关重要：匹配一个操作的 DAG，并用另一个替换它。这是许多编译器的组成部分，对于"消除恒等节点"或"将 x+0 替换为 x"等窥孔优化、通用规范化框架（例如 LLVM 中的指令组合器）不可或缺，同时也是在多个层次的 IR 上实现优化算法的一种有用抽象。

MLIR 的一个特殊优势（也是与 LLVM、GCC、XLA、TensorFlow 等其他编译器基础设施的主要区别）在于，它使用单一编译器 IR 在多个抽象层次上表示代码：一个 MLIR 操作可以是"TensorFlow 操作"、"XLA HLO"、Affine 循环嵌套、LLVM IR 指令（间接包括 X86、Lanai、PTX 和其他目标特定指令），或者任何 MLIR 操作系统能够合理表达的其他内容。鉴于 MLIR 涵盖如此广泛的不同问题范围，用于执行图到图重写的单一基础设施可以帮助解决许多不同领域的挑战。

[静态单赋值](https://en.wikipedia.org/wiki/Static_single_assignment_form)（SSA）表示（如 MLIR）使得访问操作的操作数和"使用者"变得容易。因此，这些图到图重写的自然抽象是 DAG 模式匹配：客户端定义 DAG 块模式（其中块是定义 DAG 子图的操作序列），每个模式包含要生成的结果 DAG 以及结果的代价（或相反地，执行替换的收益）。通用基础设施高效地查找并执行重写。

虽然这个概念很简单，但细节更为微妙。本文定义并探索了一组能够解决大范围不同问题的抽象，并可应用于 MLIR 当前面临和预期将面临的许多不同类型问题。我们通过将模式应用算法与计算循环的"驱动器"分离来实现这一点，并为以声明方式定义模式留出空间。

### 常量折叠

DAG 到 DAG 模式匹配的一个退化但普遍的情形是常量折叠：操作数包含常量的操作通常可以折叠为一个结果常量值。

MLIR 操作可以重写一个 [`fold`](../Canonicalization.md/#canonicalizing-with-the-fold-method) 例程，与通用 DAG 到 DAG 模式匹配器相比，它提供了更简单的 API，并允许在通用匹配器无法适用的情况下使用。例如，DAG 重写可以删除当前函数中的任意节点，这可能会使迭代器失效。而常量折叠作为一种 API，不会删除任何节点，它只提供一个（或多个）常量值，并允许客户端根据需要更新其数据结构。

## 相关工作

这里有大量相关工作需要考虑，因为几乎所有现存的编译器都必须多次解决这个问题。一个共同的问题是，所有这些系统都被设计为解决一个特定且通常较窄的问题：而 MLIR 则希望在单一基础设施中解决其中许多问题。以下是一些相关的图重写系统，以及它们工作的优缺点（与 MLIR 中基础设施最为相似的设计是 LLVM DAG 到 DAG 指令选择算法）。

### AST 层级模式匹配器

文献中充满了源到源翻译器，这些翻译器通过变换等式来提升性能（例如将 `X*0` 转换为 `0`）。一个典型的例子是 GCC 的 `fold` 函数，它对 AST 执行[许多优化](https://github.com/gcc-mirror/gcc/blob/master/gcc/fold-const.c)。Clang 有[类似的例程](https://clang.llvm.org/docs/InternalsManual.html#constant-folding-in-the-clang-ast)用于表达式的简单常量折叠（C++ 标准要求），但不对其 AST 执行通用优化。

AST 优化器的主要缺点是无法跨具有多个使用的操作进行观察。[文献中广为人知](https://llvm.org/pubs/2008-06-LCTES-ISelUsingSSAGraphs.pdf)的是，DAG 模式匹配比树模式匹配更为强大，但另一方面，DAG 模式匹配可能导致需要检查的计算重复。

### "组合器"和其他窥孔优化器

编译器最终会积累大量用于各种目的的窥孔优化器，例如 GCC 的 ["combine" 例程](https://github.com/gcc-mirror/gcc/blob/master/gcc/combine.c)（试图将两条机器指令合并为一条）、LLVM 的 [Inst Combine](https://github.com/llvm/llvm-project/tree/main/llvm/lib/Transforms/InstCombine) [pass](https://llvm.org/docs/Passes.html#instcombine-combine-redundant-instructions)、LLVM 的 [DAG Combiner](https://github.com/llvm-mirror/llvm/blob/master/lib/CodeGen/SelectionDAG/DAGCombiner.cpp)、Swift 编译器的 [SIL Combiner](https://github.com/apple/swift/tree/main/lib/SILOptimizer/SILCombiner) 等。这些通常匹配一个或多个操作，并产生零个或多个操作作为结果。LLVM 的 [Legalization](https://github.com/llvm/llvm-project/tree/main/llvm/lib/CodeGen/SelectionDAG) 基础设施有不同的外层循环，但工作方式相同。

这些 pass 差异很大，但也有统一的结构：它们大多有一个工作列表外层循环来访问操作。然后使用访问者模式（或等价形式）切换操作类别并分派到某个方法。该方法包含一长串手写 C++ 代码，对各种特殊情况进行模式匹配。LLVM 引入了一个"match"函数，允许使用模板元编程以更具声明式风格编写模式（MLIR 有类似的工具）。这里有一个简单的例子：

```c++
  // Y - (X + 1) --> ~X + Y
  if (match(Op1, m_OneUse(m_Add(m_Value(X), m_One()))))
    return BinaryOperator::CreateAdd(Builder.CreateNot(X), Op0);
```

这里有一个稍微复杂一点的例子（这并不是最大或最复杂的 :)

```c++
  // C2 is ODD
  // LHS = XOR(Y,C1), Y = AND(Z,C2), C1==(C2+1) => LHS == NEG(OR(Z, ~C2))
  // ADD(LHS, RHS) == SUB(RHS, OR(Z, ~C2))
  if (match(LHS, m_Xor(m_Value(Y), m_APInt(C1))))
    if (C1->countTrailingZeros() == 0)
      if (match(Y, m_And(m_Value(Z), m_APInt(C2))) && *C1 == (*C2 + 1)) {
        Value NewOr = Builder.CreateOr(Z, ~(*C2));
        return Builder.CreateSub(RHS, NewOr, "sub");
      }
```

这些系统易于搭建，模式匹配模板有一些优势（对于新的子模式类型可扩展，在使用点看起来紧凑）。另一方面，它们有许多众所周知的问题，例如：

*   这些模式非常容易出错，并且包含大量冗余。
*   被匹配的 IR 通常具有等式（例如在匹配可交换运算符时），C++ 代码必须手动处理——看看[完整代码](https://github.com/llvm/llvm-project/blob/c0b5000bd848303320c03f80fbf84d71e74518c9/llvm/lib/Transforms/InstCombine/InstCombineAddSub.cpp#L767)中定义第二种模式的 `checkForNegativeOperand`。
*   匹配代码编译缓慢，既因为它生成大量代码，也因为模板实例化缓慢。
*   添加新模式（例如在上面的例子中为前导零计数添加模式）很麻烦，且往往不会发生。
*   这些模式的代价模型并没有真正定义——它是根据代码中匹配模式的顺序涌现出来的。
*   不重新编译编译器就无法扩展。
*   将定理证明器和其他工具应用于这些模式并不实际——它们无法被重复用于其他目的。

除了这些结构化的"组合器"之外，还有很多临时性的系统，例如 [LLVM 机器码窥孔优化器](http://llvm.org/viewvc/llvm-project/llvm/trunk/lib/CodeGen/PeepholeOptimizer.cpp?view=markup)。

### LLVM 的 DAG 到 DAG 指令选择基础设施

LLVM 中的指令选择子系统是多年迭代和探索的结果，驱动力来自于 LLVM 需要支持许多目标的代码生成、现代指令集（例如 X86）代码生成器的复杂性，以及对跨目标代码复用的狂热追求。Eli Bendersky 撰写了一篇[简洁的概述](https://eli.thegreenplace.net/2013/02/25/a-deeper-look-into-the-llvm-code-generator-part-1)来解释其工作原理，[LLVM 文档](https://llvm.org/docs/CodeGenerator.html#select-instructions-from-dag)则更深入地描述了它，包括其优势和局限性。它允许编写如下模式：

```
def : Pat<(or GR64:$src, (not (add GR64:$src, 1))),
          (BLCI64rr GR64:$src)>;
```

这个例子定义了一个用于 [X86 目标描述](https://github.com/llvm/llvm-project/blob/main/llvm/lib/Target/X86/X86InstrInfo.td)中 ["blci" 指令](https://en.wikipedia.org/wiki/Bit_Manipulation_Instruction_Sets#TBM_\(Trailing_Bit_Manipulation\))的匹配器，该文件中还有许多其他模式（查找 `Pat<>` 模式，因为它们没有与汇编/反汇编生成逻辑等编译器细节纠缠在一起）。

对于 MLIR 而言，这个系统有很多值得借鉴的地方，例如：

*   它以声明式格式定义。
*   它可扩展到目标定义的操作。
*   它自动处理等式匹配，例如可交换模式。
*   它允许自定义抽象和对目标特定共性的深度因式分解。
*   它生成紧凑的代码——编译为状态机并进行解释执行。
*   它允许指令模式被定义并重复用于多种目的。
*   模式在编译时进行"类型检查"，早期检测出大量错误并消除模式规范中的冗余。
*   它允许对奇怪/复杂情况使用通用 C++ 代码。

这里有很多好的东西，但也有一些不理想的方面：

*   该表示专门针对指令选择设计，仅适用于此，这意味着紧邻的问题如 DAGCombiner 和 Legalizer 无法使用它。
*   在编译器运行时无法扩展，必须重新编译编译器才能扩展。
*   模式匹配失败时的错误消息[并非最优](https://www.google.com/search?q=llvm+cannot+select)。
*   由于与笨拙的 SelectionDAG 表示打交道，以及在需求驱动下进行设计和实现，它存在许多实现问题和限制（例如无法为多结果操作编写模式）。
*   随时间有机增长留下了许多锋利的边角。

### 总结

MLIR 面临大量模式匹配和图重写问题，拥有一个在多个层次上表示代码的通用表示的主要优势之一，就是它允许投资于——并高度利用——用于完成此类工作的单一基础设施。

## 目标

我们希望涵盖 MLIR 领域中的许多问题，包括一对多扩展（例如在指令选择期间的类型合法化，一位宽的加法可能被拆分为多个较小位宽的加法）、多对一模式（例如将乘加转换为单个 muladd 操作），以及通用的多对多模式（例如目标指令的指令选择）。模式有与之关联的收益，通用基础设施应当负责为给定应用找出收益最高的匹配。

我们将从给定根节点选择特定最优模式的任务、用于在给定一组目标的情况下重写整个图的算法，以及模式本身的定义分离开来。之所以这样做，是因为 DAG 块模式匹配是 NP 完全问题。此外，我们希望支持通过多个步骤逐步变换输入程序的迭代重写算法。此外，我们希望支持 MLIR 栈中许多不同类型的客户端，它们对编译时代价的容忍度可能不同，对最优性的要求不同，以及其他算法目标或约束也不同。

我们的目标是使 MLIR 变换易于实现，并降低编译器错误的可能性。我们预计随着时间推移将定义大量模式，并相信这类模式将具有大量合法性/有效性约束——其中许多难以以一致的方式推理，可能是目标特定的，且其实现可能特别容易出错。因此，我们旨在将模式定义的 API 设计得简单、对程序员错误有较强的抵御能力，并允许将生成节点的合法性与所定义模式的思想分离关注点。

最后，错误处理是最重要的关注点，我们希望模式匹配失败能够以合理的方式被诊断。这在一般情况下是个难题，因为故障空间太大，无法完全枚举并以最优方式处理，但 MLIR 已被设计为能够很好地表示操作的来源。模式重写基础设施的目标只是精确传播这些来源信息，以及诊断模式匹配失败及一组模式无法适用的原因。

### 非目标

模式基础设施并不旨在解决所有编译器问题，它只是一个 DAG 到 DAG 的模式匹配系统。需要全局数据流分析的编译器算法（例如公共子表达式消除、条件常量传播以及许多其他算法）不会通过这个基础设施直接解决。

该基础设施仅限于 DAG 模式，这（按定义）阻止模式跨图中的环进行观察。在像 MLIR 这样的基于 SSA 的 IR 中，这意味着这些模式无法跨基本块参数进行观察。鉴于我们试图解决的问题集，我们认为这是可以接受的——我们不了解任何其他尝试这样做的系统，并认为为此担心的回报较低。

该设计包含了 DAG 模式具有关联收益的能力，但这些收益以魔法数字定义（通常等于被替换节点的数量）。对于任何给定的应用，必须定义这些魔法数字的单位。
