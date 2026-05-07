# 操作定义规范（ODS）

除了特化 `mlir::Op` C++ 模板之外，MLIR 还支持以表格驱动（table-driven）的方式定义操作（operation）和数据类型。这通过 [TableGen][TableGen] 实现——TableGen 是一种通用语言及其配套工具，用于维护特定领域信息的记录。操作的相关事实被简洁地写入 TableGen 记录，并在编译器构建时展开为等价的 `mlir::Op` C++ 模板特化。

本手册详细介绍了以这种表格驱动方式定义操作的所有可用机制，旨在作为规范文档而非教程。如需教程，请参考[快速入门：向 MLIR 添加图重写](../Tutorials/QuickstartRewrites.md)。

本手册在详述每种机制的同时，也尝试总结最佳实践，以引用列表的形式呈现。

[TOC]

## 动机

MLIR 支持可插拔的方言（dialect），方言中包含（除其他内容外）一系列操作。这种开放且可扩展的生态系统带来了"字符串化"类型 IR（intermediate representation，中间表示）问题，例如：优化和分析遍（pass）中重复的字符串比较、不直观的访问器方法（如通用/易出错的 `getOperand(3)` 对比自文档化的 `getStride()`）及更泛化的返回类型、冗长的无默认参数泛型构造函数、冗长的文本 IR 转储，等等。此外，操作验证存在以下问题：

1.  最好情况：一个从字符串到验证函数的集中映射表，
1.  中间情况：验证逻辑在代码库中重复，或
1.  最坏情况：没有任何验证函数。

解决方案是支持以表格驱动的方式定义操作（op）。这样，每个方言都可以有一个集中的地方，包含关于每个操作的所有必要信息，包括其约束条件、自定义汇编格式等。这些描述也用于生成辅助函数和类，以支持构建、验证、解析、打印、分析等众多功能。

## 优势

与 C++ 模板相比，这种表格驱动的方式具有以下几点优势（但不限于此）：

*   **单一事实来源**：我们力求将操作的所有事实编码进记录中，让读者无需在多处代码片段之间跳转即可完全理解一个操作。
*   **消除样板代码**：我们可以从记录中自动生成操作数/属性/结果的 getter 方法、操作构建方法、操作验证方法以及许多其他工具。这大大减少了定义新操作所需的样板代码。
*   **促进自动生成**：使用这些操作信息记录的场景绝不仅限于操作定义本身。我们可以利用它们驱动许多其他组件的自动生成，如计算图的序列化。

## TableGen 语法

我们使用 TableGen 作为指定操作信息的语言。TableGen 本身只提供编写记录的语法；TableGen 文件（通常以 `.td` 为后缀）中允许的语法和构造可以在[这里][TableGenProgRef]找到。

*   TableGen 的 `class` 类似于 C++ 的类，可以模板化和继承。
*   TableGen 的 `def` 类似于 C++ 的对象；它可以通过特化 TableGen `class` 来声明（例如 `def MyDef : MyClass<...>;`），也可以完全独立声明（例如 `def MyDef;`）。它不能进一步模板化或被继承。
*   TableGen 的 `dag` 是有向无环图（directed acyclic graph）元素的专用类型。一个 `dag` 有一个操作符和零个或多个参数，语法为 `(operator arg0, arg1, argN)`。操作符可以是任意 TableGen `def`；参数可以是任何内容，包括 `dag` 本身。我们可以给操作符和参数都附加名称，如 `(MyOp:$op_name MyArg:$arg_name)`。

请参阅[语言参考][TableGenProgRef]以了解 TableGen 支持的所有类型和表达式。

## 操作定义

MLIR 定义了若干公共构造来帮助定义操作，并通过专用的 [TableGen 后端][TableGenBackend]——[`OpDefinitionsGen`][OpDefinitionsGen]——提供其语义。这些构造定义于 [`OpBase.td`][OpBase]。主要包括：

*   `Op` 类：这是定义操作的主要构造。在特化该类时，借助以下构造来指定操作的所有事实。
*   `Dialect` 类：属于同一逻辑组的操作放在同一方言中。`Dialect` 类包含方言级别的信息。
*   `Trait` 类层次结构：用于指定操作的特殊属性和约束，包括操作是否有副作用，或其输出是否与输入形状相同。
*   `ins`/`outs` 标记：这是 `OpDefinitionsGen` 后端内置的两个特殊标记，分别引导操作数/属性和结果的定义。
*   `TypeConstraint` 类层次结构：用于指定对操作数或结果的约束。一个值得关注的子类层次结构是 `Type`，代表常见 C++ 类型的约束。
*   `AttrConstraint` 类层次结构：用于指定对属性（attribute）的约束。一个值得关注的子类层次结构是 `Attr`，代表值为常见类型的属性的约束。
*   `Property` 类层次结构：用于指定操作固有的、非属性支撑（non-attribute-backed）的属性（property）。这些属性可以使用 `predicate` 字段或 `ConfinedProp` 类来施加约束。`Property` 的超类 `PropConstraint` 用于在重写模式中描述对属性的约束。

操作通过用具体内容特化 `Op` 类来定义其所有必要字段。例如，`tf.AvgPool` 的定义如下：

```tablegen
def TF_AvgPoolOp : TF_Op<"AvgPool", [NoMemoryEffect]> {
  let summary = "Performs average pooling on the input.";

  let description = [{
Each entry in `output` is the mean of the corresponding size `ksize`
window in `value`.
  }];

  let arguments = (ins
    TF_FpTensor:$value,

    ConfinedAttr<I64ArrayAttr, [ArrayMinCount<4>]>:$ksize,
    ConfinedAttr<I64ArrayAttr, [ArrayMinCount<4>]>:$strides,
    TF_AnyStrAttrOf<["SAME", "VALID"]>:$padding,
    DefaultValuedAttr<TF_ConvertDataFormatAttr, "NHWC">:$data_format
  );

  let results = (outs
    TF_FpTensor:$output
  );

  TF_DerivedOperandTypeAttr T = TF_DerivedOperandTypeAttr<0>;
}
```

下面描述所有需要的字段。请参阅 `Op` 类的定义以获取支持字段的完整列表。

### 操作名称

操作名称是 MLIR 中操作的唯一标识符，例如 TensorFlow 方言中加法操作的 `tf.Add`。这相当于汇编语言中的助记符，用于文本格式中的解析和打印，也用于图重写中的模式匹配。

完整的操作名称由方言名称和操作名称组成，前者由方言提供，后者作为 `Op` 类的第二个模板参数提供。

### 操作文档

这包括一行 `summary`（摘要）和更长的人类可读 `description`（描述）。它们用于驱动方言文档的自动生成，需要在操作的定义体中提供：

```tablegen
let summary = "...";

let description = [{
...
}];
```

`description` 应使用 Markdown 语法编写。

建议将文档置于定义开头，以便于理解操作。

> *   将文档置于操作定义的开头。
> *   摘要应简短精炼，应为以大写字母开头、无尾部标点的单行文字。将扩展说明放入描述中。

### 操作参数

参数有三种：操作数（operand）、属性（attribute）和属性（property）。操作数是其他操作在运行时产生的值；而属性（attribute）和属性（property）是编译时已知的常量值，分为两类：

1.  固有属性（Natural attributes）：这些属性影响操作的行为（例如卷积的 padding）；
1.  派生属性（Derived attributes）：这些属性不是定义操作所必需的，而是从操作的信息中派生出来的。例如输出的类型形状。这主要用于生成便利接口或与其他框架/翻译器交互。

    所有派生属性都应能够以属性（Attribute）的形式具体化。即，即使它们没有被具体化，也应该可以将其存储为属性。

属性（Property）类似于属性（attribute），区别在于它们不存储在 MLIR 上下文中，而是与操作内联存储。

操作数、属性（attribute）和属性（property）在 `dag` 类型的 `arguments` 中指定，以 `ins` 引导：

```tablegen
let arguments = (ins
  <type-constraint>:$<operand-name>,
  ...
  <attr-constraint>:$<attr-name>,
  ...
  <property>:$<property-name>,
);
```

其中 `<type-constraint>` 是来自 `TypeConstraint` 类层次结构的 TableGen `def`。类似地，`<attr-constraint>` 是来自 `AttrConstraint` 类层次结构的 TableGen `def`，而 `<property>` 是 `Property` 的子类（可以使用其 `predicate` 字段或 `ConfinedProp` 子类对其施加约束）。

对操作数和属性的相对顺序没有要求，可以自由混合。操作数本身的相对顺序很重要。对于每个命名参数，都会生成一个命名 getter，返回该参数（对于属性，返回类型从存储类型构造；对于操作数，返回类型为 `Value`）。每个属性的原始值（即存储值）也可以通过生成的 `<name>Attr` getter 访问，用于转换遍（transformation pass）中，此时更用户友好的返回类型可能不够合适。

所有参数都应命名，以便：
- 提供文档，
- 驱动 getter 方法的自动生成，以及
- 提供在其他地方（如约束）引用的句柄。

#### 可变参数操作数（Variadic operands）

