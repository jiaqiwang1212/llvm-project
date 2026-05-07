# 模式重写：通用 DAG 到 DAG 的重写

[TOC]

本文档详细介绍了 MLIR 中模式重写(pattern rewriting)基础设施的设计和 API，这是一个通用的 DAG 到 DAG 变换框架。该框架在整个 MLIR 中被广泛用于规范化(canonicalization)、转换(conversion)和通用变换。

关于 DAG 到 DAG 变换的介绍以及该框架背后的设计原理，请参阅
[通用 DAG 重写器设计原理](Rationale/RationaleGenericDAGRewriter.md)。

## 简介

模式重写框架大致可分为两部分：模式定义和模式应用。

## 定义模式

模式通过继承 `RewritePattern` 类来定义。该类是 MLIR 中所有重写模式的基类，由以下组件构成：

### 收益

这是应用给定模式的预期收益(benefit)。该收益在模式构造时是静态的，但可以在模式初始化时动态计算，例如允许从领域特定信息（如目标架构）推导收益。这个限制允许执行模式融合并将模式编译成高效的状态机，而
[Thier、Ertl 和 Krall](https://dl.acm.org/citation.cfm?id=3179501) 已经证明，匹配谓词在几乎所有情况下都消除了动态计算代价的需求：你只需为每个可能的代价实例化同一个模式，并使用谓词来守卫匹配。

### 根操作名称（可选）

此模式匹配的根操作(operation)名称。如果指定了，则只有具有给定根名称的操作才会被提供给 `matchAndRewrite` 实现。如果未指定，则可以提供任何操作类型。应尽可能提供根操作名称，因为在应用代价模型时这简化了模式分析。要匹配任何操作类型，必须提供一个特殊标签以明确表达意图：`MatchAnyOpTypeTag`。

### `matchAndRewrite` 实现

这是匹配给定根 `Operation` 并执行 IR 重写的代码块。`RewritePattern` 可以通过 `matchAndRewrite` 方法指定此实现。在匹配被确认成功之前，不应发生任何 IR 修改。以下是一些示例：

```c++
class MyPattern : public RewritePattern {
public:
  /// This overload constructs a pattern that only matches operations with the
  /// root name of `MyOp`.
  MyPattern(PatternBenefit benefit, MLIRContext *context)
      : RewritePattern(MyOp::getOperationName(), benefit, context) {}
  /// This overload constructs a pattern that matches any operation type.
  MyPattern(PatternBenefit benefit)
      : RewritePattern(benefit, MatchAnyOpTypeTag()) {}

  LogicalResult matchAndRewrite(Operation *op, PatternRewriter &rewriter) const override {
    // The `matchAndRewrite` method performs both the matching and the mutation.
    // Note that the match must reach a successful point before IR mutation may
    // take place.
  }
};
```

#### 限制

*   所有 IR 修改，包括创建，都*必须*通过给定的 `PatternRewriter` 执行。该类提供了用于执行模式中可能发生的所有修改的钩子。例如，这意味着不应该通过操作的 `erase` 方法来删除它。要删除操作，应使用适当的 `PatternRewriter` 钩子（在此情况下为 `eraseOp`）。
*   根操作必须满足以下之一：就地更新、被替换或被删除。
*   `matchAndRewrite` 当且仅当 IR 被修改时才能返回"成功"。


### 应用递归

递归是模式重写中的一个重要主题，因为模式往往可以应用于其自身的结果。例如，想象一个从循环操作中剥离单次迭代的模式。如果循环有多个可剥离的迭代，该模式可能在应用过程中被多次应用。从该模式的实现来看，递归应用的界限可能是显而易见的（例如，循环中没有可剥离的迭代），但从模式驱动器的角度来看，这种递归存在潜在风险。模式的递归应用通常表明匹配逻辑中存在 bug。这类 bug 通常不会导致崩溃，但会在应用过程中产生无限循环。鉴于此，模式重写基础设施保守地假设没有任何模式具有有界递归，并在检测到递归时失败。已知能够正确支持递归处理的模式可以在初始化模式时通过调用 `setHasBoundedRewriteRecursion` 来发出信号。这将通知模式驱动器该模式可能发生递归应用，且该模式能够安全地处理它。

### 调试名称和标签

为了辅助调试，模式可以指定：调试名称（通过 `setDebugName`），应对应唯一标识特定模式的标识符；以及一组调试标签（通过 `addDebugLabels`），对应唯一标识模式组的标识符。各种工具使用这些信息来辅助调试模式重写，例如在调试日志中提供模式过滤等。以下是一个简单的代码示例：

```c++
class MyPattern : public RewritePattern {
public:
  /// Inherit constructors from RewritePattern.
  using RewritePattern::RewritePattern;

  void initialize() {
    setDebugName("MyPattern");
    addDebugLabels("MyRewritePass");
  }

  // ...
};

void populateMyPatterns(RewritePatternSet &patterns, MLIRContext *ctx) {
  // Debug labels may also be attached to patterns during insertion. This allows
  // for easily attaching common labels to groups of patterns.
  patterns.addWithLabel<MyPattern, ...>("MyRewritePatterns", ctx);
}
```

### 初始化

模式的多个状态需要由模式显式初始化，例如，如果模式安全地处理递归应用，则需要设置 `setHasBoundedRewriteRecursion`。该模式状态既可以在模式的构造函数中初始化，也可以通过实用工具 `initialize` 钩子初始化。使用 `initialize` 钩子无需为注入额外的模式状态初始化而重新定义模式构造函数。以下是一个示例：

```c++
class MyPattern : public RewritePattern {
public:
  /// Inherit the constructors from RewritePattern.
  using RewritePattern::RewritePattern;

  /// Initialize the pattern.
  void initialize() {
    /// Signal that this pattern safely handles recursive application.
    setHasBoundedRewriteRecursion();
  }

  // ...
};
```

### 构造

构造 RewritePattern 应使用静态工具方法 `RewritePattern::create<T>` 来执行。此方法确保模式被正确初始化并准备好插入到 `RewritePatternSet` 中。

## 模式重写器

`PatternRewriter` 是一个特殊类，允许模式与模式应用驱动器进行通信。如上所述，*所有* IR 修改，包括创建，都必须通过 `PatternRewriter` 类执行。这是必要的，因为底层模式驱动器可能具有在发生修改时会失效的状态。以下展示了一些更常用的 `PatternRewriter` API 示例，请参阅
[类文档](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/IR/PatternMatch.h#L235)
以获取最新的可用 API 列表：

*   删除操作：`eraseOp`

此方法删除没有结果或其所有结果都已知没有使用的操作。

*   通知 `match` 失败原因：`notifyMatchFailure`

此方法允许在 `matchAndRewrite` 中提供诊断消息，说明模式匹配失败的原因。此消息如何显示给用户由具体的模式驱动器决定。

*   替换操作：`replaceOp`/`replaceOpWithNewOp`

此方法用一组提供的值替换操作的结果，并删除该操作。

*   就地更新操作：`(start|cancel|finalize)OpModification`

这是一组方法，为就地更新操作的属性(attribute)、位置、操作数或后继提供类事务的 API。就地更新事务以 `startOpModification` 开始，可以用 `cancelOpModification` 取消或用 `finalizeOpModification` 完成。提供了一个便利包装器 `modifyOpInPlace`，它将 `start` 和 `finalize` 包装在一个回调函数周围。

*   OpBuilder API

`PatternRewriter` 继承自 `OpBuilder` 类，因此提供了 `OpBuilder` 中的所有功能。这包括操作创建，以及许多有用的属性和类型构造方法。

## 模式应用

定义一组模式后，将其收集并提供给特定的驱动器进行应用。一个驱动器由几个高层次部分组成：

*   输入 `RewritePatternSet`

提供给驱动器的输入模式以 `RewritePatternSet` 的形式提供。该类提供了一个简化的 API 来构建模式列表。

*   驱动器特定的 `PatternRewriter`

为了确保驱动器状态不会因模式重写器中的 IR 修改而失效，驱动器必须提供一个 `PatternRewriter` 实例，并覆盖必要的钩子。如果驱动器不需要钩入某些修改，则提供默认实现来直接执行修改。

*   模式应用和代价模型

每个驱动器负责定义自己的操作访问顺序以及模式代价模型，但最终应用通过 `PatternApplicator` 类执行。该类接受 `RewritePatternSet` 作为输入，并根据提供的代价模型转换模式。该代价模型使用任何必要的驱动器特定信息为给定模式计算最终收益。计算完代价模型后，驱动器可以开始使用 `PatternApplicator::matchAndRewrite` 将模式与操作进行匹配。

以下是一个示例：

```c++
class MyPattern : public RewritePattern {
public:
  MyPattern(PatternBenefit benefit, MLIRContext *context)
      : RewritePattern(MyOp::getOperationName(), benefit, context) {}
};

/// Populate the pattern list.
void collectMyPatterns(RewritePatternSet &patterns, MLIRContext *ctx) {
  patterns.add<MyPattern>(/*benefit=*/1, ctx);
}

/// Define a custom PatternRewriter for use by the driver.
class MyPatternRewriter : public PatternRewriter {
public:
  MyPatternRewriter(MLIRContext *ctx) : PatternRewriter(ctx) {}

  /// Override the necessary PatternRewriter hooks here.
};

/// Apply the custom driver to `op`.
void applyMyPatternDriver(Operation *op,
                          const FrozenRewritePatternSet &patterns) {
  // Initialize the custom PatternRewriter.
  MyPatternRewriter rewriter(op->getContext());

  // Create the applicator and apply our cost model.
  PatternApplicator applicator(patterns);
  applicator.applyCostModel([](const Pattern &pattern) {
    // Apply a default cost model.
    // Note: This is just for demonstration, if the default cost model is truly
    //       desired `applicator.applyDefaultCostModel()` should be used
    //       instead.
    return pattern.getBenefit();
  });

  // Try to match and apply a pattern.
  LogicalResult result = applicator.matchAndRewrite(op, rewriter);
  if (failed(result)) {
    // ... No patterns were applied.
  }
  // ... A pattern was successfully applied.
}
```

## 常用模式驱动器

MLIR 提供了几种常用的模式驱动器，服务于各种不同的用例。

### 方言转换驱动器

该驱动器提供了一个框架，使用"合法性"概念在方言之间以及方言内部执行操作转换。该框架允许通过一组基于模式的操作重写模式将非法操作转换为转换目标支持的操作。该框架还提供类型转换支持。关于此驱动器的更多信息可以在[这里](DialectConversion.md)找到。

### 遍历模式重写驱动器

这是一个快速而简单的驱动器，它遍历给定的操作并应用局部收益最高的模式。模式的收益完全由模式上指定的收益以及模式在模式列表中的相对顺序（当两个模式具有相同的局部收益时）决定。

该驱动器执行后序遍历。注意，它遍历给定操作的区域，但不访问该操作本身。

该驱动器不（重新）访问被修改或新替换的操作，也不允许对同一操作进行渐进式重写。只支持对当前匹配的操作及其后代进行操作和块删除。如果你的模式集需要这些功能，考虑改用贪心模式重写驱动器，但需承担额外开销。

该驱动器通过 `walkAndApplyPatterns` 函数暴露。

注意：此驱动器通过 `RewriterBase` 提供的回调监听 IR 变化。重要的是，模式必须向重写器通告所有 IR 变化，不得绕过重写器 API 直接修改操作。

#### 调试

可以通过传递 `--debug-only=walk-rewriter` 命令行标志来调试遍历模式重写驱动器。这将打印被访问和匹配的操作。

### 贪心模式重写驱动器

该驱动器以工作列表驱动的方式处理操作，并贪心地应用局部收益最高的模式（与遍历模式重写驱动器相同）。模式被迭代地应用于操作，直到达到不动点或可配置的最大迭代次数耗尽，此时驱动器结束。

该驱动器有两种形式：

*   `applyPatternsGreedily`（"基于区域的驱动器"）将模式应用于给定区域或给定容器操作中的所有操作（但不包括容器操作本身）。即，工作列表用所有包含的操作初始化。
*   `applyOpPatternsGreedily`（"基于操作的驱动器"）将模式应用于提供的操作列表。即，工作列表用指定的操作列表初始化。

该驱动器可通过 `GreedyRewriteConfig` 进行配置。基于区域的驱动器支持两种填充初始工作列表的模式：

*   自顶向下遍历：以前序遍历方式自顶向下遍历容器操作/区域。这在编译时间上通常更高效。
*   自底向上遍历：这是默认设置。它以后序遍历构建初始工作列表，然后反转工作列表。这可能会匹配具有歧义模式集时更大的模式。

默认情况下，被就地修改的操作和新创建的操作会被添加回工作列表。位于驱动器可配置"作用域"之外的操作不会被添加到工作列表。此外，"严格模式"可以在整个重写过程中排除某些操作被添加到工作列表：

*   `GreedyRewriteStrictness::AnyOp`：没有操作被排除（除了超出作用域的操作）。
*   `GreedyRewriteStrictness::ExistingAndNewOps`：只有预先存在的操作（工作列表初始化时使用的操作）和新创建的操作才会被添加到工作列表。
*   `GreedyRewriteStrictness::ExistingOps`：只有预先存在的操作（工作列表初始化时使用的操作）才会被添加到工作列表。

注意：此驱动器通过 `RewriterBase` 提供的回调监听 IR 变化。重要的是，模式必须向重写器通告所有 IR 变化，不得绕过重写器 API 直接修改操作。

注意：此驱动器是 MLIR 中[规范化](Canonicalization.md)[传递](Passes.md/#-canonicalize)使用的驱动器。

#### 调试

要调试贪心模式重写驱动器的执行，可以使用 `-debug-only=greedy-rewriter`。此命令行标志仅为贪心模式重写器激活 LLVM 的调试日志基础设施。输出以树形结构格式化，镜像模式应用过程的结构。此输出包含重写器执行的所有操作、操作如何被处理以及模式如何被应用，以及它们为何失败。

以下是输出示例：

```
//===-------------------------------------------===//
Processing operation : 'cf.cond_br'(0x60f000001120) {
  "cf.cond_br"(%arg0)[^bb2, ^bb2] {operandSegmentSizes = array<i32: 1, 0, 0>} : (i1) -> ()

  * Pattern SimplifyConstCondBranchPred : 'cf.cond_br -> ()' {
  } -> failure : pattern failed to match

  * Pattern SimplifyCondBranchIdenticalSuccessors : 'cf.cond_br -> ()' {
    ** Insert  : 'cf.br'(0x60b000003690)
    ** Replace : 'cf.cond_br'(0x60f000001120)
  } -> success : pattern applied successfully
} -> success : pattern matched
//===-------------------------------------------===//
```

此输出描述了对 `cf.cond_br` 操作的处理过程。我们首先尝试应用 `SimplifyConstCondBranchPred`，它失败了。之后，另一个模式（`SimplifyCondBranchIdenticalSuccessors`）被应用，它匹配了 `cf.cond_br` 并将其替换为 `cf.br`。

## 调试

### 模式过滤

为了简化测试用例的定义和缩减，`FrozenRewritePatternSet` 类提供了内置支持来过滤哪些模式应提供给模式驱动器进行应用。过滤行为通过在构造 `FrozenRewritePatternSet` 时提供 `disabledPatterns` 和 `enabledPatterns` 列表来指定。`disabledPatterns` 列表应包含在模式应用期间被禁用的模式的调试名称或标签集合，即哪些模式应被过滤掉。`enabledPatterns` 列表应包含在模式应用期间被启用的模式的调试名称或标签集合，不满足此约束的模式将被过滤掉。注意，`disabledPatterns` 列表中指定的模式即使满足 `enabledPatterns` 列表中的条件也会被过滤掉。以下是一个示例：

```c++
void MyPass::initialize(MLIRContext *context) {
  // No patterns are explicitly disabled.
  SmallVector<std::string> disabledPatterns;
  // Enable only patterns with a debug name or label of `MyRewritePatterns`.
  SmallVector<std::string> enabledPatterns(1, "MyRewritePatterns");

  RewritePatternSet rewritePatterns(context);
  // ...
  frozenPatterns = FrozenRewritePatternSet(rewritePatterns, disabledPatterns,
                                           enabledPatterns);
}
```

### 通用传递工具

使用重写模式的传递应致力于提供一套通用的选项和开关，以简化在不同传递/项目/等之间切换时的调试体验。为此，MLIR 提供了一套通用工具，可以在定义自定义传递时轻松包含。这些工具定义在 `mlir/Rewrite/PassUtil.td` 中；以下是一个示例用法：

```tablegen
def MyRewritePass : Pass<"..."> {
  let summary = "...";
  let constructor = "createMyRewritePass()";

  // Inherit the common pattern rewrite options from `RewritePassUtils`.
  let options = RewritePassUtils.options;
}
```

#### 重写传递选项

本节记录了用于控制重写模式应用行为的常用传递选项。

##### 模式过滤

暴露了两个常用的模式过滤选项，`disable-patterns` 和 `enable-patterns`，与上述[模式过滤](#模式过滤)部分描述的 `disabledPatterns` 和 `enabledPatterns` 列表行为相匹配。这些选项的 tablegen 定义片段如下所示：

```tablegen
ListOption<"disabledPatterns", "disable-patterns", "std::string",
           "Labels of patterns that should be filtered out during application">,
ListOption<"enabledPatterns", "enable-patterns", "std::string",
           "Labels of patterns that should be used during application, all "
           "other patterns are filtered out">,
```

这些选项可用于在传递中构造任何 `FrozenRewritePatternSet` 时提供过滤行为：

```c++
void MyRewritePass::initialize(MLIRContext *context) {
  RewritePatternSet rewritePatterns(context);
  // ...

  // When constructing the `FrozenRewritePatternSet`, we provide the filter
  // list options.
  frozenPatterns = FrozenRewritePatternSet(rewritePatterns, disabledPatterns,
                                           enabledPatterns);
}
```
