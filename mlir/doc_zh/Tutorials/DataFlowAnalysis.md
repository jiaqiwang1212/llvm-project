# 在 MLIR 中编写数据流分析

在 MLIR 或任何编译器中编写数据流分析，往往看起来相当令人望而生畏、错综复杂。数据流分析通常涉及在 IR 的各种不同控制流结构之间传播信息，而 MLIR 拥有许多这样的结构（基于块的分支、基于 Region 的分支、CallGraph 等），并非总是清楚如何最好地执行传播。为了帮助在 MLIR 中编写这类分析，本文档详细介绍了几个简化该过程并使其更易于上手的工具。

## 前向数据流分析

数据流分析的一种类型是前向传播分析。顾名思义，这种类型的分析向前传播信息（例如从定义到使用）。为了提供一些具体的背景知识，让我们来了解一下如何在 MLIR 中编写一个简单的前向数据流分析。假设对于这个分析，我们希望传播一个特殊的 "metadata" 字典属性的信息。该属性的内容只是一组描述特定值的元数据，例如 `metadata = { likes_pizza = true }`。我们将收集 IR 中操作的 `metadata` 并对其进行传播。

### 格（Lattice）

在介绍如何设置分析本身之前，首先需要引入 `Lattice`（格）的概念，以及我们将如何将其用于分析。格表示给定值的分析的所有可能值或结果。格元素持有分析为给定值计算的信息集合，并且是在 IR 中传播的内容。对于我们的分析，这对应于 `metadata` 字典属性。

无论所持有的值是什么，每种类型的格都包含两个特殊的元素状态：

*   `uninitialized`（未初始化）

    -   该元素尚未被初始化。

*   `top`/`overdefined`/`unknown`（顶/过定义/未知）

    -   该元素涵盖所有可能的值。
    -   这是一种非常保守的状态，本质上意味着"我无法对该值做任何假设，它可以是任何值"

在合并（或如本文档后续所称的 `join`）分析信息时，这两种状态非常重要。当存在两个不同的来源点时，格元素会被 `join`，例如具有多个前驱的块的参数。关于 `join` 操作，有一个重要的说明：它必须是单调的（有关更多信息，请参阅下面示例中的 `join` 方法）。这确保了 `join` 元素的一致性。上述两个特殊状态在 `join` 期间具有独特的属性：

*   `uninitialized`（未初始化）

    -   如果其中一个元素是 `uninitialized`，则使用另一个元素。
    -   在 `join` 的上下文中，`uninitialized` 本质上意味着"取另一个值"。

*   `top`/`overdefined`/`unknown`（顶/过定义/未知）

    -   如果被 join 的元素之一是 `overdefined`，则结果为 `overdefined`。

对于我们在 MLIR 中的分析，我们需要定义一个类来表示我们的数据流分析所使用的格元素持有的值：

