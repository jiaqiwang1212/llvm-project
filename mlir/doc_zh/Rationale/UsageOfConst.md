# MLIR 核心 IR 类型中 'const' 的使用

又名，`const` 去哪了？

表示 IR 本身的 MLIR 数据结构（Instruction、Block 等）构成了一种基于图的数据结构，编译器分析和 pass 频繁遍历这个图（例如从定义遍历到使用者）。MLIR 早期设计采用了 LLVM 的 `const` 模型，这一模型熟悉且易于理解（尽管 LLVM 的实现在许多方面存在缺陷）。

设计团队此后决定改为不同的模型，该模型对核心 IR 类型完全回避使用 `const`：你永远不应在 `Operation` 上看到 `const` 方法，永远不应看到类型 `const Value`，也不必为此感到不安。也就是说，对于非 IR 类型（如 `SmallVector` 和许多其他类型），你*应该*使用 `const`。

下面的文档从"为何要做出改变"的角度解释了这一设计点，以阐明其设计原理以及促使我们做出这一可能存在争议的设计决策的权衡取舍。

Bjarke Roune 对此情况做了如下概括：

> 在我看来，`const` 正确性非常有价值，能捕获许多错误，并使代码库中的变更位置一目了然。在我看来，`const` 正确性在 IR 元素这一特定场合仍然不值得，原因在于 IR 的特殊用途和属性，尤其是从分析将指令的指针/引用传递给将改变该指令的优化是很常见的。分析应该是 const 的，而优化需要获取非 `const` 指针。因此，所有分析要么最终成为模板（如果它们从未在 const 上下文中实例化，那么 `const` 正确性的意义就已落空），要么需要以某种安全方式去除 const，要么就会出现 `const_cast`。这些选择都很糟糕，其糟糕程度可能足以超过 const 的好处。

# 重新审视 MLIR 中的 `const`

本文论述了这一设计正在给 MLIR 代码库引入显著的次优性，认为这一设计的成本/收益权衡是一个糟糕的权衡，并提出切换到一个更简单的方法——完全消除对这些 IR 类型使用 const。

**注意：** 本文仅讨论诸如 `const Value` 和 `const Operation*` 之类的内容。对于其他类型，例如 `SmallVector` 引用、`Attribute` 等不可变类型，没有提议进行任何更改。

## 背景：LLVM 的 Const 模型

LLVM 和 MLIR 数据结构将 IR 数据结构（如 `mlir::Operation` 及其使用者）提供为一种结构化循环图数据结构。IR 的客户端通常沿图上下遍历，执行动态向下转换（各种形式）以检查模式，并使用一些高抽象度的模式匹配和绑定工具来完成工作。

LLVM 设计的基本思想是，这些 IR 遍历应当保留指针的 const 性：如果你有一个指向某指令的 const 指针，并请求其父级（或操作数、使用者等），你应该得到一个指向包含该指令的基本块（或定义操作数的值、使用该指令的指令等）的 const 指针。指令类看起来像这样：

```c++
namespace llvm {
class Instruction : ...  {
  BasicBlock *Parent;
public:
  // A const instruction returns a const parent pointer.
  inline const BasicBlock *getParent() const { return Parent; }
  // A non-const instruction returns a non-const parent pointer.
  inline       BasicBlock *getParent()       { return Parent; }
...
};
}
```

这种设计的理由是，从 getParent 返回非 const 指针在 const 正确性上是错误的，因为这样你就可以遍历基本块找到该指令，并获得对同一指令的非 const 引用——所有这些都不需要 `const_cast`。

这种 `const` 模型很简单，C++ 类型系统通过方法的代码复制来大体支持它。也就是说，LLVM 在这方面实际上是不一致且存在缺陷的。即使是核心类也存在错误：`llvm::Instruction::getOperand()` 目前就不是 const 正确的！还有其他子系统（例如 `llvm/IR/PatternMatch.h` API），你可以在 const IR 对象上执行模式匹配并绑定一个非 const IR 对象。

LLVM 是一项有数百人参与的成熟技术。它仍然没有正确遵循其所设定的 const 模型这一事实，强烈暗示以下之一：1）该设计过于复杂而无法实际执行；2）该模型的收益不值得其复杂性的代价；或 3）1 和 2 兼而有之，以某种组合形式存在。

## MLIR 中 Const 正确性的优势

尽管本文主张在 MLIR 中消除 const，但重要的是将其作为与 const 模型所提供优势的权衡来评估，使我们能够进行成本/收益权衡。以下是我们看到的优势：

在 MLIR 类型上允许 const 的主要优势是将其作为 API 中的标记，表明该函数不会修改指定的值。例如，支配器 API 有一个 `dominates(const Block*, const Block*)` 方法，其中的 const 提供了一种表明调用不会修改传入的基本块的方式——类似地，`Instruction::isTerminator() const` 等谓词不会修改接收器对象。

MLIR 遵循 C++ 代码普遍流行的模式（通常使用 const）也是一个优势。与社区规范保持一致很重要。

## MLIR 中 Const 正确性的代价

如上所述，MLIR 的早期工作采用了与 LLVM 相同的设计意图，在 API 中允许 const 正确的遍历。在这里我们通过查看一些示例来讨论这样做的各种代价，大致按严重程度递增排列。

### 普遍重复的访问器

正如上面的 getParent() 示例所示，实现这种 const 模型要求所有图遍历访问器都被复制为 const 和非 const 版本。这会导致 API 膨胀和编译时间变慢，但这些都是次要问题。

更重要的问题是，这种重复可能如此显著，以至于信号消失在噪声中，例如 `mlir::Operation` 最终会得到如下内容，仅仅为了满足 const 就有双倍的 API 表面积：

