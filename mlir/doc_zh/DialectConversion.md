# 方言转换

本文档描述了 MLIR 中用于在方言之间及方言内部执行操作转换的框架。该框架通过一组基于模式的操作重写模式，将非法操作转换为转换目标所支持的操作。

方言转换框架由以下组件构成：

*   一个[转换目标](#conversion-target)
*   一组[重写模式](#rewrite-pattern-specification)
*   一个[类型转换器](#type-conversion)（可选）

[TOC]

## 转换模式

在对一组操作应用转换时，可以选择几种不同的转换模式：

*   部分转换

    -   部分转换会将尽可能多的操作合法化到目标，但允许未被显式标记为"非法"的已有操作保持未转换状态。这允许在存在未知操作的情况下对输入的部分内容进行降级。
    -   可以通过 `applyPartialConversion` 应用部分转换。

*   完全转换

    -   完全转换会合法化所有输入操作，仅在所有操作都被正确合法化到给定转换目标时才成功。这确保在转换过程结束后只存在已知操作。
    -   可以通过 `applyFullConversion` 应用完全转换。

*   分析转换

    -   分析转换会分析哪些操作在应用转换时可以合法化到给定的转换目标。这通过执行"部分"转换并记录哪些操作在成功时会被成功转换来完成。请注意，实际上不会对输入操作应用任何重写或转换。
    -   可以通过 `applyAnalysisConversion` 应用分析转换。

在所有情况下，框架都以前序方式遍历操作，在遍历其包含的任何 Region 中的操作之前先检查该操作。

## 转换目标

转换目标是对转换过程中什么被认为是合法的正式定义。转换框架最终生成的操作必须在 `ConversionTarget` 上被标记为合法，重写才能成功。根据转换模式，现有操作不一定总是需要合法。操作和方言可以用以下任何提供的合法性动作来标记：

*   合法（Legal）

    -   此动作表示给定操作的每个实例都是合法的，即属性、操作数、类型等的任意组合都是有效的。

*   动态（Dynamic）

    -   此动作表示给定操作只有某些实例是合法的。这允许定义细粒度约束，例如说明 `arith.addi` 仅在对 32 位整数运算时才是合法的。

*   非法（Illegal）

    -   此动作表示给定操作没有任何实例是合法的。标记为"非法"的操作必须始终被转换才能使转换成功。此动作还允许在其他情况下合法的方言中有选择地将特定操作标记为非法。

既未被显式标记为合法也未被标记为非法的操作和方言与上述情况不同（称为"未知"操作），处理方式也不同，例如，对于上述部分转换的目的。

下面展示了一个转换目标示例：

```c++
struct MyTarget : public ConversionTarget {
  MyTarget(MLIRContext &ctx) : ConversionTarget(ctx) {
    //--------------------------------------------------------------------------
    // 将操作标记为合法：

    /// 将 LLVM 方言中的所有操作标记为合法。
    addLegalDialect<LLVMDialect>();

    /// 将 `arith.constant` 操作标记为在此目标上始终合法。
    addLegalOp<arith::ConstantOp>();

    //--------------------------------------------------------------------------
    // 将操作标记为动态合法。

    /// 将 Affine 方言中的所有操作标记为具有动态合法性约束。
    addDynamicallyLegalDialect<affine::AffineDialect>(
        [](Operation *op) { ... });

    /// 将 `func.return` 标记为动态合法，并提供特定的合法性回调。
    addDynamicallyLegalOp<func::ReturnOp>([](func::ReturnOp op) { ... });

    /// 将未知操作（即未直接设置合法化动作的操作）视为动态合法。
    markUnknownOpDynamicallyLegal([](Operation *op) { ... });

    //--------------------------------------------------------------------------
    // 将操作标记为非法。

    /// GPU 方言中的所有操作都是非法的。
    addIllegalDialect<GPUDialect>();

    /// 将 `cf.br` 和 `cf.cond_br` 标记为非法。
    addIllegalOp<cf::BranchOp, cf::CondBranchOp>();
  }

  /// 实现默认合法化处理器以处理标记为动态合法但未提供显式处理器的操作。
  bool isDynamicallyLegal(Operation *op) override { ... }
};
```

### 递归合法性

在某些情况下，可能希望将整个 Region 标记为合法。这为"合法"概念提供了额外的上下文粒度。如果一个操作被标记为递归合法（静态或动态），那么嵌套在其中的所有操作也被认为是合法的，即使它们本身会被认为是"非法"的。可以通过 `markOpRecursivelyLegal<>` 来标记操作：

```c++
ConversionTarget &target = ...;

/// 操作必须首先被标记为 `Legal` 或 `Dynamic`。
target.addLegalOp<MyOp>(...);
target.addDynamicallyLegalOp<MySecondOp>(...);

/// 将操作标记为始终递归合法。
target.markOpRecursivelyLegal<MyOp>();
/// 可选地使用回调来允许选择性标记。
target.markOpRecursivelyLegal<MyOp, MySecondOp>([](Operation *op) { ... });
/// 可选地使用回调来允许选择性标记。
target.markOpRecursivelyLegal<MyOp>([](MyOp op) { ... });
```

## 重写模式规范

定义转换目标之后，必须提供一组合法化模式，将非法操作转换为合法操作。此处提供的模式具有与主要 [Pattern](PatternRewriter.md) 文档中描述的相同结构和类似限制。提供的模式不需要生成在目标上直接合法的操作。框架会自动构建一个转换图，将非法操作转换为一组合法操作。

例如，假设您定义了一个支持一种操作的目标：`foo.add`。当提供以下模式：[`bar.add` -> `baz.add`，`baz.add` -> `foo.add`] 时，框架将自动检测到可以将 `bar.add` -> `foo.add` 合法化，即使不存在直接转换。这意味着您不必为 `bar.add` -> `foo.add` 定义直接合法化模式。

### 转换模式

除了通用的 `RewritePattern` 类之外，转换框架还提供了一种特殊类型的重写模式，当模式依赖于与转换过程特定构造的交互时可以使用，即 `ConversionPattern`。

#### 重新映射的操作数 / 适配器
转换模式为 `matchAndRewrite` 方法提供了一个额外的 `operands` / `adaptor` 参数。这些操作数对应于匹配操作的各个操作数最近的替换值。

```c++
struct MyConversionPattern : public ConversionPattern {
  /// ConversionPattern 上的 `matchAndRewrite` 钩子接受一个额外的
  /// `operands` 参数，包含原始操作的重新映射操作数。
  virtual LogicalResult
  matchAndRewrite(Operation *op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const;
};
```

示例：

```mlir
%0 = "test.foo"() : () -> i1  // 被模式 A 匹配
"test.bar"(%0) : (i1) -> ()   // 被模式 B 匹配
```

假设两个模式被背靠背应用：首先，模式 A 将 `"test.foo"` 替换为 `"test.qux"`，一个具有不同结果类型的操作。方言转换基础设施对这种改变类型的 IR 修改有特殊支持。

```mlir
%0 = "test.qux"() : () -> i2
%r = builtin.unrealized_conversion_cast %0 : i2 to i1
"test.bar"(%r) : (i1) -> ()
```

在 `replaceOp` 调用期间简单地替换 `"test.bar"` 的操作数是不安全的，因为这会改变操作数的类型，从而可能改变操作的语义。相反，方言转换驱动程序（概念上）插入一个 `builtin.unrealized_conversion_cast` 操作，将新创建的 `"test.qux"` 操作与 `"test.bar"` 操作连接起来，而不改变后者的类型。

现在，第二个模式 B 被应用。`operands` 参数包含具有最新替换类型（类型为 `i2` 的 `%0`）的 SSA 值，而从匹配操作查询操作数仍然返回具有原始操作数类型 `i1` 的 SSA 值。

注意：如果转换模式是使用类型转换器实例化的，`operands` 参数包含类型与类型转换器所规定的合法化操作数类型相匹配的 SSA 值。有关更多详细信息，请参见[类型安全](#type-safety)。

注意：方言转换框架不保证 `operands` 参数中存在任何特定值。唯一保证的是 `operands` SSA 值的类型。例如，`operands` 可能包含转换驱动程序插入但在整个转换过程中通常会折叠掉的临时 `builtin.unrealized_conversion_cast` 操作的结果，而不是提供给 `replaceOp` API 调用的实际替换值。

#### 立即 IR 修改与延迟 IR 修改

方言转换驱动程序可以在两种模式下运行：(a) 回滚模式（默认）和 (b) 无回滚模式。这可以通过 `ConversionConfig::allowPatternRollback` 控制。在回滚模式下运行时，当当前合法化路径（模式应用序列）遇到无法合法化的操作而卡住时，驱动程序能够回溯并回滚已经应用的模式。

在无回滚模式下运行时，所有 IR 修改（如操作替换、操作删除、操作插入或就地操作修改）会立即应用。

在回滚模式下运行时，某些 IR 修改会延迟到转换过程结束时执行。例如，`ConversionPatternRewriter::eraseOp` API 调用不会立即删除操作，而只是将其标记为待删除。在整个转换过程中，该操作对模式和 IR 遍历仍然可见。再如，`replaceOp` 和 `replaceAllUsesWith` 不会立即更新原始 SSA 值的使用者。此步骤也会延迟到转换过程结束时执行。

延迟某些 IR 修改有两个好处：(1) 模式回滚更简单，因为需要回滚的 IR 修改更少；(2) 在回滚时保留了已删除操作/块的指针；(3) 模式仍然可以在一定程度上访问/遍历原始 IR。但是，支持模式回滚需要以复杂的内部 C++ 数据结构形式进行额外的记账。在回滚模式下运行对编译时间有显著影响，容易出错，并使调试转换 pass 更加复杂。因此，鼓励程序员在可能时在无回滚模式下运行。

下表概述了在回滚模式下哪些 IR 更改以延迟方式应用。

| 类型                                                    | 回滚模式       | 无回滚模式  |
| ------------------------------------------------------- | -------------- | ----------- |
| 操作插入/移动（`create`/`insert`）                      | 立即           | 立即        |
| 操作替换（`replaceOp`）                                 | 延迟           | 立即        |
| 操作删除（`eraseOp`）                                   | 延迟           | 立即        |
| 操作修改（`modifyOpInPlace`）                           | 立即           | 立即        |
| 值替换（`replaceAllUsesWith`）                          | 延迟           | 立即        |
| 块插入（`createBlock`）                                 | 立即           | 立即        |
| 块替换                                                  | 不支持         | 不支持      |
| 块删除                                                  | 部分延迟       | 立即        |
| 块签名转换（`applySignatureConversion`）                | 部分延迟       | 立即        |
| Region/块内联（`inlineBlockBefore` 等）                 | 部分延迟       | 立即        |

值替换在回滚模式下会延迟并具有不同的语义：由于实际替换会延迟到转换过程结束，在 `replaceAllUsesWith` 调用之后仍然可以创建对已替换值的额外使用。这些使用也将在转换过程结束时被替换。

块替换在两种模式下都不支持，因为重写器基础设施目前没有用于替换块的 API：没有接受 `Block *` 的 `replaceAllUsesWith` 重载。

块删除在回滚模式下是部分延迟的：块从 IR 图中分离，但直到转换过程结束才释放块的内存。此机制确保在回滚块删除时块指针不会改变。

块签名转换是块插入、操作插入、值替换和块删除的组合。在回滚模式下，前两步是立即的，但后两步是延迟的。

Region/块内联是块/操作插入和（可选）值替换的组合。在回滚模式下，插入步骤是立即的，但替换步骤是延迟的。

注意：在回滚模式下运行时，转换驱动程序插入较少的临时 `builtin.unrealized_conversion_cast` 操作。由于某些 IR 修改是延迟的，使得以类型安全的方式连接旧的（尚未重写的）和新的（已经重写的）IR 变得不必要，因此这类操作较少需要。这对调试体验有负面影响：在整个转换过程中转储 IR 时，用户会看到旧 IR 和新 IR 的混合，但它们的连接方式在 IR 中并不总是可见的。这些信息的一部分存储在内部 C++ 数据结构中，在 IR 转储期间不可见。

#### 类型安全

提供给转换模式（通过适配器或操作数的 `ArrayRef`）的重新映射操作数的类型取决于类型转换规则。

如果模式是使用[类型转换器](#type-converter)初始化的，转换驱动程序会传递类型与类型转换器所规定的匹配操作的合法化操作数类型相匹配的值。为此，转换驱动程序可能会插入目标物化，将最近映射的值转换为预期的合法化类型。驱动程序尽力重用现有的物化，但基础设施不保证这一点。如果匹配操作的操作数类型无法合法化，则在调用 `matchAndRewrite` 钩子之前，模式应用失败。

示例：
```c++
// 将所有 FloatType 转换为 IntegerType 的类型转换器。
TypeConverter converter;
converter.addConversion([](FloatType t) {
    return IntegerType::get(t.getContext(), t.getWidth());
});

// 假设 `MyConversionPattern` 是使用 `converter` 初始化的。
struct MyConversionPattern : public ConversionPattern {
  virtual LogicalResult
  matchAndRewrite(Operation *op, ArrayRef<Value> operands, /* ... */) const {
//                                               ^^^^^^^^
//      如果 `op` 有一个 FloatType 操作数，`operands` 中对应的值
//      保证具有合法化的 IntegerType。如果另一个模式
//      之前用合法化类型的 SSA 值替换了操作数 SSA 值
//      （通过 "replaceOp" 或 "applySignatureConversion"），
//      您将直接获得该 SSA 值（除非该替换值也被替换了）。
//      否则，您将获得到合法化类型的物化。
```

如果模式是在没有类型转换器的情况下初始化的，转换驱动程序会将最近映射的值传递给模式，排除任何物化。如果需要与原始操作数相同类型的值，用户可以直接从匹配的操作中获取相应的操作数。

示例：在没有类型转换器的情况下初始化上面示例的模式时，`operands` 包含最近的替换值，无论其类型如何。

注意：在没有类型转换器的情况下运行时，物化被有意地从查找过程中排除，因为它们的存在可能取决于其他模式。传递物化会使转换基础设施变得脆弱和不可预测。此外，可能存在多种类型的物化。（当多个模式使用不同的类型转换器运行时可能会出现这种情况。）在这种情况下，不清楚应该传递哪种物化。

上述规则确保模式不必显式确保类型安全，或对传入的重新映射操作数的类型进行净化。关于类型转换的更多信息在下面的[专门章节](#type-conversion)中详细介绍。

## 类型转换

有时在转换过程中需要转换正在操作的类型集合。在这些情况下，可以定义一个 `TypeConverter` 对象，详细说明在与模式交互时应该如何转换类型。`TypeConverter` 可以用于转换块参数和 Region 的签名，定义模式的预期输入类型，以及通常情况下协调类型差异。

### 类型转换器

`TypeConverter` 包含几个钩子，用于详细说明如何转换类型，以及如何在各种情况下物化类型之间的转换。`TypeConverter` 的两个主要方面是转换和物化。

`conversion`（转换）描述了给定的源 `Type` 应该如何转换为 N 个目标类型。如果源类型被转换为自身，我们说它是"合法"类型。类型转换通过下面描述的 `addConversion` 方法指定。

有两种转换函数：上下文感知的和上下文无关的转换。上下文无关的转换函数将 `Type` 转换为 `Type`。上下文感知的转换函数将 `Value` 转换为类型。后者允许用户根据 IR 自定义类型转换规则。

注意：上下文感知的类型转换函数会影响框架缓存转换结果的能力。在没有上下文感知转换的情况下，所有上下文无关的类型转换都可以被缓存。否则，只有在上下文感知类型转换之后添加的上下文无关转换才能被缓存（转换以逆序应用）。因此，建议在 `addConversion` 调用序列中尽早添加上下文感知转换（使其最后应用）。

`materialization`（物化）描述了如何将一组值转换为具有特定类型的值列表。与 `conversion` 的一个重要区别是，`materialization` 可以产生 IR，而 `conversion` 不能。这些物化被转换框架用于确保转换过程中的类型安全。根据情况，有几种类型的物化。

*   源物化（Source Materialization）

    -   当一个值被替换为不同类型的值，但在转换过程结束时仍有使用者期望原始（"源"）类型时，使用源物化。源物化将替换值转换回源类型。
    -   此物化在以下情况下使用：
        *   当一个块参数已被转换为不同的类型，但原始参数仍有在转换过程完成后将保持活跃的使用者。
        *   当一个块参数已被删除，但该参数仍有在转换过程完成后将保持活跃的使用者。
        *   当一个操作的结果类型已被转换为不同的类型，但原始结果仍有在转换过程完成后将保持活跃的使用者。

*   目标物化（Target Materialization）

    -   目标物化将一个值转换为转换模式的类型转换器所期望的类型。
    -   当模式期望重新映射的操作数为某组特定类型，但原始输入操作数尚未被替换或已被替换为不同类型的值时，使用目标物化。

如果一个转换后的值被一个未转换的操作使用，它需要转换回 `source` 类型，因此需要源物化；如果一个未转换的值被一个正在转换的操作使用，它需要转换为 `target` 类型，因此需要目标物化。

如上所述，转换过程保证在转换过程中保留 IR 的类型契约。这意味着值使用的类型在转换过程中不会隐式改变。当值定义（块参数或操作结果）的类型被改变时，该定义的使用者也必须在转换过程中被更新。如果没有更新，必须物化一个类型转换，以确保 IR 中仍然存在预期类型的值。如果需要物化但无法执行，则整个转换过程失败。

下面详细介绍了几个可用的钩子：

```c++
class TypeConverter {
 public:
  /// 注册一个转换函数。转换函数必须可转换为以下任意形式
  /// （其中 `T` 是 `Value` 或从 `Type` 派生的类，包括 `Type` 本身）：
  ///
  ///   * std::optional<Type>(T)
  ///     - 此形式表示 1-1 类型转换。它应该返回 nullptr
  ///       或 `std::nullopt` 以表示失败。如果返回 `std::nullopt`，
  ///       允许转换器尝试另一个转换函数来执行转换。
  ///   * std::optional<LogicalResult>(T, SmallVectorImpl<Type> &)
  ///     - 此形式表示 1-N 类型转换。它应该返回
  ///       `failure` 或 `std::nullopt` 以表示转换失败。如果
  ///       新类型集合为空，则删除该类型，并且期望在转换过程中
  ///       删除现有值的所有使用。如果返回 `std::nullopt`，
  ///       允许转换器尝试另一个转换函数来执行转换。
  ///
  /// 接受 `Value` 作为第一个参数的转换函数是上下文感知的，
  /// 即它们在转换给定值的类型时可以考虑 IR。
  /// 上下文无关的转换函数接受 `Type` 或派生类作为第一个参数。
  ///
  /// 注意：上下文无关的转换会被缓存，但上下文感知的转换不会。
  ///
  /// 注意：在尝试转换类型时，例如通过 'convertType'，
  ///       最近添加的转换将首先被调用。
  template <typename FnT,
            typename T = typename llvm::function_traits<FnT>::template arg_t<0>>
  void addConversion(FnT &&callback) {
    registerConversion(wrapCallback<T>(std::forward<FnT>(callback)));
  }

  /// 以下所有物化都需要可转换为以下形式的函数对象：
  ///   `std::optional<Value>(OpBuilder &, T, ValueRange, Location)`,
  /// 其中 `T` 是 `Type` 的任意子类。此函数负责使用提供的
  /// OpBuilder 和 Location 创建一个操作，将一系列值"转换"为
  /// 给定类型 `T` 的单个值。成功时必须返回转换类型的 Value，
  /// 失败但可以尝试其他物化时返回 `std::nullopt`，
  /// 不可恢复的失败时返回 `nullptr`。它只会对 `T` 的（子）类型调用。
  /// 当类型转换可能在转换完成后持续存在时，必须提供物化函数。

  /// 此方法注册一个物化，当将替换值转换回其原始源类型时调用。
  /// 当原始值的某些使用在主转换之后仍然存在时使用此方法。
  template <typename FnT,
            typename T = typename llvm::function_traits<FnT>::template arg_t<1>>
  void addSourceMaterialization(FnT &&callback) {
    sourceMaterializations.emplace_back(
        wrapSourceMaterialization<T>(std::forward<FnT>(callback)));
  }

  /// 此方法注册一个物化，当根据模式的类型转换器将值转换为目标类型时调用。
  ///
  /// 注意：目标物化可以可选地检查"原始"类型。此类型可能与输入值的类型不同。
  /// 例如，假设转换模式 "P1" 将 SSA 值 "v1"（类型 "t1"）替换为 "v2"（类型 "t2"）。
  /// 然后另一个转换模式 "P2" 匹配一个以 "v1" 为操作数的操作。假设 "P2" 确定
  /// "t1" 的转换目标类型是 "t3"，这可能与 "t2" 不同。在这个例子中，
  /// 目标物化将被调用，参数为：outputType = "t3"，inputs = "v2"，
  /// originalType = "t1"。注意，原始类型 "t1" 不能仅从 "t3" 和 "v2" 恢复；
  /// 这就是 originalType 参数存在的原因。
  ///
  /// 注意：在 1:N 转换期间，结果类型可以是 TypeRange。在这种情况下，
  /// 物化会产生一个 SmallVector<Value>。
  template <typename FnT,
            typename T = typename llvm::function_traits<FnT>::template arg_t<1>>
  void addTargetMaterialization(FnT &&callback) {
    targetMaterializations.emplace_back(
        wrapTargetMaterialization<T>(std::forward<FnT>(callback)));
  }
};
```

通过类型转换器的物化是可选的。如果 `ConversionConfig::buildMaterializations` 标志设置为 "false"，方言转换驱动程序将构建一个 `unrealized_conversion_cast` 操作，而不是在需要物化时调用相应的类型转换器回调。

### Region 签名转换

从类型转换的角度来看，块参数的类型有些特殊。在整个转换过程中，块可能在不同操作的 Region 之间移动。鉴于此，块类型的转换必须通过转换模式显式完成。

要转换 Region 中块参数的类型，必须调用 `ConversionPatternRewriter` 上的自定义钩子：`convertRegionTypes`。此钩子使用提供的类型转换器将类型转换应用于给定 Region 的所有块。此钩子还接受一个可选的 `TypeConverter::SignatureConversion` 参数，该参数将自定义转换应用于 Region 的入口块。入口块参数的类型通常在语义上与操作相关联，例如 `func::FuncOp`、`AffineForOp` 等。

要仅转换给定块的签名，可以使用 `applySignatureConversion` 钩子。

签名转换 `TypeConverter::SignatureConversion` 可以通过编程方式构建：

```c++
class SignatureConversion {
public:
    /// 将原始签名的输入重新映射为一组新类型。
    /// 新类型附加到新签名转换中。
    void addInputs(unsigned origInputNo, ArrayRef<Type> types);

    /// 将新输入类型附加到签名转换中，仅当新类型不打算
    /// 重新映射现有输入时使用。
    void addInputs(ArrayRef<Type> types);

    /// 将原始签名的输入重新映射为新签名中的类型范围。
    void remapInput(unsigned origInputNo, unsigned newInputNo,
                    unsigned newInputCount = 1);

    /// 将原始签名的输入重新映射为另一个 `replacement` 值。
    /// 这将删除原始参数。
    void remapInput(unsigned origInputNo, Value replacement);
};
```

`TypeConverter` 为签名转换和合法性检查提供了几个默认实用程序：
`convertSignatureArgs`/`convertBlockSignature`/`isLegal(Region *|Type)`。

## 调试

要调试方言转换框架的执行，可以使用 `-debug-only=dialect-conversion`。此命令行标志仅为转换框架激活 LLVM 的调试日志记录基础设施。输出格式为树形结构，反映了转换过程的结构。此输出包含重写器执行的所有操作、生成的操作如何被合法化以及为何失败。

下面显示了示例输出：

```
//===-------------------------------------------===//
Legalizing operation : 'func.return'(0x608000002e20) {
  "func.return"() : () -> ()

  * Fold {
  } -> FAILURE : unable to fold

  * Pattern : 'func.return -> ()' {
    ** Insert  : 'spirv.Return'(0x6070000453e0)
    ** Replace : 'func.return'(0x608000002e20)

    //===-------------------------------------------===//
    Legalizing operation : 'spirv.Return'(0x6070000453e0) {
      "spirv.Return"() : () -> ()

    } -> SUCCESS : operation marked legal by the target
    //===-------------------------------------------===//
  } -> SUCCESS : pattern applied successfully
} -> SUCCESS
//===-------------------------------------------===//
```

此输出描述了 `func.return` 操作的合法化过程。我们首先尝试通过折叠操作来合法化，但对于 `func.return` 而言这不成功。然后，应用一个将 `func.return` 替换为 `spirv.Return` 的模式。新生成的 `spirv.Return` 随后被处理以进行合法化，但发现已经根据目标是合法的。
