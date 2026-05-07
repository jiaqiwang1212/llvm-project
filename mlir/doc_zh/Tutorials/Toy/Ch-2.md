# 第 2 章：生成基础 MLIR

[TOC]

现在我们已经熟悉了我们的语言和 AST，让我们看看 MLIR 如何帮助编译 Toy。

## 简介：多层次中间表示

其他编译器，如 LLVM（参见 [Kaleidoscope 教程](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)），提供了一组固定的预定义类型和（通常是*低层次* / RISC 风格的）指令。对于给定语言的前端，在生成 LLVM IR 之前，需要执行任何语言特定的类型检查、分析或转换。例如，Clang 将使用其 AST 不仅执行静态分析，还执行转换，例如通过 AST 克隆和重写来进行 C++ 模板实例化。最后，构造层次高于 C/C++ 的语言可能需要从其 AST 进行非平凡的降级才能生成 LLVM IR。

因此，多个前端最终都会重新实现大量基础设施，以支持这些分析和转换的需求。MLIR 通过被设计为可扩展来解决这个问题。因此，几乎没有预定义的指令（MLIR 术语中的*操作*）或类型。

## 与 MLIR 交互

[语言参考](../../LangRef.md)

MLIR 被设计为完全可扩展的基础设施；没有封闭的属性集合（可以理解为常量元数据）、操作或类型。MLIR 通过[方言（Dialects）](../../LangRef.md/#dialects)的概念来支持这种可扩展性。方言在唯一的 `namespace` 下为抽象提供分组机制。

在 MLIR 中，[`Operations`](../../LangRef.md/#operations) 是抽象和计算的核心单元，在许多方面类似于 LLVM 指令。操作可以具有特定于应用程序的语义，并可用于表示 LLVM 中所有核心 IR 结构：指令、全局变量（如函数）、模块等。

以下是 Toy `transpose` 操作的 MLIR 汇编表示：

```mlir
%t_tensor = "toy.transpose"(%tensor) {inplace = true} : (tensor<2x3xf64>) -> tensor<3x2xf64> loc("example/file/path":12:1)
```

让我们分解这个 MLIR 操作的结构：

-   `%t_tensor`

    *   该操作定义的结果的名称（其中包括[带前缀符号以避免冲突](../../LangRef.md/#identifiers-and-keywords)）。一个操作可以定义零个或多个结果（在 Toy 上下文中，我们将自限为单结果操作），这些结果是 SSA 值。该名称在解析时使用，但不是持久的（例如，它不会在 SSA 值的内存表示中被跟踪）。

-   `"toy.transpose"`

    *   操作的名称。它应是唯一的字符串，方言的命名空间前缀在 "`.`" 之前。可以将其理解为 `toy` 方言中的 `transpose` 操作。

-   `(%tensor)`

    *   零个或多个输入操作数（或参数）的列表，这些是由其他操作定义的或引用块参数的 SSA 值。

-   `{ inplace = true }`

    *   零个或多个属性的字典，这些是始终为常量的特殊操作数。这里我们定义了一个名为 'inplace' 的布尔属性，其常量值为 true。

-   `(tensor<2x3xf64>) -> tensor<3x2xf64>`

    *   这以函数形式引用操作的类型，括号中写出参数类型，后面是返回值的类型。

-   `loc("example/file/path":12:1)`

    *   这是该操作在源代码中的位置。

以上展示的是操作的通用形式。如前所述，MLIR 中的操作集合是可扩展的。操作使用少量概念来建模，使得操作可以被通用地推理和操作。这些概念包括：

-   操作的名称。
-   SSA 操作数值的列表。
-   [属性](../../LangRef.md/#attributes)的列表。
-   结果值的[类型](../../LangRef.md/#type-system)列表。
-   用于调试目的的[源位置](../../Diagnostics.md/#source-locations)。
-   后继[块](../../LangRef.md/#blocks)的列表（主要用于分支）。
-   [区域](../../LangRef.md/#regions)的列表（用于函数等结构性操作）。

在 MLIR 中，每个操作都有一个强制关联的源位置。与 LLVM 不同，在 LLVM 中调试信息位置是元数据且可以被丢弃，而在 MLIR 中，位置是核心需求，API 依赖并操作它。因此，丢弃位置是一种显式选择，不会因失误而发生。

举例说明：如果某个转换将一个操作替换为另一个，那么新操作仍然必须附加一个位置。这使得追踪该操作的来源成为可能。

值得注意的是，mlir-opt 工具——一个用于测试编译器 pass 的工具——默认不在输出中包含位置信息。`-mlir-print-debuginfo` 标志指定包含位置信息。（运行 `mlir-opt --help` 以获取更多选项。）

### 不透明 API

MLIR 被设计为允许所有 IR 元素（如属性、操作和类型）都可以自定义。同时，IR 元素始终可以归结为上述基本概念。这使得 MLIR 能够对*任何*操作进行解析、表示和[往返（round-trip）](../../../getting_started/Glossary.md/#round-trip) IR。例如，我们可以将上面的 Toy 操作放入一个 `.mlir` 文件中，并通过 *mlir-opt* 往返而无需注册任何 `toy` 相关的方言：

```mlir
func.func @toy_func(%tensor: tensor<2x3xf64>) -> tensor<3x2xf64> {
  %t_tensor = "toy.transpose"(%tensor) { inplace = true } : (tensor<2x3xf64>) -> tensor<3x2xf64>
  return %t_tensor : tensor<3x2xf64>
}
```

对于未注册的属性、操作和类型，MLIR 会强制执行一些结构约束（例如支配关系等），但除此之外它们是完全不透明的。例如，MLIR 几乎没有关于未注册操作能否在特定数据类型上运行、可以接受多少操作数或产生多少结果的信息。这种灵活性对于自举目的很有用，但通常不建议在成熟系统中使用。未注册的操作在转换和分析中必须保守地处理，而且它们更难以构造和操作。

这种处理方式可以通过构造一个对 Toy 来说应该是无效的 IR，并观察它在不触发验证器的情况下往返来验证：

```mlir
func.func @main() {
  %0 = "toy.print"() : () -> tensor<2x3xf64>
}
```

这里存在多个问题：`toy.print` 操作不是终结符；它应该接受一个操作数；并且它不应该返回任何值。在下一节中，我们将把方言和操作注册到 MLIR，插入验证器，并添加更友好的 API 来操作我们的操作。

## 定义 Toy 方言

为了有效地与 MLIR 交互，我们将定义一个新的 Toy 方言。该方言将对 Toy 语言的结构进行建模，并提供高层次分析和转换的便捷途径。

```c++
/// This is the definition of the Toy dialect. A dialect inherits from
/// mlir::Dialect and registers custom attributes, operations, and types. It can
/// also override virtual methods to change some general behavior, which will be
/// demonstrated in later chapters of the tutorial.
class ToyDialect : public mlir::Dialect {
public:
  explicit ToyDialect(mlir::MLIRContext *ctx);

  /// Provide a utility accessor to the dialect namespace.
  static llvm::StringRef getDialectNamespace() { return "toy"; }

  /// An initializer called from the constructor of ToyDialect that is used to
  /// register attributes, operations, types, and more within the Toy dialect.
  void initialize();
};
```

这是一个方言的 C++ 定义，但 MLIR 也支持通过 [tablegen](https://llvm.org/docs/TableGen/ProgRef.html) 声明式地定义方言。使用声明式规范更为简洁，因为它消除了定义新方言时大量样板代码的需要。它还能轻松生成方言文档，可以直接与方言一起描述。在这种声明式格式中，toy 方言将被指定为：

```tablegen
// Provide a definition of the 'toy' dialect in the ODS framework so that we
// can define our operations.
def Toy_Dialect : Dialect {
  // The namespace of our dialect, this corresponds 1-1 with the string we
  // provided in `ToyDialect::getDialectNamespace`.
  let name = "toy";

  // A short one-line summary of our dialect.
  let summary = "A high-level dialect for analyzing and optimizing the "
                "Toy language";

  // A much longer description of our dialect.
  let description = [{
    The Toy language is a tensor-based language that allows you to define
    functions, perform some math computation, and print results. This dialect
    provides a representation of the language that is amenable to analysis and
    optimization.
  }];

  // The C++ namespace that the dialect class definition resides in.
  let cppNamespace = "toy";
}
```

要查看生成的内容，可以使用 `gen-dialect-decls` 动作运行 `mlir-tblgen` 命令，如下所示：

```shell
${build_root}/bin/mlir-tblgen -gen-dialect-decls ${mlir_src_root}/examples/toy/Ch2/include/toy/Ops.td -I ${mlir_src_root}/include/
```

方言定义完成后，现在可以将其加载到 MLIRContext 中：

```c++
  context.loadDialect<ToyDialect>();
```

默认情况下，`MLIRContext` 只加载[内置方言（Builtin Dialect）](../../Dialects/Builtin.md)，该方言提供了一些核心 IR 组件，这意味着其他方言（例如我们的 `Toy` 方言）必须显式加载。

## 定义 Toy 操作

现在我们有了 `Toy` 方言，可以开始定义操作了。这将允许提供系统其余部分可以接入的语义信息。以 `toy.constant` 操作的创建为例，该操作将在 Toy 语言中表示一个常量值。

```mlir
 %4 = "toy.constant"() {value = dense<1.0> : tensor<2x3xf64>} : () -> tensor<2x3xf64>
```

该操作接受零个操作数，一个名为 `value` 的[密集元素（dense elements）](../../Dialects/Builtin.md/#densetypedelementsattr)属性来表示常量值，并返回 [RankedTensorType](../../Dialects/Builtin.md/#rankedtensortype) 的单个结果。操作类继承自 [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) `mlir::Op` 类，该类还可以接受一些可选的[*特征（traits）*](../../Traits)来自定义其行为。`Traits` 是一种可以向操作注入额外行为的机制，例如额外的访问器、验证等。让我们看看下面对上述常量操作的一种可能定义：

```c++
class ConstantOp : public mlir::Op<
                     /// `mlir::Op` is a CRTP class, meaning that we provide the
                     /// derived class as a template parameter.
                     ConstantOp,
                     /// The ConstantOp takes zero input operands.
                     mlir::OpTrait::ZeroOperands,
                     /// The ConstantOp returns a single result.
                     mlir::OpTrait::OneResult,
                     /// We also provide a utility `getType` accessor that
                     /// returns the TensorType of the single result.
                     mlir::OpTrait::OneTypedResult<TensorType>::Impl> {

 public:
  /// Inherit the constructors from the base Op class.
  using Op::Op;

  /// Provide the unique name for this operation. MLIR will use this to register
  /// the operation and uniquely identify it throughout the system. The name
  /// provided here must be prefixed by the parent dialect namespace followed
  /// by a `.`.
  static llvm::StringRef getOperationName() { return "toy.constant"; }

  /// Return the value of the constant by fetching it from the attribute.
  mlir::DenseElementsAttr getValue();

  /// Operations may provide additional verification beyond what the attached
  /// traits provide.  Here we will ensure that the specific invariants of the
  /// constant operation are upheld, for example the result type must be
  /// of TensorType and matches the type of the constant `value`.
  LogicalResult verifyInvariants();

  /// Provide an interface to build this operation from a set of input values.
  /// This interface is used by the `builder` classes to allow for easily
  /// generating instances of this operation:
  ///   mlir::OpBuilder::create<ConstantOp>(...)
  /// This method populates the given `state` that MLIR uses to create
  /// operations. This state is a collection of all of the discrete elements
  /// that an operation may contain.
  /// Build a constant with the given return type and `value` attribute.
  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::Type result, mlir::DenseElementsAttr value);
  /// Build a constant and reuse the type from the given 'value'.
  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    mlir::DenseElementsAttr value);
  /// Build a constant by broadcasting the given 'value'.
  static void build(mlir::OpBuilder &builder, mlir::OperationState &state,
                    double value);
};
```

然后我们可以在 `ToyDialect` 初始化器中注册这个操作：

```c++
void ToyDialect::initialize() {
  addOperations<ConstantOp>();
}
```

### Op 与 Operation：使用 MLIR 操作

现在我们已经定义了一个操作，我们将要访问和转换它。在 MLIR 中，有两个与操作相关的主要类：`Operation` 和 `Op`。`Operation` 类用于通用地对所有操作建模。它是"不透明的"，即它不描述特定操作或操作类型的属性。相反，`Operation` 类提供了一个进入操作实例的通用 API。另一方面，每种特定类型的操作都由一个 `Op` 派生类来表示。例如，`ConstantOp` 表示一个具有零输入和一个输出的操作，其输出始终被设置为相同的值。`Op` 派生类充当围绕 `Operation*` 的智能指针包装器，提供操作特定的访问器方法和类型安全的操作属性。这意味着当我们定义 Toy 操作时，我们只是在为构建和与 `Operation` 类交互定义一个清晰的、语义上有用的接口。这就是为什么我们的 `ConstantOp` 没有定义类字段；该操作的所有数据都存储在引用的 `Operation` 中。这种设计的一个副作用是我们总是"按值"传递 `Op` 派生类，而不是通过引用或指针（*按值传递*是 MLIR 中的常见惯用语，同样适用于属性、类型等）。给定一个通用的 `Operation*` 实例，我们总是可以使用 LLVM 的类型转换基础设施获取特定的 `Op` 实例：

```c++
void processConstantOp(mlir::Operation *operation) {
  ConstantOp op = llvm::dyn_cast<ConstantOp>(operation);

  // This operation is not an instance of `ConstantOp`.
  if (!op)
    return;

  // Get the internal operation instance wrapped by the smart pointer.
  mlir::Operation *internalOperation = op.getOperation();
  assert(internalOperation == operation &&
         "these operation instances are the same");
}
```

### 使用操作定义规范（ODS）框架

除了特化 `mlir::Op` C++ 模板之外，MLIR 还支持以声明式方式定义操作。这通过[操作定义规范](../../DefiningDialects/Operations.md)框架来实现。关于操作的事实被简洁地指定到一个 TableGen 记录中，该记录将在编译时被扩展为等效的 `mlir::Op` C++ 模板特化。使用 ODS 框架是在 MLIR 中定义操作的理想方式，具有简单、简洁以及在面对 C++ API 变化时的总体稳定性等优点。

让我们看看如何定义与我们的 ConstantOp 等效的 ODS 版本：

ODS 中的操作通过继承 `Op` 类来定义。为了简化我们的操作定义，我们将为 Toy 方言中的操作定义一个基类。

```tablegen
// Base class for toy dialect operations. This operation inherits from the base
// `Op` class in OpBase.td, and provides:
//   * The parent dialect of the operation.
//   * The mnemonic for the operation, or the name without the dialect prefix.
//   * A list of traits for the operation.
class Toy_Op<string mnemonic, list<Trait> traits = []> :
    Op<Toy_Dialect, mnemonic, traits>;
```

定义好所有基础部分后，我们可以开始定义常量操作。

我们通过继承上面的基类 'Toy_Op' 来定义一个 toy 操作。在这里，我们为操作提供助记符和特征列表。这里的[助记符](../../DefiningDialects/Operations.md/#operation-name)与 `ConstantOp::getOperationName` 中给出的不含方言前缀 `toy.` 的部分相匹配。与我们的 C++ 定义相比，这里缺少 `ZeroOperands` 和 `OneResult` 特征；这些将根据我们稍后定义的 `arguments` 和 `results` 字段自动推断。

```tablegen
def ConstantOp : Toy_Op<"constant"> {
}
```

此时您可能想知道 TableGen 生成的 C++ 代码是什么样的。只需使用 `gen-op-decls` 或 `gen-op-defs` 动作运行 `mlir-tblgen` 命令即可：

```shell
${build_root}/bin/mlir-tblgen -gen-op-defs ${mlir_src_root}/examples/toy/Ch2/include/toy/Ops.td -I ${mlir_src_root}/include/
```

根据所选动作，这将打印 `ConstantOp` 类的声明或其实现。将此输出与手工编写的实现进行比较，在开始使用 TableGen 时非常有帮助。

#### 定义参数和结果

定义好操作的框架后，我们现在可以为操作提供[输入](../../DefiningDialects/Operations.md/#operation-arguments)和[输出](../../DefiningDialects/Operations.md/#operation-results)。操作的输入或参数可以是属性，也可以是 SSA 操作数值的类型。结果对应于操作产生的值的一组类型：

```tablegen
def ConstantOp : Toy_Op<"constant"> {
  // The constant operation takes an attribute as the only input.
  // `F64ElementsAttr` corresponds to a 64-bit floating-point ElementsAttr.
  let arguments = (ins F64ElementsAttr:$value);

  // The constant operation returns a single value of TensorType.
  // F64Tensor corresponds to a 64-bit floating-point TensorType.
  let results = (outs F64Tensor);
}
```

通过为参数或结果提供名称（例如 `$value`），ODS 将自动生成匹配的访问器：`DenseElementsAttr ConstantOp::value()`。

#### 添加文档

定义操作后的下一步是为其添加文档。操作可以提供 [`summary` 和 `description`](../../DefiningDialects/Operations.md/#operation-documentation) 字段来描述操作的语义。这些信息对方言用户很有用，甚至可以用来自动生成 Markdown 文档。

```tablegen
def ConstantOp : Toy_Op<"constant"> {
  // Provide a summary and description for this operation. This can be used to
  // auto-generate documentation of the operations within our dialect.
  let summary = "constant operation";
  let description = [{
    Constant operation turns a literal into an SSA value. The data is attached
    to the operation as an attribute. For example:

      %0 = "toy.constant"()
         { value = dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]> : tensor<2x3xf64> }
        : () -> tensor<2x3xf64>
  }];

  // The constant operation takes an attribute as the only input.
  // `F64ElementsAttr` corresponds to a 64-bit floating-point ElementsAttr.
  let arguments = (ins F64ElementsAttr:$value);

  // The generic call operation returns a single value of TensorType.
  // F64Tensor corresponds to a 64-bit floating-point TensorType.
  let results = (outs F64Tensor);
}
```

#### 验证操作语义

到目前为止，我们已经涵盖了原始 C++ 操作定义的大部分内容。下一个要定义的部分是验证器。幸运的是，与命名访问器类似，ODS 框架将根据我们给出的约束自动生成大量必要的验证逻辑。这意味着我们不需要验证返回类型的结构，甚至不需要验证输入属性 `value`。在许多情况下，ODS 操作甚至不需要额外的验证。要添加额外的验证逻辑，操作可以覆盖 [`verifier`](../../DefiningDialects/Operations.md/#custom-verifier-code) 字段。`verifier` 字段允许定义一个 C++ 代码块，该块将作为 `ConstantOp::verify` 的一部分运行。该代码块可以假设操作的所有其他不变量已经被验证：

```tablegen
def ConstantOp : Toy_Op<"constant"> {
  // Provide a summary and description for this operation. This can be used to
  // auto-generate documentation of the operations within our dialect.
  let summary = "constant operation";
  let description = [{
    Constant operation turns a literal into an SSA value. The data is attached
    to the operation as an attribute. For example:

      %0 = "toy.constant"()
         { value = dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]> : tensor<2x3xf64> }
        : () -> tensor<2x3xf64>
  }];

  // The constant operation takes an attribute as the only input.
  // `F64ElementsAttr` corresponds to a 64-bit floating-point ElementsAttr.
  let arguments = (ins F64ElementsAttr:$value);

  // The generic call operation returns a single value of TensorType.
  // F64Tensor corresponds to a 64-bit floating-point TensorType.
  let results = (outs F64Tensor);

  // Add additional verification logic to the constant operation. Setting this bit
  // to `1` will generate a `::llvm::LogicalResult verify()` declaration on the
  // operation class that is called after ODS constructs have been verified, for
  // example the types of arguments and results. We implement additional verification
  // in the definition of this `verify` method in the C++ source file.
  let hasVerifier = 1;
}
```

#### 附加 `build` 方法

原始 C++ 示例中最后缺少的组件是 `build` 方法。ODS 可以自动生成一些简单的 build 方法，在本例中它将为我们生成第一个 build 方法。对于其余的方法，我们定义 [`builders`](../../DefiningDialects/Operations.md/#custom-builder-methods) 字段。该字段接受 `OpBuilder` 对象的列表，这些对象接受一个对应于 C++ 参数列表的字符串，以及一个可选的代码块，可用于内联指定实现。

```tablegen
def ConstantOp : Toy_Op<"constant"> {
  ...

  // Add custom build methods for the constant operation. These methods populate
  // the `state` that MLIR uses to create operations, i.e. these are used when
  // using `ConstantOp::create(builder, ...)`.
  let builders = [
    // Build a constant with a given constant tensor value.
    OpBuilder<(ins "DenseElementsAttr":$value), [{
      // Call into an autogenerated `build` method.
      build(builder, result, value.getType(), value);
    }]>,

    // Build a constant with a given constant floating-point value. This builder
    // creates a declaration for `ConstantOp::build` with the given parameters.
    OpBuilder<(ins "double":$value)>
  ];
}
```

#### 指定自定义汇编格式

到目前为止，我们可以生成我们的"Toy IR"。例如，以下代码：

```toy
# User defined generic function that operates on unknown shaped arguments.
def multiply_transpose(a, b) {
  return transpose(a) * transpose(b);
}

def main() {
  var a<2, 3> = [[1, 2, 3], [4, 5, 6]];
  var b<2, 3> = [1, 2, 3, 4, 5, 6];
  var c = multiply_transpose(a, b);
  var d = multiply_transpose(b, a);
  print(d);
}
```

生成以下 IR：

```mlir
module {
  "toy.func"() ({
  ^bb0(%arg0: tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":4:1), %arg1: tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":4:1)):
    %0 = "toy.transpose"(%arg0) : (tensor<*xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:10)
    %1 = "toy.transpose"(%arg1) : (tensor<*xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:25)
    %2 = "toy.mul"(%0, %1) : (tensor<*xf64>, tensor<*xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:25)
    "toy.return"(%2) : (tensor<*xf64>) -> () loc("test/Examples/Toy/Ch2/codegen.toy":5:3)
  }) {sym_name = "multiply_transpose", type = (tensor<*xf64>, tensor<*xf64>) -> tensor<*xf64>} : () -> () loc("test/Examples/Toy/Ch2/codegen.toy":4:1)
  "toy.func"() ({
    %0 = "toy.constant"() {value = dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>} : () -> tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":9:17)
    %1 = "toy.reshape"(%0) : (tensor<2x3xf64>) -> tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":9:3)
    %2 = "toy.constant"() {value = dense<[1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00]> : tensor<6xf64>} : () -> tensor<6xf64> loc("test/Examples/Toy/Ch2/codegen.toy":10:17)
    %3 = "toy.reshape"(%2) : (tensor<6xf64>) -> tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":10:3)
    %4 = "toy.generic_call"(%1, %3) {callee = @multiply_transpose} : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":11:11)
    %5 = "toy.generic_call"(%3, %1) {callee = @multiply_transpose} : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":12:11)
    "toy.print"(%5) : (tensor<*xf64>) -> () loc("test/Examples/Toy/Ch2/codegen.toy":13:3)
    "toy.return"() : () -> () loc("test/Examples/Toy/Ch2/codegen.toy":8:1)
  }) {sym_name = "main", type = () -> ()} : () -> () loc("test/Examples/Toy/Ch2/codegen.toy":8:1)
} loc(unknown)
```

这里需要注意的是，所有 Toy 操作都使用通用汇编格式打印。这种格式就是本章开头分解 `toy.transpose` 时展示的格式。MLIR 允许操作定义自己的自定义汇编格式，可以[声明式](../../DefiningDialects/Operations.md/#declarative-assembly-format)地或通过 C++ 命令式地实现。定义自定义汇编格式可以通过去除通用格式所需的许多冗余内容，将生成的 IR 变得更易读。让我们来看一个我们希望简化的操作格式的例子。

##### `toy.print`

`toy.print` 的当前形式有些冗长。我们希望去掉许多额外的字符。让我们先考虑 `toy.print` 的一个好格式是什么，然后看看如何实现它。查看 `toy.print` 的基本结构，我们得到：

```mlir
toy.print %5 : tensor<*xf64> loc(...)
```

在这里，我们将格式精简到了最基本的内容，它变得更易读了。为了提供自定义汇编格式，操作可以覆盖 `hasCustomAssemblyFormat` 字段用于 C++ 格式，或者覆盖 `assemblyFormat` 字段用于声明式格式。让我们先看看 C++ 变体，因为声明式格式在内部也是映射到它的。

```tablegen
/// Consider a stripped definition of `toy.print` here.
def PrintOp : Toy_Op<"print"> {
  let arguments = (ins F64Tensor:$input);

  // Divert the printer and parser to `parse` and `print` methods on our operation,
  // to be implemented in the .cpp file. More details on these methods is shown below.
  let hasCustomAssemblyFormat = 1;
}
```

打印器和解析器的 C++ 实现如下所示：

```c++
/// The 'OpAsmPrinter' class is a stream that will allows for formatting
/// strings, attributes, operands, types, etc.
void PrintOp::print(mlir::OpAsmPrinter &printer) {
  printer << "toy.print " << op.input();
  printer.printOptionalAttrDict(op.getAttrs());
  printer << " : " << op.input().getType();
}

/// The 'OpAsmParser' class provides a collection of methods for parsing
/// various punctuation, as well as attributes, operands, types, etc. Each of
/// these methods returns a `ParseResult`. This class is a wrapper around
/// `LogicalResult` that can be converted to a boolean `true` value on failure,
/// or `false` on success. This allows for easily chaining together a set of
/// parser rules. These rules are used to populate an `mlir::OperationState`
/// similarly to the `build` methods described above.
mlir::ParseResult PrintOp::parse(mlir::OpAsmParser &parser,
                                 mlir::OperationState &result) {
  // Parse the input operand, the attribute dictionary, and the type of the
  // input.
  mlir::OpAsmParser::UnresolvedOperand inputOperand;
  mlir::Type inputType;
  if (parser.parseOperand(inputOperand) ||
      parser.parseOptionalAttrDict(result.attributes) || parser.parseColon() ||
      parser.parseType(inputType))
    return mlir::failure();

  // Resolve the input operand to the type we parsed in.
  if (parser.resolveOperand(inputOperand, inputType, result.operands))
    return mlir::failure();

  return mlir::success();
}
```

定义了 C++ 实现后，让我们看看如何将其映射到[声明式格式](../../DefiningDialects/Operations.md/#declarative-assembly-format)。声明式格式主要由三种不同组件组成：

*   指令（Directives）
    -   一种内置函数类型，带有可选的参数集合。
*   字面量（Literals）
    -   由 \`\` 包围的关键字或标点符号。
*   变量（Variables）
    -   在操作本身上注册的实体，即参数（属性或操作数）、结果、后继等。在上面的 `PrintOp` 示例中，变量为 `$input`。

我们 C++ 格式的直接映射看起来像这样：

```tablegen
/// Consider a stripped definition of `toy.print` here.
def PrintOp : Toy_Op<"print"> {
  let arguments = (ins F64Tensor:$input);

  // In the following format we have two directives, `attr-dict` and `type`.
  // These correspond to the attribute dictionary and the type of a given
  // variable represectively.
  let assemblyFormat = "$input attr-dict `:` type($input)";
}
```

[声明式格式](../../DefiningDialects/Operations.md/#declarative-assembly-format)还有许多更有趣的特性，所以在用 C++ 实现自定义格式之前一定要先了解它。对几个操作的格式进行美化后，我们现在得到了更易读的：

```mlir
module {
  toy.func @multiply_transpose(%arg0: tensor<*xf64>, %arg1: tensor<*xf64>) -> tensor<*xf64> {
    %0 = toy.transpose(%arg0 : tensor<*xf64>) to tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:10)
    %1 = toy.transpose(%arg1 : tensor<*xf64>) to tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:25)
    %2 = toy.mul %0, %1 : tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:25)
    toy.return %2 : tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":5:3)
  } loc("test/Examples/Toy/Ch2/codegen.toy":4:1)
  toy.func @main() {
    %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":9:17)
    %1 = toy.reshape(%0 : tensor<2x3xf64>) to tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":9:3)
    %2 = toy.constant dense<[1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00]> : tensor<6xf64> loc("test/Examples/Toy/Ch2/codegen.toy":10:17)
    %3 = toy.reshape(%2 : tensor<6xf64>) to tensor<2x3xf64> loc("test/Examples/Toy/Ch2/codegen.toy":10:3)
    %4 = toy.generic_call @multiply_transpose(%1, %3) : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":11:11)
    %5 = toy.generic_call @multiply_transpose(%3, %1) : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":12:11)
    toy.print %5 : tensor<*xf64> loc("test/Examples/Toy/Ch2/codegen.toy":13:3)
    toy.return loc("test/Examples/Toy/Ch2/codegen.toy":8:1)
  } loc("test/Examples/Toy/Ch2/codegen.toy":8:1)
} loc(unknown)
```

上面我们介绍了在 ODS 框架中定义操作的几个概念，但还有许多我们尚未涉及的内容：区域、可变参数操作数等。查阅[完整规范](../../DefiningDialects/Operations.md)了解更多详情。

## 完整的 Toy 示例

现在我们可以生成我们的"Toy IR"了。您可以构建 `toyc-ch2` 并在上面的示例上亲自尝试：`toyc-ch2 test/Examples/Toy/Ch2/codegen.toy -emit=mlir -mlir-print-debuginfo`。我们还可以检查往返：先运行 `toyc-ch2 test/Examples/Toy/Ch2/codegen.toy -emit=mlir -mlir-print-debuginfo 2> codegen.mlir`，再运行 `toyc-ch2 codegen.mlir -emit=mlir`。您还应该对最终定义文件使用 `mlir-tblgen` 并研究生成的 C++ 代码。

此时，MLIR 已了解我们的 Toy 方言和操作。在[下一章](Ch-3.md)中，我们将利用我们新的方言为 Toy 语言实现一些高层次的语言特定分析和转换。