要声明可变参数操作数，用 `Variadic<...>` 包裹操作数的 `TypeConstraint`。

通常操作没有可变参数操作数，或只有一个可变参数操作数。对于后一种情况，很容易推断哪些动态操作数对应于静态的可变参数操作数定义。但是，如果操作有多个可变长度操作数（可选或可变参数），则在没有操作进一步信息的情况下，无法将动态操作数归属到对应的静态可变参数操作数定义。因此，需要使用 `SameVariadicOperandSize` 或 `AttrSizedOperandSegments` 特征（trait）来表明所有可变长度操作数具有相同数量的动态值。

#### VariadicOfVariadic 操作数

要声明具有可变数量子范围的可变参数操作数，用 `VariadicOfVariadic<..., "<segment-attribute-name>">` 包裹操作数的 `TypeConstraint`。

`VariadicOfVariadic` 的第二个字段是包含可变子范围大小的 `DenseI32ArrayAttr` 参数的名称。在确定子范围大小或更新子范围大小时将使用该属性。

#### 可选操作数（Optional operands）

要声明可选操作数，用 `Optional<...>` 包裹操作数的 `TypeConstraint`。

通常操作没有可选操作数，或只有一个可选操作数。对于后一种情况，很容易推断哪些动态操作数对应于静态操作数定义。但是，如果操作有多个可变长度操作数（可选或可变参数），则在没有操作进一步信息的情况下，无法将动态操作数归属到对应的静态可变参数操作数定义。因此，需要使用 `SameVariadicOperandSize` 或 `AttrSizedOperandSegments` 特征来表明所有可变长度操作数具有相同数量的动态值。

#### 可选属性（Optional attributes）

要声明可选属性，用 `OptionalAttr<...>` 包裹属性的 `AttrConstraint`。

#### 带默认值的属性

要声明带默认值的属性，用 `DefaultValuedAttr<..., "...">` 包裹属性的 `AttrConstraint`。

`DefaultValuedAttr` 的第二个参数应为包含 C++ 默认值的字符串。例如，浮点数默认值应指定为 `"0.5f"`，整数数组默认值应指定为 `"{1, 2, 3}"`。

当属性值等于默认值时，生成的操作打印函数不会打印该带默认值的属性。

对于枚举属性，可以使用 `DefaultValuedEnumAttr<EnumAttr, EnumCase>` 代替 `DefaultValuedAttr`。这允许使用 TableGen 的 `EnumCase` 变量而不是原始字符串来指定默认值。例如 `DefaultValuedEnumAttr<SomeI64Enum, I64Case5>`。

#### 限制属性（Confining attributes）

`ConfinedAttr` 作为一种通用机制，用于对超出值类型范围的属性建模进一步的约束。可以使用 `ConfinedAttr` 将更原始的约束组合成复杂约束。例如，最小值必须为 10 的 32 位整数属性可以表示为 `ConfinedAttr<I32Attr, [IntMinValue<10>]>`。

目前支持以下原始约束：

*   `IntMinValue<N>`：指定整数属性大于或等于 `N`
*   `IntMaxValue<N>`：指定整数属性小于或等于 `N`
*   `IntNEQValue<N>`：指定整数属性不等于 `N`
*   `IntPositive`：指定整数属性的值为正数
*   `IntNonNegative`：指定整数属性的值为非负数
*   `IntPowerOf2`：指定整数属性的值是大于 0 的 2 的幂
*   `ArrayMinCount<N>`：指定数组属性至少有 `N` 个元素
*   `ArrayMaxCount<N>`：指定数组属性至多有 `N` 个元素
*   `ArrayCount<N>`：指定数组属性恰好有 `N` 个元素
*   `DenseArrayCount<N>`：指定稠密数组属性恰好有 `N` 个元素
*   `DenseArrayStrictlyPositive<arrayType>`：指定类型为 `arrayType` 的稠密数组属性的所有元素为正数
*   `DenseArrayStrictlyNonNegative<arrayType>`：指定类型为 `arrayType` 的稠密数组属性的所有元素为非负数
*   `DenseArraySorted<arrayType>`：指定类型为 `arrayType` 的稠密数组属性的元素按非递减顺序排列
*   `DenseArrayStrictlySorted<arrayType>`：指定类型为 `arrayType` 的稠密数组属性的元素按递增顺序排列
*   `IntArrayNthElemEq<I, N>`：指定整数数组属性的第 `I` 个元素等于 `N`
*   `IntArrayNthElemMinValue<I, N>`：指定整数数组属性的第 `I` 个元素大于或等于 `N`
*   `IntArrayNthElemMaxValue<I, N>`：指定整数数组属性的第 `I` 个元素小于或等于 `N`
*   `IntArrayNthElemInRange<I, M, N>`：指定整数数组属性的第 `I` 个元素大于或等于 `M` 且小于或等于 `N`
*   `IsNullAttr`：指定一个必须为空的可选属性

TODO: 设计并实现更多原始约束

#### 可选和带默认值的属性（property）

要声明带默认值的属性（property），使用 `DefaultValuedProp<..., "...">`。如果属性（property）的存储数据类型与其接口类型不同（例如对于数组属性，存储为 `SmallVector` 但使用 `ArrayRef` 作为接口类型），则将默认值的存储类型等价物作为第三个参数添加。

在汇编格式中使用 `prop-dict` 指令时，当属性（property）值等于默认值时，生成的操作打印函数不会打印该带默认值的属性（property）。

要声明可选属性（property），使用 `OptionalProp<...>`。这将底层属性（property）包装在 `std::optional` 中，并给它一个默认值 `std::nullopt`。

#### 组合约束

`AllAttrOf` 用于允许组合多个必须同时成立的约束。

例如：
```tablegen
def OpAllAttrConstraint1 : TEST_Op<"all_attr_constraint_of1"> {
  let arguments = (ins I64ArrayAttr:$attr);
  let results = (outs I32);
}
def OpAllAttrConstraint2 : TEST_Op<"all_attr_constraint_of2"> {
  let arguments = (ins I64ArrayAttr:$attr);
  let results = (outs I32);
}
def Constraint0 : AttrConstraint<
    CPred<"::llvm::cast<::mlir::IntegerAttr>(::llvm::cast<ArrayAttr>($_self)[0]).getInt() == 0">,
    "[0] == 0">;
def Constraint1 : AttrConstraint<
    CPred<"::llvm::cast<::mlir::IntegerAttr>(::llvm::cast<ArrayAttr>($_self)[1]).getInt() == 1">,
    "[1] == 1">;
def : Pat<(OpAllAttrConstraint1
            AllAttrOf<[Constraint0, Constraint1]>:$attr),
          (OpAllAttrConstraint2 $attr)>;
```

### 操作区域（regions）

操作的区域在 `dag` 类型的 `regions` 中指定，以 `region` 引导：

```tablegen
let regions = (region
  <region-constraint>:$<region-name>,
  ...
);
```

#### 可变参数区域

类似于用于可变参数操作数和结果的 `Variadic` 类，`VariadicRegion<...>` 可用于区域。可变参数区域目前只能指定为区域列表中的最后一个区域。

### 操作结果（results）

类似于操作数，结果在 `dag` 类型的 `results` 中指定，以 `outs` 引导：

```tablegen
let results = (outs
  <type-constraint>:$<result-name>,
  ...
);
```

#### 可变参数结果

类似于可变参数操作数，`Variadic<...>` 也可用于结果。类似地，`SameVariadicResultSize` 用于同一操作中的多个可变参数结果。

### 操作后继者（successors）

对于终止符操作（terminator operation），后继者在 `dag` 类型的 `successors` 中指定，以 `successor` 引导：

```tablegen
let successors = (successor
  <successor-constraint>:$<successor-name>,
  ...
);
```

#### 可变参数后继者

类似于用于可变参数操作数和结果的 `Variadic` 类，`VariadicSuccessor<...>` 可用于后继者。可变参数后继者目前只能指定为后继者列表中的最后一个后继者。

### 操作特征（traits）与约束

特征（trait）是影响语法或语义的操作属性。MLIR C++ 在 `mlir::OpTrait` 命名空间中对各种特征建模。

