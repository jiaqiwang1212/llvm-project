# 定义方言

本文档描述了如何定义[方言](../LangRef.md/#dialects)。

[TOC]

## LangRef 简要回顾

在深入介绍如何定义这些构造之前，以下是来自 [MLIR LangRef](../LangRef.md) 的快速回顾。

方言是与 MLIR 生态系统交互和扩展的机制。它们允许定义新的[属性](../LangRef.md/#attributes)、[操作](../LangRef.md/#operations)和[类型](../LangRef.md/#type-system)。方言用于对各种不同的抽象进行建模；从传统[算术](../Dialects/ArithOps.md)到[模式重写](../Dialects/PDLOps.md)；是 MLIR 最基本的方面之一。

## 定义方言

在最基本的层面上，在 MLIR 中定义方言就像特化 [C++ `Dialect` 类](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/IR/Dialect.h)一样简单。话虽如此，MLIR 通过 [TableGen](https://llvm.org/docs/TableGen/index.html) 提供了强大的声明式规范机制；这是一种通用语言，带有工具来维护领域特定信息的记录；它通过自动生成所有必要的样板 C++ 代码简化了定义过程，在更改方言定义的各个方面时显著减少了维护负担，并在此基础上提供了额外的工具（如文档生成）。鉴于上述原因，声明式规范是定义新方言的预期机制，也是本文档中详细介绍的方法。在继续之前，强烈建议用户查阅 [TableGen 程序员参考](https://llvm.org/docs/TableGen/ProgRef.html)，了解其语法和构造的介绍。

以下展示了一个简单的方言定义示例。我们通常建议在不同的 `.td` 文件中定义 Dialect 类和属性、操作、类型及其他方言子组件，以在各种不同的方言组件之间建立适当的分层。这也可以防止意外为某些构造生成多个定义的情况。此建议适用于所有 MLIR 构造，包括例如[接口](../Interfaces.md)。

```tablegen
// 包含定义我们方言所需的 tablegen 构造的定义。
include "mlir/IR/DialectBase.td"

// 这是一个简单的方言定义。
def MyDialect : Dialect {
  let summary = "我的方言的简短单行描述。";
  let description = [{
    我的方言是一个非常重要的方言。本节包含更详细的描述，
    记录了所有重要的信息。
  }];

  /// 这是方言的命名空间。它用于封装方言的子组件，
  /// 例如操作（"my_dialect.foo"）。
  let name = "my_dialect";

  /// 方言及其子组件所在的 C++ 命名空间。
  let cppNamespace = "::my_dialect";
}
```

上面展示了对方言的非常简单的描述，但方言有许多其他功能，你可能需要也可能不需要使用。

### 初始化

每个方言都必须实现一个初始化钩子，以添加属性、操作、类型、附加任何所需接口，或执行任何其他在构造时应发生的方言必要初始化。此钩子为每个方言声明以供定义，形式为：

```c++
void MyDialect::initialize() {
  // 方言初始化逻辑应在此处定义。
}
```

### 文档

`summary` 和 `description` 字段允许为方言提供用户文档。`summary` 字段期望一个简单的单行字符串，`description` 字段用于长篇和详细的文档。此文档可用于为方言生成 Markdown 文档，并由上游 [MLIR 方言](https://mlir.llvm.org/docs/Dialects/)使用。

### 类名

生成的 C++ 类的名称与 TableGen 方言定义的名称相同，但去掉了所有 `_` 字符。这意味着如果将方言命名为 `Foo_Dialect`，生成的 C++ 类将是 `FooDialect`。在上面的示例中，我们将得到一个名为 `MyDialect` 的 C++ 方言。

### C++ 命名空间

我们方言的 C++ 类及其所有子组件所在的命名空间由 `cppNamespace` 字段指定。默认情况下，使用方言名称作为唯一命名空间。要避免放置在任何命名空间中，使用 `""`。要指定嵌套命名空间，使用 `"::"` 作为命名空间之间的分隔符，例如，给定 `"A::B"`，C++ 类将被放置在：`namespace A { namespace B { <classes> } }` 中。

注意，这与方言的 C++ 代码配合工作。根据如何包含生成的文件，你可能希望指定完整的命名空间路径或部分路径。通常，尽可能使用完整命名空间是最好的做法。这使得不同命名空间和项目中的方言更容易相互交互。

### C++ 访问器生成

在为方言及其组件（属性、操作、类型等）生成访问器时，我们分别用 `get` 和 `set` 作为名称前缀，并将 `snake_style` 名称转换为驼峰命名（前缀时使用 `UpperCamel`，单独变量名使用 `lowerCamel`）。例如，如果操作定义为：

```tablegen
def MyOp : MyDialect<"op"> {
  let arguments = (ins StrAttr:$value, StrAttr:$other_value);
}
```

将为 `value` 和 `other_value` 属性生成如下访问器：

```c++
StringAttr MyOp::getValue();
void MyOp::setValue(StringAttr newValue);

StringAttr MyOp::getOtherValue();
void MyOp::setOtherValue(StringAttr newValue);
```

### 依赖方言

MLIR 拥有非常大的生态系统，包含服务于许多不同用途的方言。鉴于此，方言可能希望重用其他方言的某些组件是相当常见的。这可能意味着在规范化期间从这些方言生成操作、重用属性或类型等。当一个方言依赖另一个方言时，即当它构建和/或通常依赖另一个方言的组件时，应显式记录方言依赖关系。显式依赖关系确保依赖方言与该方言一起加载。方言依赖关系可以使用 `dependentDialects` 方言字段记录：

```tablegen
def MyDialect : Dialect {
  // 在此处注册 Arithmetic 和 Func 方言作为 `MyDialect` 的依赖项。
  let dependentDialects = [
    "arith::ArithDialect",
    "func::FuncDialect"
  ];
}
```

### 额外声明

声明式方言定义尝试尽可能多地自动生成逻辑和方法。话虽如此，总会有无法覆盖的长尾情况。对于这些情况，可以使用 `extraClassDeclaration`。`extraClassDeclaration` 字段中的代码将被字面复制到生成的 C++ Dialect 类中。

注意，`extraClassDeclaration` 是一种为高级用户处理长尾情况的机制；对于尚未实现但广泛适用的情况，改进基础设施更为可取。

### `hasConstantMaterializer`：从属性实体化常量

此字段用于从 `Attribute` 值和 `Type` 实体化常量操作。这通常在此方言中的操作被折叠，并且应生成常量操作时使用。`hasConstantMaterializer` 用于启用实体化，并在方言上声明 `materializeConstant` 钩子。此钩子接受一个 `Attribute` 值（通常由 `fold` 返回），并生成一个实体化该值的"类常量"操作。有关 MLIR 中 `folding` 的更深入介绍，请参阅[规范化文档](../Canonicalization.md)。

常量实体化逻辑可以在源文件中定义：

```c++
/// 从给定属性值和所需结果类型实体化单个常量操作的钩子。
/// 此方法应使用提供的 builder 创建操作，而不更改插入位置。
/// 生成的操作预期是类常量的。成功时，此钩子应返回为表示常量值而生成的操作。
/// 否则，失败时应返回 nullptr。
Operation *MyDialect::materializeConstant(OpBuilder &builder, Attribute value,
                                          Type type, Location loc) {
  ...
}
```

### `hasNonDefaultDestructor`：提供自定义析构函数

当 Dialect 类具有自定义析构函数时，即当方言在 `~MyDialect` 中有特殊逻辑要运行时，应使用此字段。在这种情况下，只为 Dialect 类生成析构函数的声明。

### 可丢弃属性验证

如 [MLIR 语言参考](../LangRef.md/#attributes)所述，*可丢弃属性*是一种属性类型，其语义由名称前缀对应的方言定义。例如，如果某个操作具有名为 `gpu.contained_module` 的属性，`gpu` 方言定义了该属性的语义和不变量，如何时以及在何处使用是有效的。为了挂入对我们方言前缀的属性的验证，可以使用方言上的几个钩子：

#### `hasOperationAttrVerify`

此字段生成验证钩子，当此方言的可丢弃属性在操作的属性字典中使用时调用。此钩子的形式为：

```c++
/// 验证在 `op` 的字典中使用的给定属性的用法，其名称以该方言的命名空间为前缀。
LogicalResult MyDialect::verifyOperationAttribute(Operation *op, NamedAttribute attribute);
```

#### `hasRegionArgAttrVerify`

此字段生成验证钩子，当此方言的可丢弃属性在区域入口块参数的属性字典中使用时调用。注意，区域入口块的块参数本身没有属性字典，但某些操作可能提供与区域参数对应的特殊字典属性。例如，实现 `FunctionOpInterface` 的操作可能在操作上具有与函数入口块参数对应的属性字典。在这些情况下，这些操作将在方言上调用此钩子以确保属性得到验证。方言需要实现的钩子形式为：

```c++
/// 验证在区域入口块参数的属性字典中使用的给定属性的用法，
/// 其名称以该方言的命名空间为前缀。
/// 注意：如上所述，区域入口块何时具有字典取决于各个操作的定义。 
LogicalResult MyDialect::verifyRegionArgAttribute(Operation *op, unsigned regionIndex,
                                                  unsigned argIndex, NamedAttribute attribute);
```

#### `hasRegionResultAttrVerify`

此字段生成验证钩子，当此方言的可丢弃属性在区域结果的属性字典中使用时调用。注意，区域的结果本身没有属性字典，但某些操作可能提供与区域结果对应的特殊字典属性。例如，实现 `FunctionOpInterface` 的操作可能在操作上具有与函数结果对应的属性字典。在这些情况下，这些操作将在方言上调用此钩子以确保属性得到验证。方言需要实现的钩子形式为：

```c++
/// 为在区域结果的属性字典中使用的给定属性生成验证，
/// 其名称以该方言的命名空间为前缀。
/// 注意：如上所述，区域入口块何时具有字典取决于各个操作的定义。 
LogicalResult MyDialect::verifyRegionResultAttribute(Operation *op, unsigned regionIndex,
                                                     unsigned argIndex, NamedAttribute attribute);
```

### 操作接口回退

某些方言拥有开放的生态系统，不注册所有可能的操作。在这种情况下，仍然可以为这些操作提供实现 `OpInterface` 的支持。当操作未注册或未为接口提供实现时，查询将回退到方言本身。可以使用 `hasOperationInterfaceFallback` 字段为操作声明此回退：

```c++
/// 为具有给定名称的操作返回具有给定 `typeId` 的接口的接口模型。
void *MyDialect::getRegisteredInterfaceForOp(TypeID typeID, StringAttr opName);
```

有关此钩子预期用途的更详细描述，请查看详细的[接口文档](../Interfaces.md/#dialect-fallback-for-opinterface)。

### 默认属性/类型解析器和打印器 

当方言注册属性或类型时，还必须覆盖相应的 `Dialect::parseAttribute`/`Dialect::printAttribute` 或 `Dialect::parseType`/`Dialect::printType` 方法。在这些情况下，方言必须显式处理方言中每个单独属性或类型的解析和打印。但是，如果方言的所有属性和类型都提供了助记符，则可以通过使用 `useDefaultAttributePrinterParser` 和 `useDefaultTypePrinterParser` 字段来自动生成这些方法。默认情况下，这些字段设置为 `1`（启用），这意味着如果方言需要显式处理其属性和类型的解析器和打印器，应根据需要将这些设置为 `0`。

### 方言范围的规范化模式

通常，[规范化](../Canonicalization.md)模式特定于方言中的各个操作。但在某些情况下，需要在方言级别添加规范化模式。例如，如果方言定义了一个在接口或 trait 上操作的规范化模式，只添加一次而不是在实现该接口的每个操作中复制可能更有好处。要启用此钩子的生成，可以使用 `hasCanonicalizer` 字段。这将在方言上声明 `getCanonicalizationPatterns` 方法，其形式为：

```c++
/// 返回此方言的规范化模式：
void MyDialect::getCanonicalizationPatterns(RewritePatternSet &results) const;
```

有关规范化模式的更详细描述，请参阅 [MLIR 中的规范化文档](../Canonicalization.md)。

### 为方言属性和类型定义字节码格式

默认情况下，方言属性和类型的字节码序列化使用常规文本格式。方言可以通过定义并附加 `BytecodeDialectInterface` 到方言来为方言中的属性和类型定义更紧凑的字节码格式。可以使用 ODS 的 `-gen-bytecode` 来生成字节码方言接口的读/写器的基本支持。本节其余部分将展示一个示例。

可以为方言 `Foo` 中的类型定义如下打印和解析：

```td
include "mlir/IR/BytecodeBase.td"

let cType = "MemRefType" in {
// 以伪代码显示降低的编码：
//   ///   MemRefType {
//   ///     shape: svarint[],
//   ///     elementType: Type,
//   ///     layout: Attribute
//   ///   }
//   ///
// 和枚举值：
//   kMemRefType = 1,
//
// ODS 生成器中的对应定义：
def MemRefType : DialectType<(type
  Array<SignedVarInt>:$shape,
  Type:$elementType,
  MemRefLayout:$layout
)> {
  let printerPredicate = "!$_val.getMemorySpace()";
}

//   ///   MemRefTypeWithMemSpace {
//   ///     memorySpace: Attribute,
//   ///     shape: svarint[],
//   ///     elementType: Type,
//   ///     layout: Attribute
//   ///   }
//   /// 具有非默认内存空间的 MemRefType 变体。
//   kMemRefTypeWithMemSpace = 2,
def MemRefTypeWithMemSpace : DialectType<(type
  Attribute:$memorySpace,
  Array<SignedVarInt>:$shape,
  Type:$elementType,
  MemRefLayout:$layout
)> {
  let printerPredicate = "!!$_val.getMemorySpace()";
  // 注意：序列化顺序与构建器顺序不匹配。
  let cBuilder = "get<$_resultType>(context, shape, elementType, layout, memorySpace)";
}
}

def FooDialectTypes : DialectTypes<"Foo"> {
  let elems = [
    ReservedOrDead,         // 分配索引 0
    MemRefType,             // 分配索引 1
    MemRefTypeWithMemSpace, // 分配索引 2
    ...
  ];
}
...
```

这里我们有：

*   最外层的 `cType`，因为我们使用两种不同的变体来表示一种 C++ 类型的编码。
*   不同的 `DialectType` 实例通过打印器谓词在打印时区分，而解析不同变体已被编码，并调用不同的构建器函数。
*   指定了自定义 `cBuilder`，因为它在磁盘上的布局与类型的构建方法的参数顺序不匹配。
*   许多常见的方言字节码读写原子（如 `VarInt`、`SVarInt`、`Blob`）定义在 `BytecodeBase` 中，也可以定义自定义形式或通过 `CompositeBytecode` 实例进行组合。
*   `ReservedOrDead` 是一个特殊关键字，用于指示跳过的枚举实例，不为其生成读/写或分发代码。
*   `Array` 是一个辅助方法，在打印时序列化列表（例如，项数的 varint，后跟该数量的项），或在解析时。

生成的代码由四个独立方法组成，以下接口可以定义字节码方言接口：

```c++
#include "mlir/Dialect/Foo/FooDialectBytecode.cpp.inc"

struct FooDialectBytecodeInterface : public BytecodeDialectInterface {
  FooDialectBytecodeInterface(Dialect *dialect)
      : BytecodeDialectInterface(dialect) {}

  //===--------------------------------------------------------------------===//
  // 属性

  Attribute readAttribute(DialectBytecodeReader &reader) const override {
    return ::readAttribute(getContext(), reader);
  }

  LogicalResult writeAttribute(Attribute attr,
                               DialectBytecodeWriter &writer) const override {
    return ::writeAttribute(attr, writer);
  }

  //===--------------------------------------------------------------------===//
  // 类型

  Type readType(DialectBytecodeReader &reader) const override {
    return ::readType(getContext(), reader);
  }

  LogicalResult writeType(Type type,
                          DialectBytecodeWriter &writer) const override {
    return ::writeType(type, writer);
  }
};
```

以及定义相应的构建规则以调用生成器（`-gen-bytecode -bytecode-dialect="Quant"`）。

## 定义可扩展方言

本节记录了可扩展方言的设计和 API。可扩展方言是可以在运行时使用新操作和类型定义进行扩展的方言。这允许用户通过元编程或从另一种语言定义方言，而无需重新编译 C++ 代码。

### 定义可扩展方言

在 C++ 中定义的方言可以在运行时通过继承 `mlir::ExtensibleDialect` 而非 `mlir::Dialect` 来使用新操作、类型等进行扩展（注意 `ExtensibleDialect` 继承自 `Dialect`）。`ExtensibleDialect` 类包含在运行时扩展方言所需的字段和方法。

```c++
class MyDialect : public mlir::ExtensibleDialect {
    ...
}
```

对于在 TableGen 中定义的方言，这通过将 `isExtensible` 标志设置为 `1` 来实现。

```tablegen
def Test_Dialect : Dialect {
  let isExtensible = 1;
  ...
}
```

可扩展的 `Dialect` 可以使用 `llvm::dyn_cast` 或 `llvm::cast` 转换回 `ExtensibleDialect`：

```c++
if (auto extensibleDialect = llvm::dyn_cast<ExtensibleDialect>(dialect)) {
    ...
}
```

### 定义动态方言

动态方言是可以在运行时定义的可扩展方言。它们只包含动态操作、类型和属性。它们可以使用 `insertDynamic` 在 `DialectRegistry` 中注册。

```c++
auto populateDialect = [](MLIRContext *ctx, DynamicDialect* dialect) {
  // 创建和加载动态方言时运行的代码。
  // 例如，这是注册方言的动态操作、类型和属性的地方。
  ...
}

registry.insertDynamic("dialectName", populateDialect);
```

一旦动态方言在 `MLIRContext` 中注册，就可以使用 `getOrLoadDialect` 检索它。

```c++
Dialect *dialect = ctx->getOrLoadDialect("dialectName");
```

### 在运行时定义操作

`DynamicOpDefinition` 类表示在运行时定义的操作的定义。使用 `DynamicOpDefinition::get` 函数创建它。在运行时定义的操作必须提供名称、注册操作的方言、操作验证器。它还可以选择性地定义自定义解析器和打印器、折叠钩子等。

```c++
// 操作名称，不带方言名称前缀。
StringRef name = "my_operation_name";

// 定义操作的方言。
Dialect* dialect = ctx->getOrLoadDialect<MyDialect>();

// 操作验证器定义。
AbstractOperation::VerifyInvariantsFn verifyFn = [](Operation* op) {
    // 操作验证逻辑。
    ...
}

// 解析器函数定义。
AbstractOperation::ParseAssemblyFn parseFn =
    [](OpAsmParser &parser, OperationState &state) {
        // 解析操作，假设名称已经被解析。
        ...    
};

// 打印器函数
auto printFn = [](Operation *op, OpAsmPrinter &printer) {
        printer << op->getName();
        // 打印操作，假设名称已经被打印。
        ...
};

// 通用折叠器实现，更多信息请参见 AbstractOperation::foldHook。
auto foldHookFn = [](Operation * op, ArrayRef<Attribute> operands, 
                                   SmallVectorImpl<OpFoldResult> &result) {
    ...
};

// 返回操作支持的任何规范化模式重写，供规范化 pass 使用。
auto getCanonicalizationPatterns = 
        [](RewritePatternSet &results, MLIRContext *context) {
    ...
}

// 操作的定义。
std::unique_ptr<DynamicOpDefinition> opDef =
    DynamicOpDefinition::get(name, dialect, std::move(verifyFn),
        std::move(parseFn), std::move(printFn), std::move(foldHookFn),
        std::move(getCanonicalizationPatterns));
```

一旦操作被定义，它可以被 `ExtensibleDialect` 注册：

```c++
extensibleDialect->registerDynamicOperation(std::move(opDef));
```

注意，提供给操作的 `Dialect` 应该是注册该操作的方言。

### 使用在运行时定义的操作

可以使用操作名称匹配在运行时定义的操作：

```c++
if (op->getName().getStringRef() == "my_dialect.my_dynamic_op") {
    ...
}
```

在运行时定义的操作可以通过用操作名称实例化一个 `OperationState`，并将其与重写器（例如 `PatternRewriter`）一起使用来创建操作。

```c++
OperationState state(location, "my_dialect.my_dynamic_op",
                     operands, resultTypes, attributes);

rewriter.createOperation(state);
```

### 在运行时定义类型

与在 C++ 或 TableGen 中定义的类型不同，在运行时定义的类型只能将 `Attribute` 的列表作为参数。

类似于操作，类型在运行时使用 `DynamicTypeDefinition` 类定义，该类使用 `DynamicTypeDefinition::get` 函数创建。类型定义需要名称、将注册该类型的方言以及参数验证器。还可以选择性地定义参数的自定义解析器和打印器（假设类型名称已被解析/打印）。

```c++
// 类型名称，不带方言名称前缀。
StringRef name = "my_type_name";

// 定义类型的方言。
Dialect* dialect = ctx->getOrLoadDialect<MyDialect>();

// 类型验证器。
// 在运行时定义的类型具有属性列表作为参数。
auto verifier = [](function_ref<InFlightDiagnostic()> emitError,
                   ArrayRef<Attribute> args) {
    ...
};

// 类型参数解析器。
auto parser = [](DialectAsmParser &parser,
                 llvm::SmallVectorImpl<Attribute> &parsedParams) {
    ...
};

// 类型参数打印器。
auto printer =[](DialectAsmPrinter &printer, ArrayRef<Attribute> params) {
    ...
};

std::unique_ptr<DynamicTypeDefinition> typeDef =
    DynamicTypeDefinition::get(std::move(name), std::move(dialect),
                               std::move(verifier), std::move(printer),
                               std::move(parser));
```

如果省略了打印器和解析器，则生成格式为 `!dialect.typename<arg1, arg2, ..., argN>` 的默认解析器和打印器。

然后可以由 `ExtensibleDialect` 注册该类型：

```c++
dialect->registerDynamicType(std::move(typeDef));
```

### 在可扩展方言中解析在运行时定义的类型

TableGen 生成的 `parseType` 方法可以解析在运行时定义的类型，但覆盖的 `parseType` 方法需要为其添加必要的支持。

```c++
Type MyDialect::parseType(DialectAsmParser &parser) const {
    ...
    
    // 类型名称。
    StringRef typeTag;
    if (failed(parser.parseKeyword(&typeTag)))
        return Type();

    // 尝试解析名称为 'typeTag' 的动态类型。
    Type dynType;
    auto parseResult = parseOptionalDynamicType(typeTag, parser, dynType);
    if (parseResult.has_value()) {
        if (succeeded(parseResult.getValue()))
            return dynType;
         return Type();
    }

    ...
}
```

### 使用在运行时定义的类型

动态类型是 `DynamicType` 的实例。可以使用 `DynamicType::get` 和 `ExtensibleDialect::lookupTypeDefinition` 获取动态类型。

```c++
auto typeDef = extensibleDialect->lookupTypeDefinition("my_dynamic_type");
ArrayRef<Attribute> params = ...;
auto type = DynamicType::get(typeDef, params);
```

也可以将已知在运行时定义的 `Type` 转换为 `DynamicType`。

```c++
auto dynType = cast<DynamicType>(type);
auto typeDef = dynType.getTypeDef();
auto args = dynType.getParams();
```

### 在运行时定义属性

与在运行时定义的类型类似，在运行时定义的属性只能将 `Attribute` 的列表作为参数。

类似于类型，属性在运行时使用 `DynamicAttrDefinition` 类定义，该类使用 `DynamicAttrDefinition::get` 函数创建。属性定义需要名称、将注册该属性的方言以及参数验证器。还可以选择性地定义参数的自定义解析器和打印器（假设属性名称已被解析/打印）。

```c++
// 属性名称，不带方言名称前缀。
StringRef name = "my_attribute_name";

// 定义属性的方言。
Dialect* dialect = ctx->getOrLoadDialect<MyDialect>();

// 属性验证器。
// 在运行时定义的属性具有属性列表作为参数。
auto verifier = [](function_ref<InFlightDiagnostic()> emitError,
                   ArrayRef<Attribute> args) {
    ...
};

// 属性参数解析器。
auto parser = [](DialectAsmParser &parser,
                 llvm::SmallVectorImpl<Attribute> &parsedParams) {
    ...
};

// 属性参数打印器。
auto printer =[](DialectAsmPrinter &printer, ArrayRef<Attribute> params) {
    ...
};

std::unique_ptr<DynamicAttrDefinition> attrDef =
    DynamicAttrDefinition::get(std::move(name), std::move(dialect),
                               std::move(verifier), std::move(printer),
                               std::move(parser));
```

如果省略了打印器和解析器，则生成格式为 `!dialect.attrname<arg1, arg2, ..., argN>` 的默认解析器和打印器。

然后可以由 `ExtensibleDialect` 注册该属性：

```c++
dialect->registerDynamicAttr(std::move(typeDef));
```

### 在可扩展方言中解析在运行时定义的属性

TableGen 生成的 `parseAttribute` 方法可以解析在运行时定义的属性，但覆盖的 `parseAttribute` 方法需要为其添加必要的支持。

```c++
Attribute MyDialect::parseAttribute(DialectAsmParser &parser,
                                    Type type) const override {
    ...
    // 属性名称。
    StringRef attrTag;
    if (failed(parser.parseKeyword(&attrTag)))
        return Attribute();

    // 尝试解析名称为 'attrTag' 的动态属性。
    Attribute dynAttr;
    auto parseResult = parseOptionalDynamicAttr(attrTag, parser, dynAttr);
    if (parseResult.has_value()) {
        if (succeeded(*parseResult))
            return dynAttr;
         return Attribute();
    }
```

### 使用在运行时定义的属性

类似于类型，在运行时定义的属性是 `DynamicAttr` 的实例。可以使用 `DynamicAttr::get` 和 `ExtensibleDialect::lookupAttrDefinition` 获取动态属性。

```c++
auto attrDef = extensibleDialect->lookupAttrDefinition("my_dynamic_attr");
ArrayRef<Attribute> params = ...;
auto attr = DynamicAttr::get(attrDef, params);
```

也可以将已知在运行时定义的 `Attribute` 转换为 `DynamicAttr`。

```c++
auto dynAttr = cast<DynamicAttr>(attr);
auto attrDef = dynAttr.getAttrDef();
auto args = dynAttr.getParams();
```

### 可扩展方言的实现细节

#### 可扩展方言

可扩展方言的作用是拥有已定义操作和类型所需的数据。它们还包含用于轻松访问它们的必要访问器。

为了将 `Dialect` 转换回 `ExtensibleDialect`，我们为所有 `ExtensibleDialect` 实现了 `IsExtensibleDialect` 接口。转换是通过检查 `Dialect` 是否实现了 `IsExtensibleDialect` 来完成的。

#### 操作的表示和注册

操作在 mlir 中使用 `AbstractOperation` 类表示。它们以与在 C++ 中定义的操作相同的方式在方言中注册，即通过调用 `AbstractOperation::insert`。

唯一的区别是需要为每个操作创建一个新的 `TypeID`，因为操作不是由 C++ 类表示的。这通过使用 `TypeIDAllocator` 来完成，它可以在运行时分配一个新的唯一 `TypeID`。

#### 类型的表示和注册

与操作不同，类型需要定义一个处理类型参数的 C++ 存储类。它们还需要定义另一个 C++ 类来访问该存储。`DynamicTypeStorage` 定义了在运行时定义的类型的存储，`DynamicType` 提供对存储的访问，以及定义有用的函数。`DynamicTypeStorage` 包含一个 `Attribute` 类型参数列表，以及一个指向类型定义的指针。

类型使用 `Dialect::addType` 方法注册，该方法期望一个使用 `TypeIDAllocator` 生成的 `TypeID`。类型唯一化器也用给定的 `TypeID` 注册类型。这意味着我们可以使用单个 `DynamicType` 和不同的 `TypeID` 来表示在运行时定义的不同类型。

由于在运行时定义的不同类型具有不同的 `TypeID`，因此无法使用 `TypeID` 将 `Type` 转换为 `DynamicType`。因此，类似于 `Dialect`，所有 `DynamicType` 都定义了一个 `IsDynamicTypeTrait`，因此将 `Type` 转换为 `DynamicType` 归结为查询 `IsDynamicTypeTrait` trait。