```c++
  operand_iterator operand_begin();
  operand_iterator operand_end();

  /// Returns an iterator on the underlying Value's (Value ).
  operand_range getOperands();

  // Support const operand iteration.
  using const_operand_iterator =
      OperandIterator<const Operation, const Value>;
  using const_operand_range = llvm::iterator_range<const_operand_iterator>;

  const_operand_iterator operand_begin() const;
  const_operand_iterator operand_end() const;

  /// Returns a const iterator on the underlying Value's (Value ).
  llvm::iterator_range<const_operand_iterator> getOperands() const;

  ArrayRef<OpOperand> getOpOperands() const {
    return getOperandStorage().getOperands();
  }
  MutableArrayRef<OpOperand> getOpOperands() {
    return getOperandStorage().getOperands();
  }

  OpOperand &getOpOperand(unsigned idx) { return getOpOperands()[idx]; }
  const OpOperand &getOpOperand(unsigned idx) const {
    return getOpOperands()[idx];
  }

```

### 模板化访问器

一个相关问题是，必须提供 const 和非 const 版本的访问器导致我们不得不将比原本期望的更多代码转化为模板。诸如 `ResultIterator` 和 `ResultTypeIterator` 之类的东西之所以是模板，*仅仅*是因为它们对类型的 const 和非 const 版本是通用的。这导致它们必须在头文件中内联定义（而不是在 .cpp 文件中）。

因此，我们的 const 模型导致头文件中有更多代码，以及实现中有更多复杂性。

### 实践中的 const 不正确

对于某些东西来说，const 带来的麻烦比它的价值更大，因此它们从来没有被更新。

这意味着某些 API 在实践中不提供 const 变体，导致普遍使用 `const_cast` 来去除 const 限定符。例如 `Matchers.h` 中的逻辑根本不支持 const 指针，即使匹配和绑定值本身对 const 和非 const 值都完全有意义。实际修复这个问题会导致大量代码膨胀和复杂性。

代码的其他部分则直接是不正确的。例如，操作克隆方法在 `Operation` 上定义如下：

```C++
Operation *clone(IRMapping &mapper, MLIRContext *context) const;

Operation *clone(MLIRContext *context) const;
```

虽然克隆方法在概念上是 `const` 的（原始操作不被修改）是合理的，但这违反了模型，因为返回的操作必须是可变的，并且提供了对与原始操作相同的操作数完整图的访问，违反了我们所追求的基于图的 const 模型。

### `OpPointer` 和 `ConstOpPointer` 类

已注册操作的"类型化操作"类（例如用于 memref ops 中"memref.dim"操作的 `DimOp`）包含一个指向操作的指针，并提供用于处理它的类型化 API。

然而，这对我们当前的 `const` 设计来说是个问题——`const DimOp` 意味着指针本身是不可变的，而不是被指向的对象。之前针对这个问题的解决方案是 `OpPointer<>` 和 `ConstOpPointer<>` 类，它们的存在仅仅是为了在引用类型化操作时提供 const 正确性。我们不直接引用 `DimOp`，而是使用 `OpPointer<DimOp>` 和 `ConstOpPointer<DimOp>` 来保留这种 const 性。

虽然 `auto` 隐藏了这些 `OpPointer` 类的许多实例，但它们的存在导致了极其丑陋的 API。它还掩盖了用户没有直接 `DimOp` 对象这一事实，为语义上微妙错误创造了容易的陷阱：

```C++
// OpPointer encodes unnecessary and superfluous information into the API.
SmallVector<OpPointer<AffineForOp>, 8> stripmineSink(
  OpPointer<AffineForOp> forOp, uint64_t factor,
  ArrayRef<OpPointer<AffineForOp>> targets);
// Compared to the much cleaner and easier to read...
SmallVector<AffineForOp, 8> stripmineSink(AffineForOp forOp, uint64_t factor,
                                          ArrayRef<AffineForOp> targets);

// OpPointer is easy to misuse.
if (auto *dimOp = inst->dyn_cast<DimOp>()) {
  // This is actually undefined behavior because dyn_cast actually returns
  // OpPointer<DimOp>. OpPointer<DimOp> happily implicitly converts to DimOp *
  // creating undefined behavior that will execute correctly most of the time.
}
```

直接传递 `DimOp` 要好得多，完全消除它们。例如，不是：

```c++
LogicalResult mlir::getIndexSet(MutableArrayRef<OpPointer<AffineForOp>> forOps,
                                FlatAffineValueConstraints *domain) {

```

直接写成这样要好得多：

```c++
LogicalResult mlir::getIndexSet(MutableArrayRef<AffineForOp> forOps,
                                FlatAffineValueConstraints *domain) {
```

特别是因为所有的 `FooOp` 类在语义上已经是指向其底层操作的智能指针。

## （已接受的）提案：从 IR 对象中移除 `const`

如我们上面所见，我们的 const 设计几乎没有什么收益，却有显著的代价，而且鉴于 IR 的主要目的是表示代码的变换，const 提供的收益非常有限。

因此，我们提议在 MLIR 中消除对 IR 对象的 const 引用支持。这意味着对代码库进行以下更改：

1.  所有 const 复制的访问器将被消除，例如 `Operation::getParent() const` 将被移除。仅 Operation.h 一个文件就预计减少约 130 行代码。
1.  仅 const 的谓词将被改为非 const，例如 `Operation::isTerminator() const` 将去掉 const。
1.  为支持 `const` 而模板化的迭代器及其他类型和函数，可以去掉那些模板参数。
1.  仅为传播 const 而存在的类型（如 `OpPointer` 和 `ConstOpPointer`）可以从代码库中完全移除。
1.  我们可以关闭关于 IR 中 const 不正确性的 bug 报告。