操作特征、[接口](../Interfaces.md/#utilizing-the-ods-framework)，以及涉及多个操作数/属性/结果的约束，都作为 `Op` 类的第三个模板参数提供，它们应派生自 `Trait` 类。详见[约束](#约束)。

### 构建方法（builder methods）

对于每个操作，会根据参数和返回类型自动生成一些构建方法。例如，对于以下操作定义：

```tablegen
def MyOp : ... {
  let arguments = (ins
    I32:$i32_operand,
    F32:$f32_operand,
    ...,

    I32Attr:$i32_attr,
    F32Attr:$f32_attr,
    ...
    I32Prop:$i32_prop,
    ...
  );

  let results = (outs
    I32:$i32_result,
    F32:$f32_result,
    ...
  );
}
```

将生成以下构建方法：

```c++
// All result-types/operands/properties/discardable attributes have one
// aggregate parameter. `Properties` is the properties structure of
// `MyOp`.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  TypeRange resultTypes,
                  ValueRange operands,
                  Properties properties,
                  ArrayRef<NamedAttribute> discardableAttributes = {});

// All result-types/operands/attributes have one aggregate parameter.
// Inherent properties and discardable attributes are mixed together in the
//  `attributes` dictionary.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  TypeRange resultTypes,
                  ValueRange operands,
                  ArrayRef<NamedAttribute> attributes);

// Each result-type/operand/attribute has a separate parameter. The parameters
// for attributes are of mlir::Attribute types.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  Type i32_result, Type f32_result, ...,
                  Value i32_operand, Value f32_operand, ...,
                  IntegerAttr i32_attr, FloatAttr f32_attr, ...,
                  int32_t i32_prop);

// Each result-type/operand/attribute has a separate parameter. The parameters
// for attributes are raw values unwrapped with mlir::Attribute instances.
// (Note that this builder will not always be generated. See the following
// explanation for more details.)
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  Type i32_result, Type f32_result, ...,
                  Value i32_operand, Value f32_operand, ...,
                  APInt i32_attr, StringRef f32_attr, ...,
                  int32_t i32_prop, ...);

// Each operand/attribute has a separate parameter but result type is aggregate.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  TypeRange resultTypes,
                  Value i32_operand, Value f32_operand, ...,
                  IntegerAttr i32_attr, FloatAttr f32_attr, ...,
                  int32_t i32_prop, ...);

// All operands/attributes have aggregate parameters.
// Generated if return type can be inferred.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  ValueRange operands,
                  Properties properties,
                  ArrayRef<NamedAttribute> discardableAttributes);

// All operands/attributes have aggregate parameters.
// Generated if return type can be inferred. Uses the legacy merged attribute
// dictionary.
static void build(OpBuilder &odsBuilder, OperationState &odsState,
                  ValueRange operands, ArrayRef<NamedAttribute> attributes);

// (And manually specified builders depending on the specific op.)
```

前两种形式提供了基本的统一性，使我们可以用相同的形式创建操作，而不论具体是哪种操作。这在实现声明式模式重写时特别有用。

第三和第四种形式适合在手动编写的代码中使用，因为它们通过签名提供了更好的保证。

如果操作的某个属性的 `Attr.returnType` 与 `Attr.storageType` 不同，且我们知道如何从解包值构建属性（即定义了 `Attr.constBuilderCall`），则会生成第四种形式。此外，对于第三种形式，如果 `arguments` 列表中靠后的属性有默认值，则该默认值会在声明中提供。目前支持 `BoolAttr`、`StrAttr`、`EnumAttr`，未来列表可能扩展。因此，如果可能，应将带默认值的属性放在 `arguments` 列表末尾以利用此特性（这本质上是由于 C++ 函数参数默认值的放置限制导致的）。否则，第三种形式的构建方法仍会生成，但不在 `arguments` 列表末尾的属性的默认值不会在构建方法签名中提供。

在以下情况下，ODS 会生成不需要指定返回类型的构建方法：

*   操作实现了 `InferTypeOpInterface` 接口；
*   所有返回类型要么是可构建的类型，要么与给定操作数相同（例如，操作数和结果之间的 `AllTypesMatch` 约束）；

根据具体操作，还可能存在其他构建方法；请参阅[生成的 C++ 文件](#运行-mlir-tblgen-查看生成内容)获取完整列表。

#### 自定义构建方法

但是，如果上述情况无法满足所有需求，可以在 `builders` 字段中定义额外的便利构建方法，如下所示：

```tablegen
def MyOp : Op<"my_op", []> {
  let arguments = (ins F32Attr:$attr);

  let builders = [
    OpBuilder<(ins "float":$val)>
  ];
}
```

`builders` 字段是添加到 Op 类的自定义构建方法列表。在此示例中，我们提供了一个接受浮点值（而不是属性）的便利构建方法。`ins` 前缀在 ODS 的许多函数声明中很常见，它们使用 TableGen 的 [`dag`](#tablegen-语法)。后面跟着以逗号分隔的类型（带引号的字符串）和以 `$` 前缀的名称列表。这将生成如下构建方法的声明：

```c++
class MyOp : /*...*/ {
  /*...*/
  static void build(::mlir::OpBuilder &builder, ::mlir::OperationState &state,
                    float val);
};
```

注意该方法有两个额外的前导参数，它们用于构建操作。特别地，该方法必须用待构建操作的属性、操作数、区域和结果类型来填充 `state`。`builder` 可用于构建属于该操作的任何 IR 对象，如类型或嵌套操作。由于类型和名称将按原样生成到 C++ 代码中，它们应该是对于类型（在操作的命名空间中）和标识符的有效 C++ 构造（例如，`class` 不是有效的标识符）。

构建方法的实现可以直接在 ODS 中提供，使用 TableGen 代码块，如下所示：

```tablegen
def MyOp : Op<"my_op", []> {
  let arguments = (ins F32Attr:$attr);

  let builders = [
    OpBuilder<(ins "float":$val), [{
      $_state.addAttribute("attr", $_builder.getF32FloatAttr(val));
    }]>
  ];
}
```

`builder` 和 `state` 参数的等价物分别是特殊变量 `$_builder` 和 `$_state`。`ins` 部分列出的命名参数可以直接使用，例如 `val`。构建方法的主体将通过替换特殊变量生成，其余部分应为有效的 C++。虽然对代码大小没有限制，但我们鼓励在 ODS 中只内联定义简短的构建方法，将较长的构建方法定义放在 C++ 文件中。

最后，如果某些参数需要默认值，可以使用 `CArg` 来包裹类型和默认值，如下所示：

```tablegen
def MyOp : Op<"my_op", []> {
  let arguments = (ins F32Attr:$attr);

  let builders = [
    OpBuilder<(ins CArg<"float", "0.5f">:$val), [{
      $_state.addAttribute("attr", $_builder.getF32FloatAttr(val));
    }]>
  ];
}
```

生成的代码将在声明中使用默认值，但在定义中不使用，这符合 C++ 的要求：

```c++
/// Header file.
class MyOp : /*...*/ {
  /*...*/
  static void build(::mlir::OpBuilder &builder, ::mlir::OperationState &state,
                    float val = 0.5f);
};

/// Source file.
MyOp::build(::mlir::OpBuilder &builder, ::mlir::OperationState &state,
            float val) {
  state.addAttribute("attr", builder.getF32FloatAttr(val));
}
```

### 自定义解析器和打印器方法

用于解析和打印操作自定义汇编格式的函数。

### 自定义验证器代码

对于操作各实体上指定的[约束](#约束)，将自动生成验证代码。要执行_额外的_验证，可以使用：

```tablegen
let hasVerifier = 1;
let hasRegionVerifier = 1;
```

这将在操作类上生成 `LogicalResult verify()`/`LogicalResult verifyRegions()` 方法声明，可以在其中定义任何额外的验证约束。对于需要访问嵌套操作的验证，应使用 `hasRegionVerifier` 以确保不会访问任何格式不正确的操作。其他验证可以用 `hasVerifier` 实现。请查看下一节了解这些验证方法的执行顺序。

#### 验证顺序

操作的验证涉及几个步骤：

1. `StructuralOpTrait` 首先被验证，它们可以独立运行。
2. `verifyInvariants`，由 ODS 构建，验证类型、属性等。
3. 其他将验证器标记为 `verifyTrait` 或 `verifyWithRegions=0` 的特征/接口。
4. 在操作中定义并标记为 `hasVerifier=1` 的自定义验证器。

如果操作有区域，则可能有第二阶段：

1. 将验证器标记为 `verifyRegionTrait` 或 `verifyWithRegions=1` 的特征/接口。这意味着验证器需要访问其区域中的操作。
2. 在操作中定义并标记为 `hasRegionVerifier=1` 的自定义验证器。

注意，第二阶段将在区域中的操作被验证后运行。顺序靠后的验证器可以依赖前面验证器已经验证的某些不变量，无需重新验证它们。

#### 在自定义验证器中发出诊断信息

自定义验证器应避免使用自定义操作打印器来打印操作，因为这要求被打印的操作（有时还包括其父操作）首先通过验证。特别是在发出诊断信息时，自定义验证器应使用 `Error` 严重级别（该级别默认以通用形式打印操作），并避免使用较低的严重级别（`Note`、`Remark`、`Warning`）。

### 声明式汇编格式

操作的自定义汇编格式可以用声明式字符串来指定，该字符串匹配操作的操作数、属性等，并且能够表达构建操作时需要解析的额外信息：

```tablegen
def CallOp : Std_Op<"call", ...> {
  let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<AnyType>:$args);
  let results = (outs Variadic<AnyType>);

  let assemblyFormat = [{
    $callee `(` $args `)` attr-dict `:` functional-type($args, results)
  }];
}
```

格式由三个部分组成：

#### 指令（Directives）

指令是一种内置函数，带有可选的参数集。可用的指令如下：

*   `attr-dict`

    -   表示操作的属性字典。
    -   格式中未在其他地方使用的任何固有属性（inherent attribute）将作为属性字典的一部分打印，除非存在 `prop-dict`。
    -   可丢弃属性（discardable attribute）始终是 `attr-dict` 的一部分。

*   `attr-dict-with-keyword`

    -   表示操作的属性字典，但在字典前加上 `attributes` 关键字。

*   `prop-dict`

    -   表示操作的属性（property）转换为字典。
    -   格式中未在其他地方使用的任何属性（property）或固有属性（inherent attribute）都将作为此字典的一部分解析和打印。
    -   如果存在，`attr-dict` 将不包含任何固有属性。

*   `custom < UserDirective > ( Params )`

    -   表示由用户在 C++ 中实现的自定义指令。
    -   详见下面的[自定义指令](#自定义指令)部分。

*   `functional-type ( inputs , outputs )`

    -   将 `inputs` 和 `outputs` 参数格式化为[函数类型](../Dialects/Builtin.md/#functiontype)。
    -   `inputs` 和 `outputs` 的约束与 `type` 指令的 `input` 相同。

*   ``oilist ( `keyword` elements | `otherKeyword` elements ...)``

    -   表示可选的、顺序无关的子句列表。每个子句有一个关键字和对应的汇编格式。
    -   每个子句可以出现 0 次或 1 次（以任意顺序）。
    -   在 oilist 元素中只能使用字面量、类型和变量。
    -   所有变量必须是可选的或可变参数的。

*   `operands`

    -   表示操作的所有操作数。

*   `ref ( input )`

    -   表示对变量或指令的引用，该变量或指令必须已被解析，可用作 `custom` 指令的参数。
    -   用于将之前解析的实体传递给自定义指令。
    -   输入可以是任何指令或变量，除了 `functional-type` 和 `custom`。

*   `regions`

    -   表示操作的所有区域。

*   `results`

    -   表示操作的所有结果。

*   `successors`

    -   表示操作的所有后继者。

*   `type ( input )`

    -   表示给定输入的类型。
    -   `input` 必须是操作数或结果[变量](#变量)、`operands` 指令或 `results` 指令。

*   `qualified ( type_or_attribute )`

    -   包裹一个 `type` 指令或属性参数。
    -   用于强制以方言和助记符前缀打印类型或属性。例如，`vector.multi_reduction` 操作有一个 `kind` 属性；默认情况下声明式汇编会打印：`vector.multi_reduction <minf>, ...`，但在声明式汇编格式中使用 `qualified($kind)` 将会打印：`vector.multi_reduction #vector.kind<minf>, ...`。

#### 字面量（Literals）

字面量是用反引号包围的关键字或标点。

以下是有效的标点集合：

`:`, `,`, `=`, `<`, `>`, `(`, `)`, `{`, `}`, `[`, `]`, `->`, `?`, `+`, `*`

以下是有效的空白标点：

`\n`, ` `

`\n` 字面量发出一个换行符并缩进到操作的起始位置。示例如下：

```tablegen
let assemblyFormat = [{
  `{` `\n` ` ` ` ` `this_is_on_a_newline` `\n` `}` attr-dict
}];
```

```mlir
%results = my.operation {
  this_is_on_a_newline
}
```

空字面量 \`\` 可用于删除在某些字面量元素（如 `)`/`]` 等）之后隐式插入的空格。例如，"`]`" 在不是格式最后一个元素时可能输出 `] `。"`]` \`\`" 可以在这种情况下去除尾部空格。

#### 变量（Variables）

变量是已在操作本身注册的实体，即参数（属性或操作数）、区域、结果、后继者等。在上面的 `CallOp` 示例中，变量为 `$callee` 和 `$args`。

属性变量以其对应的值类型打印，除非该值类型是可构建的。在可构建的情况下，属性的类型被省略。

#### 自定义指令

声明式汇编格式规范能处理格式化操作时大多数常见情况。对于需要或希望以声明式语法不支持的形式指定部分操作的操作，可以使用自定义指令。自定义指令本质上允许用户使用 C++ 来打印和解析否则以声明式方式指定的格式的子段。查看上面的自定义指令规范：

```
custom-directive ::= `custom` `<` UserDirective `>` `(` Params `)`
```

自定义指令有两个主要部分：`UserDirective` 和 `Params`。自定义指令在生成格式的 C++ 代码时被转换为对 `print*` 和 `parse*` 方法的调用。`UserDirective` 是用作这两个调用后缀的标识符，即 `custom<MyDirective>(...)` 将分别在解析器和打印器中调用 `parseMyDirective` 和 `printMyDirective`。`Params` 可以是变量（即属性、操作数、后继者等）、类型指令、`attr-dict` 和 C++ 代码字符串的任意组合。类型指令必须引用一个变量，但该变量不必也是自定义指令的参数。

`parse<UserDirective>` 方法的参数首先是对 `OpAsmParser`（`OpAsmParser &`）的引用，其次是与格式中指定参数对应的一组输出参数。声明式参数到 `parse` 方法参数的映射如下：

*   属性变量
    -   单个：`<Attribute-Storage-Type>(e.g. Attribute) &`
    -   可选：`<Attribute-Storage-Type>(e.g. Attribute) &`
*   操作数变量
    -   单个：`OpAsmParser::UnresolvedOperand &`
    -   可选：`Optional<OpAsmParser::UnresolvedOperand> &`
    -   可变参数：`SmallVectorImpl<OpAsmParser::UnresolvedOperand> &`
    -   VariadicOfVariadic：`SmallVectorImpl<SmallVector<OpAsmParser::UnresolvedOperand>> &`
*   Ref 指令
    -   ref 指令使用与输入操作数相同的映射传递给解析器。例如，单个区域将作为 `Region &` 传递。
*   区域变量
    -   单个：`Region &`
    -   可变参数：`SmallVectorImpl<std::unique_ptr<Region>> &`
*   后继者变量
    -   单个：`Block *&`
    -   可变参数：`SmallVectorImpl<Block *> &`
*   类型指令
    -   单个：`Type &`
    -   可选：`Type &`
    -   可变参数：`SmallVectorImpl<Type> &`
    -   VariadicOfVariadic：`SmallVectorImpl<SmallVector<Type>> &`
*   `attr-dict` 指令：`NamedAttrList &`
*   `prop-dict` 指令：`OperationState &`

当变量是可选的时，只有在变量存在时才应指定值。否则，值应保持为 `None` 或 null。

`print<UserDirective>` 方法的参数首先是对 `OpAsmPrinter`（`OpAsmPrinter &`）的引用，其次是操作（例如 `FooOp op`，也可以是 `Operation *op`），最后是与格式中指定参数对应的一组输出参数。声明式参数到 `print` 方法参数的映射如下：

*   属性变量
    -   单个：`<Attribute-Storage-Type>(e.g. Attribute)`
    -   可选：`<Attribute-Storage-Type>(e.g. Attribute)`
*   操作数变量
    -   单个：`Value`
    -   可选：`Value`
    -   可变参数：`OperandRange`
    -   VariadicOfVariadic：`OperandRangeRange`
*   Ref 指令
    -   ref 指令使用与输入操作数相同的映射传递给打印器。例如，单个区域将作为 `Region &` 传递。
*   区域变量
    -   单个：`Region &`
    -   可变参数：`MutableArrayRef<Region>`
*   后继者变量
    -   单个：`Block *`
    -   可变参数：`SuccessorRange`
*   类型指令
    -   单个：`Type`
    -   可选：`Type`
    -   可变参数：`TypeRange`
    -   VariadicOfVariadic：`TypeRangeRange`
*   `attr-dict` 指令：`DictionaryAttr`
*   `prop-dict` 指令：`FooOp::Properties`

当变量是可选的时，提供的值可能为 null。当变量在自定义指令参数中使用 `ref` 引用时，它按值传递。传递给 `print<UserDirective>` 的被引用变量与绑定变量的传递方式相同，但传递给 `parse<UserDirective>` 的被引用变量则与打印器的传递方式相同。

自定义指令可以以 C++ 代码字符串作为参数。代码会逐字粘贴到自定义解析器和打印器的调用中，并进行 `$_builder` 和 `$_ctxt` 的替换。字符串字面量可以用于参数化自定义指令。

#### 可选组（Optional Groups）

在某些情况下，操作可能有"可选"信息，例如属性或空的可变参数操作数集。在这些情况下，汇编格式的某一部分可以根据此信息的存在与否标记为 `optional`。可选组的定义如下：

```
optional-group: `(` then-elements `)` (`:` `(` else-elements `)`)? `?`
```

可选组的元素有以下要求：

*   `then-elements` 的第一个元素必须是属性、字面量、操作数、属性（property）或区域。
    -   这是因为第一个元素必须是可选解析的。
    -   如果使用属性（property），它必须定义了 `optionalParser` 且有默认值。
*   在 `then-elements` 或 `else-elements` 中，恰好有一个参数变量或类型指令被标记为组的锚点（anchor）。
    -   锚点是其存在与否控制应打印/解析哪些元素的元素。
    -   通过在元素后添加 `^` 将其标记为锚点。
    -   第一个元素不_必须_是组的锚点。
    -   当一个非可变长度区域作为锚点时，打印组的检测条件是区域是否为空。
*   字面量、变量、自定义指令和类型指令是组内唯一有效的元素。
    -   任何属性变量都可以使用，但只有可选或带默认值的属性才能标记为锚点。带默认值的锚点在其持有的值不是默认值时被认为是存在的。
    -   只能使用可变参数或可选的结果和操作数参数。
    -   所有区域变量都可以使用。当使用非可变长度区域时，如果组不存在，区域为空。

具有可选组的操作示例是 `func.return`，它有可变数量的操作数：

```tablegen
def ReturnOp : ... {
  let arguments = (ins Variadic<AnyType>:$operands);

  // We only print the operands and types if there are a non-zero number
  // of operands.
  let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
}
```

##### Unit 属性

在 MLIR 中，[`unit` 属性](../Dialects/Builtin.md/#unitattr)是特殊的，因为它只有一个可能的值，即其意义来自于它的存在本身。当 unit 属性用于锚定可选组且不是组的第一个元素时，unit 属性的存在可以直接与可选组本身的存在相关联。因此，在这些情况下，unit 属性不会被打印或出现在输出中，并在解析时通过可选组本身的存在自动推断。

例如，以下操作：

```tablegen
def FooOp : ... {
  let arguments = (ins UnitAttr:$is_read_only);

  let assemblyFormat = "attr-dict (`is_read_only` $is_read_only^)?";
}
```

格式如下：

```mlir
// When the unit attribute is present:
foo.op is_read_only

// When the unit attribute is not present:
foo.op
```

同样的逻辑适用于 `UnitProp`。

##### 可选的"else"组

可选组还支持"else"元素组。这些是在可选组的 `anchor` 元素_不_存在时解析/打印的元素。与主元素组不同，"else"组对第一个元素没有限制，且没有任何元素可以作为可选的 `anchor`。示例如下：

```tablegen
def FooOp : ... {
  let arguments = (ins UnitAttr:$foo);

  let assemblyFormat = "attr-dict (`foo_is_present` $foo^):(`foo_is_absent`)?";
}
```

格式如下：

```mlir
// When the `foo` attribute is present:
foo.op foo_is_present

// When the `foo` attribute is not present:
foo.op foo_is_absent
```

#### 要求

格式规范有一定的要求必须遵守：

1.  输出和操作名称永远不显示，因为它们是固定的，不能更改。
1.  操作中的所有操作数必须出现在格式中，可以单独出现，也可以使用 `operands` 指令。
1.  操作中的所有区域必须出现在格式中，可以单独出现，也可以使用 `regions` 指令。
1.  操作中的所有后继者必须出现在格式中，可以单独出现，也可以使用 `successors` 指令。
1.  所有操作数和结果的类型必须使用各种 `type` 指令出现在格式中，可以单独出现，也可以使用 `operands` 或 `results` 指令。
1.  除非所有非属性（non-attribute）的属性（property）都出现在格式中，否则必须存在 `prop-dict` 指令。
1.  `attr-dict` 指令必须始终存在。
1.  不得包含重叠信息；例如 `attr-dict`、类型、操作数等的多个实例。
    -   注意，`attr-dict` 不与单个属性重叠。这些属性在打印属性字典时会被省略。

##### 类型推断

格式的一个要求是操作数和结果的类型必须始终存在。在某些情况下，变量的类型可以通过类型约束或其他可用信息推断出来。在这些情况下，该变量的类型可以从格式中省略。

*   可构建类型（Buildable Types）

某些类型约束可能只有一种表示形式，允许它们直接可构建；例如 `I32` 或 `Index` 类型。ODS 中的类型可以通过设置 `builderCall` 字段或继承自 `BuildableType` 类来标记为可构建。

*   特征相等约束（Trait Equality Constraints）

许多操作具有已知的类型相等约束，作为特征注册在操作上；例如 `select` 操作的 true、false 和结果值通常具有相同的类型。汇编格式可以检查这些相等约束来推断缺失变量的类型。目前支持的特征有：`AllTypesMatch`、`TypesMatchWith`、`SameTypeOperands` 和 `SameOperandsAndResultType`。

*   InferTypeOpInterface

实现了 `InferTypeOpInterface` 的操作可以在其汇编格式中省略结果类型，因为结果类型可以从操作数推断出来。

### `hasCanonicalizer`

此布尔字段表示是否为此操作定义了规范化（canonicalization）模式。如果为 `1`，则应定义 `::getCanonicalizationPatterns()`。

### `hasCanonicalizeMethod`

当此布尔字段设置为 `true` 时，表示操作实现了一个用于简单"matchAndRewrite"风格规范化模式的 `canonicalize` 方法。如果 `hasCanonicalizer` 为 0，则会实现 `::getCanonicalizationPatterns()` 来调用此函数。

### `hasFolder`

此布尔字段表示是否为此操作定义了通用折叠规则。如果为 `1`，则应定义 `::fold()`。

### 额外声明

表格驱动操作定义的目标之一是尽可能自动生成每个操作所需的逻辑和方法。尽管如此，总会有一些长尾情况无法覆盖。对于这些情况，可以使用 `extraClassDeclaration`。`extraClassDeclaration` 中的代码将逐字复制到生成的 C++ 操作类中。

注意，`extraClassDeclaration` 是为高级用户处理长尾情况的机制；对于尚未实现但广泛适用的情况，改进基础设施更为可取。

### 额外定义

在 TableGen 中定义被许多不同操作多次继承的基础操作类时，用户可能希望提供工具函数和接口函数的公共定义。然而，许多这些定义在 `extraClassDeclaration` 中可能不可取或不可能，因为它将它们追加到操作的 C++ 类声明中。在这些情况下，用户可以添加 `extraClassDefinition` 来定义代码，该代码将添加到生成的源文件中的操作 C++ 命名空间内。替换 `$cppClass` 将被操作的 C++ 类名替换。

### 生成的 C++ 代码

[OpDefinitionsGen][OpDefinitionsGen] 处理操作定义规范文件并生成两个包含相应 C++ 代码的文件：一个用于声明，另一个用于定义。前者通过 `-gen-op-decls` 命令行选项生成，后者通过 `-gen-op-defs` 选项生成。

定义文件包含所有操作方法定义，可以通过定义 `GET_OP_CLASSES` 来包含和启用。对于每个操作，OpDefinitionsGen 生成一个操作类和一个[操作数适配器](#操作数适配器)类。此外，它还包含所有已定义操作的逗号分隔列表，可以通过定义 `GET_OP_LIST` 来包含和启用。

#### 类名和命名空间

对于每个操作，其生成的 C++ 类名是去除方言前缀后的 TableGen `def` 符号。第一个 `_` 作为分隔符。例如，对于 `def TF_AddOp`，C++ 类名将是 `AddOp`。我们去除 `TF` 前缀是因为它是用于限定操作范围的；其他方言也可以定义自己的 `AddOp`。

生成的 C++ 类的命名空间来自方言的 `cppNamespace` 字段。例如，如果方言的 `cppNamespace` 是 `A::B`，则该方言的操作将放置在 `namespace A { namespace B { ... } }` 中。如果方言没有指定 `cppNamespace`，则使用方言的名称作为命名空间。

这意味着生成的 C++ 类的限定名称不一定与[操作名称](#操作名称)中解释的操作名称完全匹配。这是为了允许灵活命名以满足编码风格要求。

#### 操作数适配器

对于每个操作，我们自动生成一个_操作数适配器_（operand adaptor）。此类解决了在不使用"魔法"常量的情况下访问以 `Value` 列表形式提供的操作数的问题。操作数适配器接受对 `Value` 数组的引用，并提供与操作类中相同名称的方法来访问它们。例如，对于二元算术操作，它可能提供 `.lhs()` 来访问第一个操作数，`.rhs()` 来访问第二个操作数。

操作数适配器类与操作类在同一命名空间中，其名称为操作名称加上 `Adaptor`，在操作类内部也有一个别名 `Adaptor`。

操作数适配器可以在同时处理操作的函数模板中使用：

```c++
template <typename BinaryOpTy>
std::pair<Value, Value> zip(BinaryOpTy &&op) {
  return std::make_pair(op.lhs(), op.rhs());;
}

void process(AddOp op, ArrayRef<Value> newOperands) {
  zip(op);
  zip(Adaptor<AddOp>(newOperands));
  /*...*/
}
```

#### 分片操作定义

拥有大量操作的大型方言可能面临生成的操作定义的 C++ 编译时间问题，原因是编译单元过大。`mlir-tblgen` 通过向 `-gen-op-defs` 和 `-gen-op-decls` 传递 `-op-shard-count` 来提供均匀分片操作定义的能力。该工具将生成一个包含按 `GET_OP_DEFS_${N}`（其中 `${N}` 是分片编号）分割的定义的单个包含文件。可以通过在方言库中添加如下文件在单个编译单元中编译一个分片：

```c++
#include "mlir/IR/Operation.h"
// Add any other required includes.

// Utilities shared by generated op definitions: custom directive parsers,
// printers, etc.
#include "OpUtils.h"

#define GET_OP_DEFS_0
#include "MyDialectOps.cpp.inc"
```

注意：这需要重构方言库中的共享工具函数，使其可以被多个编译单元共享。即，不要在同一源文件中定义 `static` 方法，而应在共享头文件中声明它们，并在各自的源文件中定义它们。

操作注册钩子（hook）也是分片的，因为模板实例化编译可能需要很长时间。操作应在方言中如下注册：

```c++
void MyDialect::initialize() {
  registerMyDialectOperations(this);
}
```

CMake 和 Bazel 函数已包含以使方言分片更容易。假设你已将操作工具函数组织到各自的头文件中，定义一个如上所示但不包含 `#define` 的文件：

```c++
// MyDialectOps.cpp
#include "mlir/IR/Operation.h"

#include "OpUtils.h"

#include "MyDialectOps.cpp.inc"
```

在 CMake 中，删除手动的 `mlir_tablegen` 调用并替换为：

```cmake
set(LLVM_TARGET_DEFINITIONS MyDialectOps.td)
add_sharded_ops(MyDialectOps 8) # shard the op definitions by 8

add_mlir_library(MyDialect
  MyDialect.cpp
  MyDialectOpDefs.cpp
  ${SHARDED_SRCS}

  DEPENDS
  MLIRTestOpsShardGen
)
```

这将自动复制 `MyDialectOps.cpp` 源文件并按指定的分片数量添加 `#define`。

建议将所有不在行内的操作成员函数（如验证器）定义在单独的源文件中。在此示例中，它被称为 `MyDialectOpDefs.cpp`。

在 Bazel 中，删除 `-gen-op-defs` 和 `-gen-op-decls` 调用，并添加：

```bazel
gentbl_sharded_ops(
    name = "MyDialectOpSrcs",
    hdr_out = "MyDialectOps.h.inc",
    shard_count = 8,
    sharder = "//mlir:mlir-src-sharder",
    src_file = "MyDialectOps.cpp",
    src_out = "MyDialectOps.cpp.inc",
    tblgen = "//mlir:mlir-tblgen",
    td_file = "MyDialectOps.td",
    deps = [":MyDialectOpsTdFiles"],
)

cc_library(
    name = "MyDialect",
    srcs = glob(["MyDialect/*.cpp"]) + [":MyDialectOpSrcs"]
)
```

## 约束

约束（constraint）是表格驱动操作定义的核心概念：操作验证和图操作匹配都基于满足约束。因此，操作定义和重写规则规范都大量涉及编写约束。我们在 [`OpBase.td`][OpBase] 中有 `Constraint` 类作为所有约束的公共基类。

操作的约束可以覆盖不同范围；它可以：

*   只涉及单个属性（例如，大于 5 的 32 位整数），
*   涉及多个操作数和结果（例如，第 1 个结果的形状必须与第 1 个操作数相同），或
*   对操作本身固有（例如，没有副作用）。

我们分别称它们为单实体约束、多实体约束和特征（trait）。

### 单实体约束

作用于单个操作数、属性或结果的约束在实体的声明处指定，如[操作参数](#操作参数)和[操作结果](#操作结果)中所述。

为了帮助建模常见类型的约束，创建了一组 `TypeConstraint`；它们是 `Type` 子类层次结构。包括用于浮点约束的 `F32`、用于浮点张量约束的 `TensorOf<[F32]>` 等。

类似地，创建了一组 `AttrConstraint` 用于帮助建模常见属性类型的约束。它们是 `Attr` 子类层次结构。包括用于浮点属性约束的 `F32Attr`、用于浮点数组属性约束的 `F32ArrayAttr` 等。

### 多实体约束

涉及多个操作数/属性/结果的约束在操作上很常见，如操作数和结果之间的元素类型和形状关系。这些约束应作为 `Op` 类模板参数指定，如[操作特征与约束](#操作特征与约束)中所述。

多实体约束在 [`OpBase.td`][OpBase] 中被建模为 `PredOpTrait`（`Trait` 的子类）。提供了一系列约束原语以帮助规范。完整列表请参见 [`OpBase.td`][OpBase]。

### 特征（Trait）

特征是操作的固有属性，如是否有副作用、是否可交换、是否是终止符等。这些约束应作为 `Op` 类模板参数指定，如[操作特征与约束](#操作特征与约束)中所述。

特征在 [`OpBase.td`][OpBase] 中被建模为 `NativeTrait`（`Trait` 的子类）。它们有对应的 C++ `mlir::OpTrait` 类支撑并将被翻译为对应的 C++ `mlir::OpTrait` 类。

### 如何指定新约束

要编写约束，你需要提供其谓词（predicate）并给它一个描述性名称。谓词用 `Pred` 类建模，是组合约束的核心工具。约束的谓词通常以嵌套方式构建，使用两类谓词：

1.  `CPred`：原始叶谓词。
2.  复合谓词：使用谓词组合器（合取：`And`，析取：`Or`，否定：`Neg`，替换：`SubstLeaves`，连接：`Concat`）从子谓词组合而成的谓词。

`CPred` 是组合更复杂谓词的基础。从 TableGen 的角度来看，它是"原子"谓词，是 TableGen 和 C++ 之间的"接口"。其内部已经是 C++ 代码，将作为不透明字符串处理，其中有特殊占位符待替换。

你可以在 `CPred` 中放入任何返回布尔值的 C++ 代码，包括求值表达式、调用函数、调用类方法等。

为了帮助与 C++ 环境交互，提供了几个特殊占位符来引用使用此谓词的上下文中的实体。它们充当对封闭环境的"钩子"。包括 `$_builder`、`$_op` 和 `$_self`：

*   `$_builder` 将被一个 `mlir::Builder` 实例替换，以便你可以访问常见的构建方法。
*   `$_op` 将被当前操作替换，以便你可以访问当前操作的信息。
*   `$_self` 将被此谓词所附加的实体替换。例如，`BoolAttr` 是一个包装 `CPred<"isa<BoolAttr>($_self)">` 的属性约束。那么对于 `BoolAttr:$attr`，`$_self` 将被 `$attr` 替换。对于类型约束，有一些特殊情况，因为我们希望每个类型定义上的约束读起来自然，并且希望将类型约束直接附加到操作数/结果上，所以 `$_self` 将被操作数/结果的类型替换。例如，对于 `F32:$operand` 中的 `F32`，其 `$_self` 将展开为 `operand(...).getType()`。

TODO: 重新考虑特殊占位符的前导符号。最终我们希望允许引用操作数/结果的 `$-name`；这些 `$-name` 可以以下划线开头。

例如，要写出属性 `attr` 是 `IntegerAttr`，在 C++ 中你可以直接调用 `isa<IntegerAttr>(attr)`。该代码可以包装在 `CPred` 中为 `isa<IntegerAttr>($_self)`，其中 `$_self` 是在展开时被当前属性 `attr` 替换的特殊占位符。

对于更复杂的谓词，你可以将其包装在单个 `CPred` 中，或使用谓词组合器来组合它们。例如，要写出属性 `attr` 是 32 位或 64 位整数的约束，你可以写为：

```tablegen
And<[
  CPred<"$isa<IntegerAttr>(_self)()">,
  Or<[
    CPred<"cast<IntegerAttr>($_self).getType().isInteger(32)">,
    CPred<"cast<IntegerAttr>($_self).getType().isInteger(64)">
  ]>
]>
```

（注意，以上只是用熟悉的示例展示如何使用 `CPred` 和谓词组合器编写复杂谓词。对于整数属性，[`OpBase.td`][OpBase] 已经定义了 `I32Attr` 和 `I64Attr`。因此你实际上可以重用它们写成 `Or<[I32Attr.predicate, I64Attr.predicate]>`。）

TODO: 构建可重用原始约束库

如果谓词用 `CPred` 和谓词组合器编写非常复杂，你也可以将其写成普通的 C++ 函数，并使用 `CPred` 作为"调用"该函数的方式。例如，要验证属性 `attr` 具有某种属性，你可以写一个 C++ 函数：

```cpp
bool HasSomeProperty(Attribute attr) { ... }
```

然后定义操作为：

```tablegen
def HasSomeProperty : AttrConstraint<CPred<"HasSomeProperty($_self)">,
                                     "has some property">;

def MyOp : Op<...> {
  let arguments = (ins
    ...
    HasSomeProperty:$attr
  );
}
```

至于是应该使用包裹整个表达式的单个 `CPred`、使用谓词组合器的多个 `CPred`，还是"调用"函数的单个 `CPred`，没有明确的标准。使用 `CPred` 和谓词组合器定义更为可取，因为它将更多信息暴露给操作定义规范（而不是将所有逻辑隐藏在 C++ 函数后面），使其有潜力驱动更多自动生成场景。但它需要一个好的公共谓词库作为构建块以避免重复，这方面工作目前正在进行中。

## 属性定义（Attribute Definition）

属性（attribute）是操作的编译时已知常量。

ODS 在 C++ 属性类之上提供属性包装器。MLIR 的核心 IR 库中定义了一些常见的 C++ [属性类][AttrClasses]，用户也可以自由定义方言特定的属性类。ODS 允许在 TableGen 中使用这些属性来定义操作，可能带有更细粒度的约束。例如，`StrAttr` 直接映射到 `StringAttr`；`F32Attr`/`F64Attr` 要求 `FloatAttr` 还具有特定位宽。

ODS 属性定义为具有存储类型（对应于存储属性的底层 `mlir::Attribute`）、返回类型（对应于生成的辅助 getter 的 C++ 返回类型）以及在内部存储和辅助方法之间转换的方法。

### 属性装饰器

有几个重要的属性适配器/装饰器/修饰符可以应用于 ODS 属性，用于指定常见的附加属性，如可选性、默认值等：

*   `DefaultValuedAttr`：指定属性的[默认值](#带默认值的属性)。
*   `OptionalAttr`：将属性指定为[可选的](#可选属性)。
*   `ConfinedAttr`：用[进一步约束](#限制属性)来适配属性。
*   `AllAttrOf`：用[多个约束](#组合约束)来适配属性。

## 枚举定义

MLIR 能够生成 C++ 枚举，使用 `IntEnum` 和 `BitEnum` 类分别生成表示从列表中取值的枚举和可以保存标志组合的枚举。

所有这些 `IntEnum` 和 `BitEnum` 类都需要通过 `EnumCase` 或 `BitEnumCase` 子类完整指定所有允许的情况。这样，ODS 能够生成额外的验证，只接受允许的情况。为了方便 TableGen 枚举与包装它们的属性（attribute）或属性（property）之间的交互，并使其更易于在 C++ 中使用，[`EnumsGen`][EnumsGen] TableGen 后端可以生成一些常用工具：C++ 枚举类、枚举类的 `llvm::DenseMapInfo`、字符串与枚举的相互转换函数。这通过 `mlir-tblgen` 的 `-gen-enum-decls` 和 `-gen-enum-defs` 命令行选项控制。

例如，给定以下 `EnumAttr`：

```tablegen
def Case15: I32EnumCase<"Case15", 15>;
def Case20: I32EnumCase<"Case20", 20>;

def MyIntEnum: I32Enum<"MyIntEnum", "An example int enum",
                           [Case15, Case20]> {
  let cppNamespace = "Outer::Inner";
  let stringToSymbolFnName = "ConvertToEnum";
  let symbolToStringFnName = "ConvertToString";
}
```

以下内容将通过 `mlir-tblgen -gen-enum-decls` 生成：

```c++
namespace Outer {
namespace Inner {
// An example int enum
enum class MyIntEnum : uint32_t {
  Case15 = 15,
  Case20 = 20,
};

std::optional<MyIntEnum> symbolizeMyIntEnum(uint32_t);
llvm::StringRef ConvertToString(MyIntEnum);
std::optional<MyIntEnum> ConvertToEnum(llvm::StringRef);
inline constexpr unsigned getMaxEnumValForMyIntEnum() {
  return 20;
}

} // namespace Inner
} // namespace Outer

namespace llvm {
template<> struct DenseMapInfo<Outer::Inner::MyIntEnum> {
  using StorageInfo = llvm::DenseMapInfo<uint32_t>;

  static inline Outer::Inner::MyIntEnum getEmptyKey() {
    return static_cast<Outer::Inner::MyIntEnum>(StorageInfo::getEmptyKey());
  }

  static inline Outer::Inner::MyIntEnum getTombstoneKey() {
    return static_cast<Outer::Inner::MyIntEnum>(StorageInfo::getTombstoneKey());
  }

  static unsigned getHashValue(const Outer::Inner::MyIntEnum &val) {
    return StorageInfo::getHashValue(static_cast<uint32_t>(val));
  }

  static bool isEqual(const Outer::Inner::MyIntEnum &lhs, const Outer::Inner::MyIntEnum &rhs) {
    return lhs == rhs;
  }
};
}
```

以下内容将通过 `mlir-tblgen -gen-enum-defs` 生成：

```c++
namespace Outer {
namespace Inner {
llvm::StringRef ConvertToString(MyIntEnum val) {
  switch (val) {
    case MyIntEnum::Case15: return "Case15";
    case MyIntEnum::Case20: return "Case20";
  }
  return "";
}

std::optional<MyIntEnum> ConvertToEnum(llvm::StringRef str) {
  return llvm::StringSwitch<std::optional<MyIntEnum>>(str)
      .Case("Case15", MyIntEnum::Case15)
      .Case("Case20", MyIntEnum::Case20)
      .Default(std::nullopt);
}
std::optional<MyIntEnum> symbolizeMyIntEnum(uint32_t value) {
  switch (value) {
  case 15: return MyIntEnum::Case15;
  case 20: return MyIntEnum::Case20;
  default: return std::nullopt;
  }
}

} // namespace Inner
} // namespace Outer
```

类似地，对于以下 `BitEnumAttr` 定义：

```tablegen
def None: I32BitEnumCaseNone<"None">;
def Bit0: I32BitEnumCaseBit<"Bit0", 0, "tagged">;
def Bit1: I32BitEnumCaseBit<"Bit1", 1>;
def Bit2: I32BitEnumCaseBit<"Bit2", 2>;
def Bit3: I32BitEnumCaseBit<"Bit3", 3>;

def MyBitEnum: I32BitEnum<"MyBitEnum", "An example bit enum",
                           [None, Bit0, Bit1, Bit2, Bit3]> {
  // Note: this is the default value, and is listed for illustrative purposes.
  let separator = "|";
}
```

我们可以得到：

```c++
// An example bit enum
enum class MyBitEnum : uint32_t {
  None = 0,
  Bit0 = 1,
  Bit1 = 2,
  Bit2 = 4,
  Bit3 = 8,
};

std::optional<MyBitEnum> symbolizeMyBitEnum(uint32_t);
std::string stringifyMyBitEnum(MyBitEnum);
std::optional<MyBitEnum> symbolizeMyBitEnum(llvm::StringRef);

inline constexpr MyBitEnum operator|(MyBitEnum a, MyBitEnum b) {
  return static_cast<MyBitEnum>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline constexpr MyBitEnum operator&(MyBitEnum a, MyBitEnum b) {
  return static_cast<MyBitEnum>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline constexpr MyBitEnum operator^(MyBitEnum a, MyBitEnum b) {
  return static_cast<MyBitEnum>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}
inline constexpr MyBitEnum &operator|=(MyBitEnum &a, MyBitEnum b) {
  return a = a | b;
}
inline constexpr MyBitEnum &operator&=(MyBitEnum &a, MyBitEnum b) {
  return a = a & b;
}
inline constexpr MyBitEnum &operator^=(MyBitEnum &a, MyBitEnum b) {
  return a = a ^ b;
}
inline constexpr MyBitEnum operator~(MyBitEnum bits) {
  // Ensure only bits that can be present in the enum are set
  return static_cast<MyBitEnum>(~static_cast<uint32_t>(bits) & static_cast<uint32_t>(15u));
}
inline constexpr bool bitEnumContainsAll(MyBitEnum bits, MyBitEnum bit) {
  return (bits & bit) == bit;
}
inline constexpr bool bitEnumContainsAny(MyBitEnum bits, MyBitEnum bit) {
  return (static_cast<uint32_t>(bits) & static_cast<uint32_t>(bit)) != 0;
}
inline constexpr MyBitEnum bitEnumClear(MyBitEnum bits, MyBitEnum bit) {
  return bits & ~bit;
}

inline std::string stringifyEnum(MyBitEnum enumValue) {
  return stringifyMyBitEnum(enumValue);
}

template <typename EnumType>
::std::optional<EnumType> symbolizeEnum(::llvm::StringRef);

template <>
inline ::std::optional<MyBitEnum> symbolizeEnum<MyBitEnum>(::llvm::StringRef str) {
  return symbolizeMyBitEnum(str);
}

namespace llvm {
template<> struct DenseMapInfo<::MyBitEnum> {
  using StorageInfo = llvm::DenseMapInfo<uint32_t>;

  static inline ::MyBitEnum getEmptyKey() {
    return static_cast<::MyBitEnum>(StorageInfo::getEmptyKey());
  }

  static inline ::MyBitEnum getTombstoneKey() {
    return static_cast<::MyBitEnum>(StorageInfo::getTombstoneKey());
  }

  static unsigned getHashValue(const ::MyBitEnum &val) {
    return StorageInfo::getHashValue(static_cast<uint32_t>(val));
  }

  static bool isEqual(const ::MyBitEnum &lhs, const ::MyBitEnum &rhs) {
    return lhs == rhs;
  }
};
```

```c++
std::string stringifyMyBitEnum(MyBitEnum symbol) {
  auto val = static_cast<uint32_t>(symbol);
  assert(15u == (15u | val) && "invalid bits set in bit enum");
  // Special case for all bits unset.
  if (val == 0) return "None";
  llvm::SmallVector<llvm::StringRef, 2> strs;
  if (1u == (1u & val)) { strs.push_back("tagged"); }
  if (2u == (2u & val)) { strs.push_back("Bit1"); }
  if (4u == (4u & val)) { strs.push_back("Bit2"); }
  if (8u == (8u & val)) { strs.push_back("Bit3"); }

  return llvm::join(strs, "|");
}

std::optional<MyBitEnum> symbolizeMyBitEnum(llvm::StringRef str) {
  // Special case for all bits unset.
  if (str == "None") return MyBitEnum::None;

  llvm::SmallVector<llvm::StringRef, 2> symbols;
  str.split(symbols, "|");

  uint32_t val = 0;
  for (auto symbol : symbols) {
    auto bit = llvm::StringSwitch<std::optional<uint32_t>>(symbol)
      .Case("tagged", 1)
      .Case("Bit1", 2)
      .Case("Bit2", 4)
      .Case("Bit3", 8)
      .Default(std::nullopt);
    if (bit) { val |= *bit; } else { return std::nullopt; }
  }
  return static_cast<MyBitEnum>(val);
}

std::optional<MyBitEnum> symbolizeMyBitEnum(uint32_t value) {
  // Special case for all bits unset.
  if (value == 0) return MyBitEnum::None;

  if (value & ~static_cast<uint32_t>(15u)) return std::nullopt;
  return static_cast<MyBitEnum>(value);
}
```

### 将枚举包装在属性中

有几种机制可以创建值来自 `*Enum` 的 `Attribute`。

最常见的是使用 `EnumAttr` 类，它接受一个 `EnumInfo`（`IntEnum` 或 `BitEnum`）作为参数，并构建一个保存枚举值的属性。此属性在方言中定义，其汇编格式可以自定义，例如在枚举值周围打印尖括号或分配助记符。

一种较旧的形式涉及使用 `*IntEnumAttr` 和 `*BitEnumAttr` 类及其对应的 `*EnumAttrCase` 类（可以在需要 `*EnumCase` 的任何地方使用）。这些类将其值存储为其位宽的 `SignlessIntegerAttr`，并对其施加约束，使其具有枚举有效范围内的值。如果设置了其 `genSpecializedAttr` 参数，它们还将生成包装属性，而不是使用裸的无符号整数属性进行存储。

### 枚举属性（property）

枚举可以包装在属性（property）中，使其可以内联存储。这使枚举 C++ 类的值成为操作属性（property）结构体的成员，并使操作的验证器检查枚举的值是否为枚举的有效值。

基本包装器是 `EnumProp`，它简单地接受一个 `EnumInfo`。

使用 `NamedEnumProp` 生成的语法更清晰，即在枚举周围加上助记符和 `<>`，它接受一个 `*EnumInfo` 和一个助记符字符串，该字符串成为属性（property）语法的一部分。

这两种 `EnumProp` 类型都有对应的 `*EnumPropWithAttrForm`，允许从 `EnumAttr` 透明升级，并可选择在通用格式中保留这些属性。

## 调试技巧

### 运行 `mlir-tblgen` 查看生成内容

TableGen 语法有时可能晦涩难懂；阅读生成的内容是理解和调试问题的非常有效的方式。要构建 `mlir-tblgen`，在构建目录中运行 `cmake --build . --target mlir-tblgen`，并在 `bin/` 子目录中找到 `mlir-tblgen` 二进制文件。所有支持的生成器可通过 `mlir-tblgen --help` 找到。例如，[生成的 C++ 代码](#生成的-c-代码)中介绍的 `--gen-op-decls` 和 `--gen-op-defs`。

要查看生成的代码，使用特定生成器调用 `mlir-tblgen`，并通过 `-I` 提供包含路径。例如：

```sh
# To see op C++ class declaration
mlir-tblgen --gen-op-decls -I /path/to/mlir/include /path/to/input/td/file
# To see op C++ class definition
mlir-tblgen --gen-op-defs -I /path/to/mlir/include /path/to/input/td/file
# To see op documentation
mlir-tblgen --gen-dialect-doc -I /path/to/mlir/include /path/to/input/td/file

# To see op interface C++ class declaration
mlir-tblgen --gen-op-interface-decls -I /path/to/mlir/include /path/to/input/td/file
# To see op interface C++ class definition
mlir-tblgen --gen-op-interface-defs -I /path/to/mlir/include /path/to/input/td/file
# To see op interface documentation
mlir-tblgen --gen-op-interface-doc -I /path/to/mlir/include /path/to/input/td/file
```

## 附录

### 在 TableGen 中报告弃用

类/def 可以使用 `Deprecate` 辅助类标记为已弃用，例如：

```tablegen
def OpTraitA : NativeOpTrait<"OpTraitA">, Deprecated<"use `bar` instead">;
```

这将使 `OpTraitA` 标记为已弃用，mlir-tblgen 可以发出警告（默认）或错误（取决于 `-on-deprecated` 标志）以使弃用状态为人所知。

### 在 C++ 中报告弃用

TableGen 生成的 C++ 实体，如类、函数或方法，可以使用 `CppDeprecated` 混入（mixin）标记为已弃用：

```tablegen
def MyOp : Op<MyDialect, "my.op">, CppDeprecated<"use 'your.op' instead">;
```

这与 TableGen 的弃用机制不同，mlir-tblgen 不会发出任何警告。相反，在使用给定实体时，C++ 编译器会发出带有给定原因的警告。

为了允许更方便的语法，存在一些 TableGen 类的辅助类，它们通常用作匿名定义。目前包括：

* `DeprecatedOpBuilder`：可以代替 `OpBuilder` 使用，参数相同，但第一个参数是原因字符串，例如 `DeprecatedOpBuilder<"use 'build' with foo instead", (ins "int":$bar)>`

注意：`CppDeprecated` 机制的支持必须由每个代码生成器单独实现。

### 需求与现有机制分析

操作描述应尽可能声明式，以允许广泛的工具使用它们并查询从它们生成的方法。这特别意味着以易于分析的方式指定特征、约束和形状推断信息（例如，尽可能避免对 C++ 函数的不透明调用）。

我们考察了几个当代系统的方法，并专注于所需的要求：

*   使用与 C++ 代码分离的注册表注册操作。
    *   MLIR 中允许未知操作，因此操作不必注册。编译器优化这些操作或包含这些操作的图的能力受到约束，但仍是正确的。
    *   当前提案不包括运行时操作描述，但不排除此类描述，可以在以后添加。
    *   操作注册表对于生成使操作的操作、验证正确构建等在 C++ 中更容易的 C++ 类（通过提供类型化表示和访问器）至关重要。
*   操作注册表将在 [TableGen](https://llvm.org/docs/TableGen/index.html) 中定义，并用于生成 C++ 类和工具函数（构建器/验证器/解析器/打印器）。
    *   TableGen 是 LLVM 后端使用的建模规范语言，非常适合基于特征的建模。这是一个实现决策，还有其他替代方式。但该规范语言非常适合建模特征的需求（从在 LLVM 处理器后端建模中的使用可以看出），且易于扩展，因此是一个实际的选择。如果出现其他好的选择，我们会考虑。
*   MLIR 同时允许已定义和未定义的操作。
    *   已定义的操作应具有固定的语义，可以有对应的参考实现定义。
    *   方言完全由方言所有者控制，通常与方言的框架一起存在。
*   操作的特征（如可交换性）与操作一起在注册表中建模。
*   操作的操作数/返回类型约束与操作一起在注册表中建模（见下面的[形状推断](../ShapeInference.md)讨论），这允许（例如）在文本转储中优化简洁的语法。
*   操作的行为与操作一起记录，包括摘要和描述。描述以 Markdown 编写，并提取以包含在生成的方言 LangRef 部分中。
*   打印和解析的通用汇编格式正常可用，但也可以指定自定义解析器和打印器，或从显示"汇编"字符串到操作数/类型映射的可选字符串表示中自动生成。
    *   解析器级重映射（如 `eq` 到枚举）将作为解析器生成的一部分支持。
*   匹配模式与操作描述分开指定。
    *   与 LLVM 相比，没有每个后端都需要了解的"基础"操作集。相反，有许多不同的方言，这些方言之间的转换/合法化形成了一个转换图。
*   参考实现可以与操作定义一起提供。

    *   参考实现可以用标准操作或其他参考实现来表达。

    TODO: 如果依赖操作的定义发生变化，记录预期行为。

[TableGen]: https://llvm.org/docs/TableGen/index.html
[TableGenProgRef]: https://llvm.org/docs/TableGen/ProgRef.html
[TableGenBackend]: https://llvm.org/docs/TableGen/BackEnds.html#introduction
[OpBase]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/IR/OpBase.td
[OpDefinitionsGen]: https://github.com/llvm/llvm-project/blob/main/mlir/tools/mlir-tblgen/OpDefinitionsGen.cpp
[EnumsGen]: https://github.com/llvm/llvm-project/blob/main/mlir/tools/mlir-tblgen/EnumsGen.cpp
[StringAttr]: ../Dialects/Builtin.md/#stringattr
[IntegerAttr]: ../Dialects/Builtin.md/#integertype
[AttrClasses]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/IR/Attributes.h