```c++
/// 我们格的值代表 DictionaryAttr 的内部结构，用于 `metadata`。
struct MetadataLatticeValue {
  MetadataLatticeValue() = default;
  /// 从提供的字典计算格值。
  MetadataLatticeValue(DictionaryAttr attr)
      : metadata(attr.begin(), attr.end()) {}

  /// 返回一个悲观值状态，即我们值类型的 `top`/`overdefined`/`unknown` 状态。
  /// 结果状态不应对 IR 的状态做任何假设。
  static MetadataLatticeValue getPessimisticValueState(MLIRContext *context) {
    // `top`/`overdefined`/`unknown` 状态是我们对任何元数据一无所知时的状态，
    // 即空字典。
    return MetadataLatticeValue();
  }
  /// 仅使用关于所提供 IR 状态的信息，返回我们值类型的悲观值状态。
  /// 这类似于上面的方法，但可能产生稍微更精细的结果。这是允许的，
  /// 因为该信息已经作为事实编码在 IR 中。
  static MetadataLatticeValue getPessimisticValueState(Value value) {
    // 检查父操作是否有元数据。
    if (Operation *parentOp = value.getDefiningOp()) {
      if (auto metadata = parentOp->getAttrOfType<DictionaryAttr>("metadata"))
        return MetadataLatticeValue(metadata);

      // 如果没有元数据，则回退到 `top`/`overdefined`/`unknown` 状态。
    }
    return MetadataLatticeValue();
  }

  /// 该方法保守地将 `lhs` 和 `rhs` 持有的信息合并为一个新值。
  /// 该方法必须是单调的。`单调性` 由以下公理的满足来保证：
  ///   * 幂等性：   join(x,x) == x
  ///   * 交换律：   join(x,y) == join(y,x)
  ///   * 结合律：   join(x,join(y,z)) == join(join(x,y),z)
  ///
  /// 当上述公理得到满足时，我们实现了 `单调性`：
  ///   * 单调性：   join(x, join(x,y)) == join(x,y)
  static MetadataLatticeValue join(const MetadataLatticeValue &lhs,
                                   const MetadataLatticeValue &rhs) {
    // 为了 join `lhs` 和 `rhs`，我们将定义一个简单的策略，即只保留相同的信息。
    // 这意味着我们只保留在两者中都为真的事实。
    MetadataLatticeValue result;
    for (const auto &lhsIt : lhs.metadata) {
      // 如上所述，只有当值相同时才合并。
      auto it = rhs.metadata.find(lhsIt.first);
      if (it == rhs.metadata.end() || it.second != lhsIt.second)
        continue;
      result.insert(lhsIt);
    }
    return result;
  }

  /// 一个简单的比较器，检查此值是否等于提供的值。
  bool operator==(const MetadataLatticeValue &rhs) const {
    if (metadata.size() != rhs.metadata.size())
      return false;
    // 检查 `rhs` 是否包含相同的元数据。
    for (const auto &it : metadata) {
      auto rhsIt = rhs.metadata.find(it.first);
      if (rhsIt == rhs.metadata.end() || it.second != rhsIt.second)
        return false;
    }
    return true;
  }

  /// 我们的值表示组合的元数据，其原始形式为 DictionaryAttr，因此我们使用 map。
  DenseMap<StringAttr, Attribute> metadata;
};
```

上面有一件值得注意的事情：我们没有针对 `uninitialized` 状态的显式方法。该状态由 `LatticeElement` 类处理，该类管理给定 IR 实体的格值。下面是该类的快速概览，以及在编写分析时我们感兴趣的 API：

```c++
/// 该类表示持有 `ValueT` 类型特定值的格元素。
template <typename ValueT>
class LatticeElement ... {
public:
  /// 返回此元素持有的值。这要求值已知，即不为 `uninitialized`。
  ValueT &getValue();
  const ValueT &getValue() const;

  /// 将 'rhs' 元素中包含的信息合并到此元素中。
  /// 返回当前元素的状态是否发生了变化。
  ChangeResult join(const LatticeElement<ValueT> &rhs);

  /// 将 'rhs' 值中包含的信息合并到此格中。
  /// 返回当前格的状态是否发生了变化。
  ChangeResult join(const ValueT &rhs);

  /// 将格元素标记为已达到悲观不动点。这意味着格可能存在冲突的值状态，
  /// 只应依赖保守已知的值状态。
  ChangeResult markPessimisticFixPoint();
};
```

定义好格之后，我们现在可以定义驱动程序，用于在 IR 中计算和传播我们的格。

### ForwardDataflowAnalysis 驱动程序

`ForwardDataFlowAnalysis` 类表示数据流分析的驱动程序，并执行所有相关的分析计算。在定义我们的分析时，我们将从该类继承并实现其中的一些钩子。在此之前，让我们快速了解该类及其对我们分析重要的 API：

