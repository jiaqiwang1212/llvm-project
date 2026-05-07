# 操作规范化

规范化是编译器 IR 设计的重要组成部分：它使得实现可靠的编译器转换更加容易，也便于推理代码的优劣，同时促进了关于特定 IR 层级目标的深入讨论。Dan Gohman 撰写了[一篇文章](https://sunfishcode.github.io/blog/2018/10/22/Canonicalization.html)探讨这些问题；如果你不熟悉这些概念，值得一读。

大多数编译器都有规范化 pass，有时甚至有多个不同的 pass（例如 LLVM 中的 instcombine、dag combine 等）。由于 MLIR 是多层次 IR，我们可以提供单一的规范化基础设施，并在其所代表的许多不同 IR 上复用。本文档描述了通用方法、执行的全局规范化，并提供了各节以供参考 IR 特定规则。

[TOC]

## 总体设计

MLIR 有一个单一的规范化 pass，它以贪心方式迭代地应用所有已加载方言的规范化模式。规范化是尽力而为的，不保证将整个 IR 转化为规范形式。它持续应用模式，直到达到不动点，或者迭代/重写的最大次数（通过 pass 选项指定）耗尽为止。这是出于效率考虑，也是为了确保有缺陷的模式不会导致无限循环。

规范化模式与操作本身一起注册，这允许每个方言将其操作和规范化的定义放在一起。

关于规范化模式，需要考虑以下几个要点：

*   规范化的目标是使后续分析和优化更加有效。因此，规范化不必要求性能提升。

*   Pass 流水线不应依赖规范化器 pass 的正确性。在移除所有规范化 pass 实例的情况下，流水线应能正确运行。

*   模式的重复应用应该收敛。不稳定或循环的重写被认为是 bug：它们可能使规范化器 pass 的行为不那么可预测，降低其有效性（即某些模式可能不会被应用），并阻止其收敛。

*   当操作数被复制时，通常最好将规范化方向朝着减少某个值使用次数的操作，因为某些模式只有在一个值只有单个用户时才能匹配。例如，通常将 "x + x" 规范化为 "x * 2" 是好的做法，因为这减少了 x 的使用次数。

*   尽可能完全消除操作总是好的，例如通过折叠已知的恒等式（如 "x + 0 = x"）。

*   运行时间较长（即复杂度为 O(n)）或具有复杂代价模型的模式不属于规范化范畴：由于算法会迭代执行直到不动点，我们希望模式执行速度快（特别是它们的匹配阶段）。

*   规范化不应丢失原始操作的语义：从转换后的 IR 中始终能够恢复原始信息。

## 什么是规范形式？

MLIR 中没有正式定义的规范形式。事实上的规范形式随着社区添加/删除/修改规范化模式和折叠器而不断演进。

规范化器 pass 在许多项目中使用，但不提供对单个模式或折叠的细粒度控制，这使得对规范形式的更改可能存在争议。某个转换是否属于规范形式必须逐案决定，但社区普遍认可的规范化包括：

* 恒等/空操作消除。例如，将 `arith.addi(%x, %c0)` 折叠为 `%x`，或删除 `memref.copy(%x, %x)`。
* 标量常量折叠。例如，将 `arith.addi(%c1, %c2)` 折叠为 `%c3`。
* 折叠逆操作。例如，将 `arith.xori(arith.xori(%x, %a), %a)` 折叠为 `%x`。
* 消除未使用/冗余值。例如，移除 `scf.for` 操作中未使用的循环携带变量，或移除冗余的 `scf.if` 结果（当两个分支产生相同值时）。
* 简单的控制流简化。例如，内联 `scf.if %true` 操作的 "then" 体并删除该 `scf.if` 操作。
* 折叠相同类型的链式元数据/形状操作。例如，将 `linalg.transpose(linalg.transpose(%x))` 替换为单个 `linalg.transpose(%x)`。
* 动态到静态类型细化，例如将常量大小折叠到有形状类型中。例如，将 `%v = tensor.empty(%c5) : tensor<?xf32>` 重写为 `%0 = tensor.empty() : tensor<5xf32>` 和 `%v = tensor.cast %0 : tensor<5xf32> to tensor<?xf32>`。
* 类型转换传播/折叠，例如将类型转换推入操作中，或者在引入更多静态类型信息时将其折叠。例如，将 `tensor.insert_slice(%src, tensor.cast(%dst))`（其中类型转换从 `tensor<5xf32>` 转换到 `tensor<?xf32>`）重写为 `tensor.cast(tensor.insert_slice(%src, %dst))`。


注意：当某些规范化会导致 IR 体积爆炸时（例如，当它们会产生"大型" tensor/vector 属性时），这些规范化不适用。

注意：有些方言定义了多种 IR 形式，有时取决于后续转换（[示例](https://mlir.llvm.org/docs/Rationale/RationaleLinalgDialect/#interchangeability-of-formsa-nameformsa)）。这些形式与 MLIR 的规范化机制无关。

## 全局应用的规则

这些转换应用于 IR 的所有层级：

*   消除没有副作用且没有使用者的操作。

*   常量折叠——例如，将 "(addi 1, 2)" 折叠为 "3"。常量折叠钩子由操作指定。

*   将常量操作数移动到交换运算符的右侧——例如，将 "(addi 4, x)" 转换为 "(addi x, 4)"。

*   `constant-like` 操作被去重并提升到第一个父屏障区域的入口块。屏障区域是与上方隔离的区域（例如函数的入口块），或通过 `DialectFoldInterface` 上的 `shouldMaterializeInto` 方法标记为屏障的区域。

## 定义规范化

有两种可用机制来定义规范化：通用的 `RewritePattern` 和 `fold` 方法。

### 使用 `RewritePattern` 进行规范化

该机制允许将规范化作为一组 `RewritePattern` 来提供，可以在 C++ 中以命令式方式定义，也可以以声明式方式定义为[声明式重写规则](DeclarativeRewrites.md)。模式重写基础设施允许表达多种不同类型的规范化。这些转换可以简单到将乘法替换为移位，甚至复杂到将条件分支替换为无条件分支。

在 [ODS](DefiningDialects/Operations.md) 中，操作可以设置 `hasCanonicalizer` 位或 `hasCanonicalizeMethod` 位，以生成 `getCanonicalizationPatterns` 方法的声明：

```tablegen
def MyOp : ... {
  // I want to define a fully general set of patterns for this op.
  let hasCanonicalizer = 1;
}

def OtherOp : ... {
  // A single "matchAndRewrite" style RewritePattern implemented as a method
  // is good enough for me.
  let hasCanonicalizeMethod = 1;
}
```

然后可以在源文件中提供规范化模式：

```c++
void MyOp::getCanonicalizationPatterns(RewritePatternSet &patterns,
                                       MLIRContext *context) {
  patterns.add<...>(...);
}

LogicalResult OtherOp::canonicalize(OtherOp op, PatternRewriter &rewriter) {
  // patterns and rewrites go here.
  return failure();
}
```

有关定义操作重写的信息，请参阅[快速入门指南](Tutorials/QuickstartRewrites.md)。

### 使用 `fold` 方法进行规范化

`fold` 机制是一种有意限制但功能强大的机制，允许在编译器的许多地方应用规范化。例如，在规范化器 pass 之外，`fold` 在[方言转换基础设施](DialectConversion.md)中被用作合法化机制，并且可以通过 `OpBuilder::createOrFold` 在任何有 `OpBuilder` 的地方直接调用。

`fold` 的限制是不能创建新操作，且只有根操作可以被替换（但不能被删除）。它允许就地更新操作，或返回一组预先存在的值（或属性）来替换操作。这确保了 `fold` 方法是真正"本地"的转换，无需模式重写器即可调用。

在 [ODS](DefiningDialects/Operations.md) 中，操作可以设置 `hasFolder` 位以生成 `fold` 方法的声明。该方法根据操作的结构有不同的形式。

```tablegen
def MyOp : ... {
  let hasFolder = 1;
}
```

如果操作有单个结果，将生成如下代码：

```c++
/// Implementations of this hook can only perform the following changes to the
/// operation:
///
///  1. They can leave the operation alone and without changing the IR, and
///     return nullptr.
///  2. They can mutate the operation in place, without changing anything else
///     in the IR. In this case, return the operation itself.
///  3. They can return an existing value or attribute that can be used instead
///     of the operation. The caller will remove the operation and use that
///     result instead.
///
OpFoldResult MyOp::fold(FoldAdaptor adaptor) {
  ...
}
```

否则，将生成如下代码：

```c++
/// Implementations of this hook can only perform the following changes to the
/// operation:
///
///  1. They can leave the operation alone and without changing the IR, and
///     return failure.
///  2. They can mutate the operation in place, without changing anything else
///     in the IR. In this case, return success.
///  3. They can return a list of existing values or attribute that can be used
///     instead of the operation. In this case, fill in the results list and
///     return success. The results list must correspond 1-1 with the results of
///     the operation, partial folding is not supported. The caller will remove
///     the operation and use those results instead.
///
/// Note that this mechanism cannot be used to remove 0-result operations.
LogicalResult MyOp::fold(FoldAdaptor adaptor,
                         SmallVectorImpl<OpFoldResult> &results) {
  ...
}
```

在上述代码中，每个方法都提供了一个 `FoldAdaptor`，其中包含每个操作数的 getter，返回对应的常量属性。这些操作数是实现了 `ConstantLike` trait 的操作数。如果任何操作数是非常量的，则改为提供空的 `Attribute` 值。例如，如果 MyOp 提供三个操作数 [`a`, `b`, `c`]，但只有 `b` 是常量，则 `adaptor` 对 `getA()` 和 `getC()` 返回 Attribute()，对 `getB()` 返回 b 的值。

上述代码中还使用了 `OpFoldResult`。该类表示折叠操作结果的可能结果：要么是 SSA `Value`，要么是 `Attribute`（用于常量结果）。如果提供了 SSA `Value`，它*必须*对应于现有值。`fold` 方法不允许生成新的 `Value`。对于返回的 `Attribute` 值的形式没有特定限制，但重要的是确保特定 `Type` 的 `Attribute` 表示是一致的。

当操作的 `fold` 钩子不成功时，方言可以通过实现 `DialectFoldInterface` 并重写 fold 钩子来提供后备方案。

#### 从属性生成常量

当 `fold` 方法返回 `Attribute` 作为结果时，这表明该结果是"常量"。`Attribute` 是该值的常量表示。`fold` 方法的用户（例如规范化器 pass）将接受这些 `Attribute` 并在 IR 中实体化常量操作来表示它们。为了启用这种实体化，操作所属的方言必须实现 `materializeConstant` 钩子。该钩子接受通常由 `fold` 返回的 `Attribute` 值，并生成一个实体化该值的"类常量"操作。

在 [ODS](DefiningDialects/_index.md) 中，方言可以设置 `hasConstantMaterializer` 位以生成 `materializeConstant` 方法的声明。

```tablegen
def MyDialect : ... {
  let hasConstantMaterializer = 1;
}
```

然后可以在源文件中实体化常量：

```c++
/// Hook to materialize a single constant operation from a given attribute value
/// with the desired resultant type. This method should use the provided builder
/// to create the operation without changing the insertion position. The
/// generated operation is expected to be constant-like. On success, this hook
/// should return the value generated to represent the constant value.
/// Otherwise, it should return nullptr on failure.
Operation *MyDialect::materializeConstant(OpBuilder &builder, Attribute value,
                                          Type type, Location loc) {
  ...
}
```

### 何时使用 `fold` 方法与 `RewriterPattern` 进行规范化

如果可以，规范化应始终以 `fold` 方法实现，否则应以 `RewritePattern` 实现。