```c++
/// 该类表示前向数据流分析的主驱动程序。它将被计算的格的值类型作为模板参数。
template <typename ValueT>
class ForwardDataFlowAnalysis : ... {
public:
  ForwardDataFlowAnalysis(MLIRContext *context);

  /// 对以给定顶层操作为根的操作运行分析。注意，顶层操作本身不会被访问。
  void run(Operation *topLevelOp);

  /// 返回附加到给定值的格元素。如果尚未为给定值添加格，
  /// 则插入并返回一个新的 'uninitialized' 值。
  LatticeElement<ValueT> &getLatticeElement(Value value);

  /// 返回附加到给定值的格元素，如果尚未为该值创建格元素，则返回 nullptr。
  LatticeElement<ValueT> *lookupLatticeElement(Value value);

  /// 将给定值范围的所有格元素标记为已达到悲观不动点。
  ChangeResult markAllPessimisticFixPoint(ValueRange values);

protected:
  /// 访问给定操作，并使用提供的操作数格元素集合（所有指针值保证非空）
  /// 将必要的分析状态合并到此操作拥有的结果和块参数的格元素中。
  /// 返回访问期间是否有任何结果或块参数值格元素发生了变化。
  /// 可以通过使用 `getLatticeElement` 来获取结果或块参数值的格元素并进行合并。
  virtual ChangeResult visitOperation(
      Operation *op, ArrayRef<LatticeElement<ValueT> *> operands) = 0;
};
```

注意：示例中某些 API 已被省略。`ForwardDataFlowAnalysis` 包含各种其他钩子，允许在适用时注入自定义行为。

我们负责定义的主要 API 是 `visitOperation` 方法。该方法负责为给定操作拥有的结果和块参数计算新的格元素。这是我们注入格元素计算逻辑（也称为操作的传递函数）的地方，该逻辑特定于我们的分析。下面是我们示例的简单实现：

```c++
class MetadataAnalysis : public ForwardDataFlowAnalysis<MetadataLatticeValue> {
public:
  using ForwardDataFlowAnalysis<MetadataLatticeValue>::ForwardDataFlowAnalysis;

  ChangeResult visitOperation(
      Operation *op, ArrayRef<LatticeElement<ValueT> *> operands) override {
    DictionaryAttr metadata = op->getAttrOfType<DictionaryAttr>("metadata");

    // 如果此操作没有元数据，我们将保守地将所有结果标记为已达到悲观不动点。
    if (!metadata)
      return markAllPessimisticFixPoint(op->getResults());

    // 否则，我们将为元数据计算一个格值，并将其合并到所有结果的当前格元素中。
    MetadataLatticeValue latticeValue(metadata);
    ChangeResult result = ChangeResult::NoChange;
    for (Value value : op->getResults()) {
      // 我们通过 `getLatticeElement` 获取 `value` 的格元素，
      // 然后将其与该操作元数据的格值进行合并。注意，在分析阶段，
      // 可以自由地为一个值创建新的格元素。这就是为什么我们在这里不使用
      // `lookupLatticeElement` 方法。
      result |= getLatticeElement(value).join(latticeValue);
    }
    return result;
  }
};
```

至此，我们拥有了计算分析所需的所有组件。分析计算完成后，我们可以使用 `lookupLatticeElement` 获取值的任何已计算信息。我们使用此函数而不是 `getLatticeElement`，是因为分析不能保证访问所有值（例如，如果值位于不可达块中），并且我们不希望在此情况下创建新的未初始化格元素。请参阅下面的快速示例：

```c++
void MyPass::runOnOperation() {
  MetadataAnalysis analysis(&getContext());
  analysis.run(getOperation());
  ...
}

void MyPass::useAnalysisOn(MetadataAnalysis &analysis, Value value) {
  LatticeElement<MetadataLatticeValue> *latticeElement = analysis.lookupLatticeElement(value);

  // 如果我们没有元素，说明 `value` 在分析期间未被访问，这意味着它可能是死代码。
  // 我们需要保守地处理这种情况。
  if (!lattice)
    return;

  // 我们的格元素有一个值，使用它：
  MetadataLatticeValue &value = lattice->getValue();
  ...
}
```
