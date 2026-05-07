# 定义方言属性和类型

本文档描述了如何定义方言[属性](../LangRef.md/#attributes)和[类型](../LangRef.md/#type-system)。

[TOC]

## LangRef 简要回顾

在深入介绍如何定义这些构造之前，以下是来自 [MLIR LangRef](../LangRef.md) 的快速回顾。

### 属性

属性是在操作中指定常量数据的机制，用于绝不允许使用变量的地方——例如 [`arith.cmpi` 操作](../Dialects/ArithOps.md/#arithcmpi-arithcmpiop)的比较谓词，或 [`arith.constant` 操作](../Dialects/ArithOps.md/#arithconstant-arithconstantop)的底层值。每个操作都有一个属性字典，将一组属性名称关联到属性值。

### 类型

MLIR 中的每个 SSA 值（如操作结果或块参数）都具有类型系统定义的类型。MLIR 拥有一个开放的类型系统，没有固定的类型列表，对它们所表示的抽象也没有限制。例如，以下[算术 AddI 操作](../Dialects/ArithOps.md/#arithaddi-arithaddiop)：

```mlir
  %result = arith.addi %lhs, %rhs : i64
```

它接受两个输入 SSA 值（`%lhs` 和 `%rhs`），并返回一个 SSA 值（`%result`）。此操作的输入和输出类型为 `i64`，是[内置 IntegerType](../Dialects/Builtin.md/#integertype) 的实例。

## 属性和类型

MLIR 中的 C++ Attribute 和 Type 类（与 Op 及许多其他类一样）是值类型的。这意味着 `Attribute` 或 `Type` 的实例按值传递，而不是按指针或引用。`Attribute` 和 `Type` 类充当在 `MLIRContext` 实例中唯一化的内部存储对象的包装器。

定义属性和类型的结构几乎相同，只有少数区别取决于上下文。因此，本文档的大部分内容并排描述了定义属性和类型的过程，并为两者提供示例。如有必要，某些章节会明确指出任何明显的差异。

一个区别是，从声明式 TableGen 定义生成 C++ 类需要在 `CMakeLists.txt` 中添加额外的目标。自定义类型不需要这样做。详细信息将在下文进一步说明。

### 添加新的属性或类型定义

如上所述，MLIR 中的 C++ Attribute 和 Type 对象是值类型的，本质上充当保存类型实际数据的内部存储对象的有用包装器。与操作类似，属性和类型通过 [TableGen](https://llvm.org/docs/TableGen/index.html) 声明式定义；这是一种通用语言，带有工具来维护领域特定信息的记录。强烈建议用户查阅 [TableGen 程序员参考](https://llvm.org/docs/TableGen/ProgRef.html)，了解其语法和构造的介绍。

开始定义新属性或类型只需分别为 `AttrDef` 或 `TypeDef` 类添加特化即可。这些类的实例对应唯一的 Attribute 或 Type 类。

以下展示了一个示例 Attribute 和 Type 定义。我们通常建议在不同的 `.td` 文件中定义 Attribute 和 Type 类，以更好地封装不同的构造，并在它们之间定义适当的分层。此建议适用于所有 MLIR 构造，包括[接口](../Interfaces.md)、操作等。

```tablegen
// 包含定义我们类型所需的 tablegen 构造的定义。
include "mlir/IR/AttrTypeBase.td"

// 通常在同一方言中为类型定义基类。这样
// 就不需要为每个类型传入方言，也可以用来
// 提前定义几个字段。
class MyDialect_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<My_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

// 这是一个简单的"整数"类型定义，带有宽度参数。
def My_IntegerType : MyDialect_Type<"Integer", "int"> {
  let summary = "任意精度直到固定限制的整数类型";
  let description = [{
    整数类型具有指定的位宽。
  }];
  /// 这里我们为类型定义了单个参数，即位宽。
  let parameters = (ins "unsigned":$width);

  /// 这里我们以声明式方式定义类型的文本格式，这将
  /// 自动生成解析器和打印器逻辑。这将允许类型实例
  /// 输出为，例如：
  ///
  ///    !my.int<10> // 10 位整数。
  ///
  let assemblyFormat = "`<` $width `>`";

  /// 指示我们的类型将向参数添加额外的验证。
  let genVerifyDecl = 1;

  /// 指示我们的类型将在汇编中使用助记符作为别名。
  let genMnemonicAlias = 1;
}
```

以下是一个属性示例：

```tablegen
// 包含定义我们属性所需的 tablegen 构造的定义。
include "mlir/IR/AttrTypeBase.td"

// 通常在同一方言中为属性定义基类。这样
// 就不需要为每个属性传入方言，也可以用来
// 提前定义几个字段。
class MyDialect_Attr<string name, string attrMnemonic, list<Trait> traits = []>
    : AttrDef<My_Dialect, name, traits> {
  let mnemonic = attrMnemonic;
}

// 这是一个简单的"整数"属性定义，带有类型和值参数。
def My_IntegerAttr : MyDialect_Attr<"Integer", "int"> {
  let summary = "包含整数值的属性";
  let description = [{
    整数属性是一个字面量属性，表示指定整数类型的整数值。
  }];
  /// 这里我们定义了两个参数，一个是"self"类型参数，另一个是
  /// 属性的整数值。self 类型参数由汇编格式特殊处理。
  let parameters = (ins AttributeSelfTypeParameter<"">:$type, APIntParameter<"">:$value);

  /// 这里我们为类型定义了一个自定义构建器，它消除了传入
  /// MLIRContext 实例的需要；因为可以从 `type` 推断出来。
  let builders = [
    AttrBuilderWithInferredContext<(ins "Type":$type,
                                        "const APInt &":$value), [{
      return $_get(type.getContext(), type, value);
    }]>
  ];

  /// 这里我们以声明式方式定义属性的文本格式，这将
  /// 自动生成解析器和打印器逻辑。这将允许属性实例
  /// 输出为，例如：
  ///
  ///    #my.int<50> : !my.int<32> // 值为 50 的 32 位整数。
  ///
  /// 注意，self 类型参数不包含在汇编格式中。
  /// 其值来自所有属性上的可选尾部类型。
  let assemblyFormat = "`<` $value `>`";

  /// 指示我们的属性将向参数添加额外的验证。
  let genVerifyDecl = 1;

  /// 向 ODS 生成器指示我们不需要默认构建器，
  /// 因为我们已经定义了自己更简单的构建器。
  let skipDefaultBuilders = 1;

  /// 指示我们的属性将在汇编中使用助记符作为别名。
  let genMnemonicAlias = 1;
}
```

### 类名

生成的 C++ 类的名称默认分别为属性和类型的 `<classParamName>Attr` 或 `<classParamName>Type`。在上面的示例中，这是提供给 `MyDialect_Attr` 和 `MyDialect_Type` 的 `name` 模板参数。对于我们上面添加的定义，我们将分别得到名为 `IntegerType` 和 `IntegerAttr` 的 C++ 类。可以通过 `cppClassName` 字段显式覆盖此名称。

### CMake 目标

如果你在 `CMakeLists.txt` 中使用 `add_mlir_dialect()` 添加了方言，则上述类将自动为自定义*类型*生成。它们将输出在名为 `<Your Dialect>Types.h.inc` 的文件中。

要同时为自定义*属性*生成类，你需要在 `CMakeLists.txt` 中添加两个额外的 TableGen 目标：

```cmake
mlir_tablegen(<Your Dialect>AttrDefs.h.inc -gen-attrdef-decls 
              -attrdefs-dialect=<Your Dialect>)
mlir_tablegen(<Your Dialect>AttrDefs.cpp.inc -gen-attrdef-defs 
              -attrdefs-dialect=<Your Dialect>)
add_public_tablegen_target(<Your Dialect>AttrDefsIncGen)
```

生成的 `<Your Dialect>AttrDefs.h.inc` 需要在任何引用自定义属性类型的地方包含。

### 文档

`summary` 和 `description` 字段允许为属性或类型提供用户文档。`summary` 字段期望一个简单的单行字符串，`description` 字段用于长篇和详细的文档。此文档可用于为方言生成 Markdown 文档，并由上游 [MLIR 方言](https://mlir.llvm.org/docs/Dialects/)使用。

### 助记符

`mnemonic` 字段，即我们上面指定的模板参数 `attrMnemonic` 和 `typeMnemonic`，用于在解析期间指定名称。这允许在解析 IR 时更容易地分派到当前属性或类型类。此字段通常是可选的，可以在不定义它的情况下添加自定义解析/打印逻辑，但大多数类都会希望利用它提供的便利。这就是为什么我们在上面的示例中将其作为模板参数添加的原因。

### 参数

`parameters` 字段是包含属性或类型参数的可变长度列表。如果未指定参数（默认值），则该类型被视为单例类型（意味着只有一个可能的实例）。此列表中的参数采用以下形式：`"c++Type":$paramName`。C++ 类型需要在上下文中构建存储实例时进行分配的参数类型需要以下之一：

- 使用 `AttrParameter` 或 `TypeParameter` 类而不是原始的 "c++Type" 字符串。这允许在使用该参数时提供自定义分配代码。`StringRefParameter` 和 `ArrayRefParameter` 是需要分配的常见参数类型示例。
- 将 `genAccessors` 字段设置为 1（默认值）以为每个参数生成访问器方法（例如，上面 Type 示例中的 `int getWidth() const`）。
- 将 `hasCustomStorageConstructor` 字段设置为 `1` 以生成一个只声明构造函数的存储类，允许你用任何必要的分配代码对其进行特化。

#### AttrParameter、TypeParameter 和 AttrOrTypeParameter

如上所述，这些类允许指定具有附加功能的参数类型。这通常适用于复杂参数，或具有额外不变量而无法使用原始 C++ 类的参数。示例包括文档（例如 `summary` 和 `syntax` 字段）、C++ 类型、在存储构造函数方法中使用的自定义分配器、用于决定参数类型的两个实例是否相等的自定义比较器等。顾名思义，`AttrParameter` 用于属性上的参数，`TypeParameter` 用于 Type 参数，`AttrOrTypeParameters` 用于两者。

以下是一个容易出错的参数陷阱，并强调了何时使用这些参数类。

```tablegen
let parameters = (ins "ArrayRef<int>":$dims);
```

上面看起来无害，但通常是一个 bug！默认存储构造函数盲目地按值复制参数。它对类型一无所知，这意味着这个 ArrayRef 的数据将被原样复制，如果底层数据的生命周期不超过 MLIRContext 的生命周期，那么在使用创建的 Attribute 或 Type 时很可能导致 use-after-free 错误。如果无法保证数据的生命周期，`ArrayRef<int>` 需要分配以确保其元素驻留在 MLIRContext 中，例如 `dims = allocator.copyInto(dims)`。

以下是针对上述情况的简单示例：

```tablegen
def ArrayRefIntParam : TypeParameter<"::llvm::ArrayRef<int>", "Array of int"> {
  let allocator = "$_dst = $_allocator.copyInto($_self);";
}

参数可以这样使用：

...
let parameters = (ins ArrayRefIntParam:$dims);
```

以下包含对其他各种可用字段的描述：

`allocator` 代码块有以下替换：

- `$_allocator` 是在其中分配对象的 TypeStorageAllocator。
- `$_dst` 是放置分配数据的变量。

`comparator` 代码块有以下替换：

- `$_lhs` 是参数类型的实例。
- `$_rhs` 是参数类型的实例。

MLIR 包含几个用于常见情况的特化类：

- `APFloatParameter` 用于 APFloat。

- `StringRefParameter<descriptionOfParam>` 用于 StringRef。

- `ArrayRefParameter<arrayOf, descriptionOfParam>` 用于值类型的 ArrayRef。

- `SelfAllocationParameter<descriptionOfParam>` 用于包含 `allocateInto(StorageAllocator &allocator)` 方法将自身分配到 `allocator` 中的 C++ 类。

- `ArrayRefOfSelfAllocationParameter<arrayOf, descriptionOfParam>` 用于按上述最后一个特化自我分配的对象数组。

- `AttributeSelfTypeParameter` 是一个特殊的 `AttrParameter`，表示从属性上的可选尾部类型派生的参数。

### Traits

与操作类似，Attribute 和 Type 类可以附加 `Traits`，提供额外的 mixin 方法和其他数据。`Trait` 可以通过尾部模板参数附加，即上面示例中的 `traits` 列表参数。有关定义和使用 trait 的更多信息，请参阅主要的 [`Trait`](../Traits) 文档。

### 接口

Attribute 和 Type 类可以附加 `Interfaces`，为属性或类型提供虚接口。`Interfaces` 以与 [Traits](#Traits) 相同的方式添加，使用 `AttrDef` 或 `TypeDef` 的 `traits` 列表模板参数。有关定义和使用接口的更多信息，请参阅主要的 [`Interface`](../Interfaces.md) 文档。

### 构建器

对于每个属性或类型，会根据类型的参数自动生成几个构建器（`get`/`getChecked`）。这些用于构建对应属性或类型的实例。例如，给定以下定义：

```tablegen
def MyAttrOrType : ... {
  let parameters = (ins "int":$intParam);
}
```

将生成以下构建器：

```c++
// 构建器命名为 `get`，返回给定参数集的新实例。
static MyAttrOrType get(MLIRContext *context, int intParam);

// 如果 `genVerifyDecl` 设置为 1，还会生成以下方法。此方法
// 类似于 `get`，但可能失败，出错时返回 nullptr。
static MyAttrOrType getChecked(function_ref<InFlightDiagnostic()> emitError,
                               MLIRContext *context, int intParam);
```

如果不需要这些自动生成的方法（例如它们与自定义构建器方法冲突），可以将 `skipDefaultBuilders` 字段设置为 1，以表示不应生成默认构建器。

#### 自定义构建器方法

默认构建器方法可能涵盖大多数与构建相关的简单情况，但当它们不能满足属性或类型的所有需求时，可以通过 `builders` 字段定义额外的构建器。`builders` 字段是自定义构建器的列表，对于类型使用 `TypeBuilder`，对于属性使用 `AttrBuilder`，这些构建器被添加到属性或类型类中。以下将展示为自定义类型 `MyType` 定义构建器的几个示例，属性的过程相同，只是属性使用 `AttrBuilder` 而不是 `TypeBuilder`。

```tablegen
def MyType : ... {
  let parameters = (ins "int":$intParam);

  let builders = [
    TypeBuilder<(ins "int":$intParam)>,
    TypeBuilder<(ins CArg<"int", "0">:$intParam)>,
    TypeBuilder<(ins CArg<"int", "0">:$intParam), [{
      // 在此内联编写 `get` 构建器的主体。
      return Base::get($_ctxt, intParam);
    }]>,
    TypeBuilderWithInferredContext<(ins "Type":$typeParam), [{
      // 此构建器表明它可以从参数推断 MLIRContext 实例。
      return Base::get(typeParam.getContext(), ...);
    }]>,
    TypeBuilder<(ins "int":$intParam), [{}], "IntegerType">,
  ];
}
```

在此示例中，我们提供了几个在不同场景下有用的便捷构建器。`ins` 前缀在 ODS 的许多函数声明中很常见，它们使用 TableGen [`dag`](#tablegen-syntax)。后面是以逗号分隔的类型（带引号的字符串或 `CArg`）和以 `$` 符号为前缀的名称的列表。使用 `CArg` 允许为该参数提供默认值。让我们逐一查看这些构建器：

第一个构建器将生成如下构建器方法的声明：

```tablegen
  let builders = [
    TypeBuilder<(ins "int":$intParam)>,
  ];
```

```c++
class MyType : /*...*/ {
  /*...*/
  static MyType get(::mlir::MLIRContext *context, int intParam);
};
```

此构建器与将为 `MyType` 自动生成的构建器相同。`context` 参数由生成器隐式添加，在构建 Type 实例时使用（通过 `Base::get`）。区别在于我们可以提供此 `get` 方法的实现。使用这种构建器定义风格，只生成声明，`MyType` 的实现者需要提供 `MyType::get` 的定义。

第二个构建器将生成如下构建器方法的声明：

```tablegen
  let builders = [
    TypeBuilder<(ins CArg<"int", "0">:$intParam)>,
  ];
```

```c++
class MyType : /*...*/ {
  /*...*/
  static MyType get(::mlir::MLIRContext *context, int intParam = 0);
};
```

这里的约束与第一个构建器示例相同，只是 `intParam` 现在附加了默认值。

第三个构建器将生成如下构建器方法的声明：

```tablegen
  let builders = [
    TypeBuilder<(ins CArg<"int", "0">:$intParam), [{
      // 在此内联编写 `get` 构建器的主体。
      return Base::get($_ctxt, intParam);
    }]>,
  ];
```

```c++
class MyType : /*...*/ {
  /*...*/
  static MyType get(::mlir::MLIRContext *context, int intParam = 0);
};

MyType MyType::get(::mlir::MLIRContext *context, int intParam) {
  // 在此内联编写 `get` 构建器的主体。
  return Base::get(context, intParam);
}
```

这与第二个构建器示例相同。区别在于现在将使用提供的代码块作为主体自动生成构建器方法的定义。在内联指定主体时，可以使用 `$_ctxt` 访问 `MLIRContext *` 参数。

第四个构建器将生成如下构建器方法的声明：

```tablegen
  let builders = [
    TypeBuilderWithInferredContext<(ins "Type":$typeParam), [{
      // 此构建器表明它可以从参数推断 MLIRContext 实例。
      return Base::get(typeParam.getContext(), ...);
    }]>,
  ];
```

```c++
class MyType : /*...*/ {
  /*...*/
  static MyType get(Type typeParam);
};

MyType MyType::get(Type typeParam) {
  // 此构建器表明它可以从参数推断 MLIRContext 实例。
  return Base::get(typeParam.getContext(), ...);
}
```

在此构建器示例中，与第三个构建器示例的主要区别在于不再添加 `MLIRContext` 参数。这是因为构建器使用了 `TypeBuilderWithInferredContext`，这意味着不需要上下文参数，因为可以从构建器的参数中推断出来。

第五个构建器将生成带有自定义返回类型的构建器方法声明：

```tablegen
  let builders = [
    TypeBuilder<(ins "int":$intParam), [{}], "IntegerType">,
  ]
```

```c++
class MyType : /*...*/ {
  /*...*/
  static IntegerType get(::mlir::MLIRContext *context, int intParam);

};
```

这与前三个示例生成相同的构建器声明，但构建器的返回类型由用户指定，而不是属性或类型类。这对于定义在构建时可能折叠或规范化的属性和类型的构建器非常有用。

### 解析和打印

如果指定了助记符，`hasCustomAssemblyFormat` 和 `assemblyFormat` 字段可用于指定属性或类型的汇编格式。没有参数的属性和类型不需要使用这两个字段，在这种情况下，属性或类型的语法就是助记符。

对于每个方言，将创建两个"分派"函数：一个用于解析，一个用于打印。这些静态函数与类定义放在一起，具有以下函数签名：

```c++
static ParseResult generatedAttributeParser(DialectAsmParser& parser, StringRef *mnemonic, Type attrType, Attribute &result);
static LogicalResult generatedAttributePrinter(Attribute attr, DialectAsmPrinter& printer);

static ParseResult generatedTypeParser(DialectAsmParser& parser, StringRef *mnemonic, Type &result);
static LogicalResult generatedTypePrinter(Type type, DialectAsmPrinter& printer);
```

上述函数应分别添加到你的 `Dialect::printType` 和 `Dialect::parseType` 方法中，或者如果所有属性或类型都定义了助记符，则考虑使用 `useDefaultAttributePrinterParser` 和 `useDefaultTypePrinterParser` ODS 方言选项。

助记符、hasCustomAssemblyFormat 和 assemblyFormat 字段是可选的。如果没有定义任何字段，生成的代码将不包含任何解析或打印代码，并从上面的分派函数中省略属性或类型。在这种情况下，方言作者负责在相应的 `Dialect::parseAttribute`/`Dialect::printAttribute` 和 `Dialect::parseType`/`Dialect::printType` 方法中进行解析/打印。

#### 使用 `hasCustomAssemblyFormat`

在 ODS 中定义了助记符的属性和类型可以定义 `hasCustomAssemblyFormat`，以指定在 C++ 中定义的自定义解析器和打印器。设置为 `1` 时，将在 Attribute 或 Type 类上声明相应的 `parse` 和 `print` 方法供用户定义。

对于 Types，这些方法的形式为：

- `static Type MyType::parse(AsmParser &parser)`

- `void MyType::print(AsmPrinter &p) const`

对于 Attributes，这些方法的形式为：

- `static Attribute MyAttr::parse(AsmParser &parser, Type attrType)`

- `void MyAttr::print(AsmPrinter &p) const`

可以在自定义 `print` 方法中使用换行符和缩进。但是，上游 MLIR 方言不推荐也不允许多行 Types 或 Attributes。它们可以在自定义方言中使用，以提高灵活性和可读性，例如在多个嵌套 Types 和 Attributes 的情况下。

#### 使用 `assemblyFormat`

在 ODS 中定义了助记符的属性和类型可以定义 `assemblyFormat`，以声明式方式描述自定义解析器和打印器。汇编格式由字面量、变量和指令组成。

- 字面量是用反引号括起来的关键字或有效标点符号，例如 `` `keyword` `` 或 `` `<` ``。
- 变量是以美元符号为前缀的参数名，例如 `$param0`，捕获一个属性或类型参数。
- 指令是关键字后跟定义特殊解析器和打印器行为的可选参数列表。

```tablegen
// 带有汇编格式的示例类型。
def MyType : TypeDef<My_Dialect, "MyType"> {
  // 定义助记符以允许方言的解析器钩子调用到
  // 生成的解析器。
  let mnemonic = "my_type";

  // 定义两个参数，其 C++ 类型在字符串字面量中指示。
  let parameters = (ins "int":$count, "AffineMap":$map);

  // 定义汇编格式。用 `<` 和 `>` 括起格式，
  // 使 MLIR 打印器使用美化格式。
  let assemblyFormat = "`<` $count `,` `map` `=` $map `>`";
}
```

`MyType` 的声明式汇编格式在 IR 中产生如下格式：

```mlir
!my_dialect.my_type<42, map = affine_map<(i, j) -> (j, i)>>
```

##### 参数的解析和打印

对于许多基本参数类型，不需要额外的工作来定义这些参数的解析或打印方式。

- 任何参数的默认打印器是 `$_printer << $_self`，其中 `$_self` 是参数的 C++ 值，`$_printer` 是一个 `AsmPrinter`。
- 参数的默认解析器是 `FieldParser<$cppClass>::parse($_parser)`，其中 `$cppClass` 是参数的 C++ 类型，`$_parser` 是一个 `AsmParser`。

可以通过重载这些函数或在 ODS 参数类中定义 `parser` 和 `printer`，为其他 C++ 类型添加打印和解析行为。

重载示例：

```c++
using MyParameter = std::pair<int, int>;

AsmPrinter &operator<<(AsmPrinter &printer, MyParameter param) {
  printer << param.first << " * " << param.second;
}

template <> struct FieldParser<MyParameter> {
  static FailureOr<MyParameter> parse(AsmParser &parser) {
    int a, b;
    if (parser.parseInteger(a) || parser.parseStar() ||
        parser.parseInteger(b))
      return failure();
    return MyParameter(a, b);
  }
};
```

使用 ODS 参数类的示例：

```tablegen
def MyParameter : TypeParameter<"std::pair<int, int>", "pair of ints"> {
  let printer = [{ $_printer << $_self.first << " * " << $_self.second }];
  let parser = [{ [&] -> FailureOr<std::pair<int, int>> {
    int a, b;
    if ($_parser.parseInteger(a) || $_parser.parseStar() ||
        $_parser.parseInteger(b))
      return failure();
    return std::make_pair(a, b);
  }() }];
}
```

使用此参数且汇编格式为 `` `<` $myParam `>` `` 的类型在 IR 中如下所示：

```mlir
!my_dialect.my_type<42 * 24>
```

###### 非 POD 参数

不是普通旧数据（例如引用）的参数可能需要定义 `cppStorageType`，以便在将数据复制到分配器之前保存数据。例如，`StringRefParameter` 使用 `std::string` 作为其存储类型，而 `ArrayRefParameter` 使用 `SmallVector` 作为其存储类型。这些参数的解析器预期返回 `FailureOr<$cppStorageType>`。

要在 `cppStorageType` 和参数的 C++ 类型之间添加自定义转换，参数可以覆盖 `convertFromStorage`，默认值为 `"$_self"`（即尝试从 `cppStorageType` 进行隐式转换）。

###### 可选和默认值参数

可选参数可以从属性或类型的汇编格式中省略。当可选参数等于其默认值时将被省略。汇编格式中的可选参数可以通过设置 `defaultValue`（C++ 默认值的字符串）来指示。如果在解析期间没有遇到该参数的值，则将其设置为此默认值。如果参数等于其默认值，则不打印该参数。使用参数的 `comparator` 字段，但如果未指定，则使用相等运算符。

使用 `OptionalParameter` 时，默认值设置为 C++ 存储类型的默认构造值。例如，`Optional<int>` 将设置为 `std::nullopt`，`Attribute` 将设置为 `nullptr`。通过将这些参数与其"空"值进行比较来测试这些参数的存在性。

可选组是一组基于锚点存在与否可选打印的元素。只有可选参数或只捕获可选参数的指令才能在可选组中使用。放置锚点的组在其存在时打印，否则打印另一个。如果作为锚点使用的指令捕获了多个可选参数，则当任何捕获的参数存在时打印可选组。例如，只有当 `custom` 指令捕获至少一个可选参数时，它才能用作可选组锚点。

假设参数 `a` 是 `IntegerAttr`。

```
( `(` $a^ `)` ) : (`x`)?
```

在上述汇编格式中，如果 `a` 存在（非空），则打印为 `(5 : i32)`。如果不存在，则打印为 `x`。在可选组内使用的指令只有在所有捕获的参数也是可选时才被允许。

也可以使用 `DefaultValuedParameter` 指定可选参数，它指定当参数等于某个给定值时应省略该参数。

```tablegen
let parameters = (ins DefaultValuedParameter<"Optional<int>", "5">:$a)
let mnemonic = "default_valued";
let assemblyFormat = "(`<` $a^ `>`)?";
```

效果如下：

```mlir
!test.default_valued     // a = 5
!test.default_valued<10> // a = 10
```

对于可选的 `Attribute` 或 `Type` 参数，可以通过 `$_ctxt` 访问当前 MLIR 上下文。例如：

```tablegen
DefaultValuedParameter<"IntegerType", "IntegerType::get($_ctxt, 32)">
```

出现在参数声明列表中默认值参数*之前*的参数值可以作为替换使用。例如：

```tablegen
let parameters = (ins
  "IntegerAttr":$value,
  DefaultValuedParameter<"Type", "$value.getType()">:$type
);
```

###### 属性 Self 类型参数

属性在属性值本身的汇编格式之后可以选择性地有一个尾部类型。MLIR 在属性值上解析，并在将 `Type` 传递给方言解析器钩子之前可选地解析冒号类型。

```
dialect-attribute  ::= `#` dialect-namespace `<` attr-data `>`
                       (`:` type)?
                     | `#` alias-name pretty-dialect-sym-body? (`:` type)?
```

`AttributeSelfTypeParameter` 是一个由汇编格式生成器特殊处理的属性参数。只能指定一个这样的参数，其值来自尾部类型。此参数的默认值为 `NoneType::get($_ctxt)`。

但是，为了让 MLIR 打印类型，属性必须实现 `TypedAttrInterface`。例如：

```tablegen
// 此属性只有一个 self 类型参数。
def MyExternAttr : AttrDef<MyDialect, "MyExtern", [TypedAttrInterface]> {
  let parameters = (AttributeSelfTypeParameter<"">:$type);
  let mnemonic = "extern";
  let assemblyFormat = "";
}
```

此属性可以如下所示：

```mlir
#my_dialect.extern // none
#my_dialect.extern : i32
#my_dialect.extern : tensor<4xi32>
#my_dialect.extern : !my_dialect.my_type
```

##### 汇编格式指令

属性和类型汇编格式具有以下指令：

- `params`：捕获属性或类型的所有参数。
- `qualified`：标记参数以打印其前导方言和助记符。
- `struct`：为键值对列表生成"类结构体"解析器和打印器。
- `custom`：分派对用户定义解析器和打印器函数的调用。
- `ref`：在 custom 指令中，引用先前绑定的变量。

###### `params` 指令

此指令用于引用属性或类型的所有参数，属性 self 类型除外（它与普通参数分开处理）。作为顶级指令使用时，`params` 为以逗号分隔的参数列表生成解析器和打印器。例如：

```tablegen
def MyPairType : TypeDef<My_Dialect, "MyPairType"> {
  let parameters = (ins "int":$a, "int":$b);
  let mnemonic = "pair";
  let assemblyFormat = "`<` params `>`";
}
```

在 IR 中，此类型将显示为：

```mlir
!my_dialect.pair<42, 24>
```

`params` 指令也可以传递给其他指令，例如 `struct`，作为引用所有参数的参数，以代替显式列出所有参数变量。

###### `qualified` 指令

此指令可用于包装属性或类型参数，使其以完全限定的形式打印，即包含方言名称和助记符前缀。

例如：

```tablegen
def OuterType : TypeDef<My_Dialect, "MyOuterType"> {
  let parameters = (ins MyPairType:$inner);
  let mnemonic = "outer";
  let assemblyFormat = "`<` pair `:` $inner `>`";
}
def OuterQualifiedType : TypeDef<My_Dialect, "MyOuterQualifiedType"> {
  let parameters = (ins MyPairType:$inner);
  let mnemonic = "outer_qual";
  let assemblyFormat = "`<` pair `:` qualified($inner) `>`";
}
```

在 IR 中，类型将显示为：

```mlir
!my_dialect.outer<pair : <42, 24>>
!my_dialect.outer_qual<pair : !mydialect.pair<42, 24>>
```

如果存在可选参数，当它们不存在时不会在参数列表中打印。

###### `struct` 指令

`struct` 指令接受一个变量或指令列表进行捕获，将为以逗号分隔的键值对列表生成解析器和打印器。如果 `struct` 中包含可选参数，则可以省略它。变量按照在参数列表中指定的顺序打印，但**可以任意顺序解析**。例如：

```tablegen
def MyStructType : TypeDef<My_Dialect, "MyStructType"> {
  let parameters = (ins StringRefParameter<>:$sym_name,
                        "int":$a, "int":$b, "int":$c);
  let mnemonic = "struct";
  let assemblyFormat = "`<` $sym_name `->` struct($a, $b, $c) `>`";
}
```

在 IR 中，此类型可以以指令中捕获的参数的任意排列顺序出现。

```mlir
!my_dialect.struct<"foo" -> a = 1, b = 2, c = 3>
!my_dialect.struct<"foo" -> b = 2, c = 3, a = 1>
```

将 `params` 作为唯一参数传递给 `struct` 使指令捕获属性或类型的所有参数。对于相同的上述类型，汇编格式为 `` `<` struct(params) `>` `` 将产生：

```mlir
!my_dialect.struct<b = 2, sym_name = "foo", c = 3, a = 1>
```

参数打印的顺序是它们在属性或类型的 `parameter` 列表中声明的顺序。

传递 `custom<Foo>($variable)` 允许为封装的变量提供自定义打印器和解析器。有关如何定义打印器和解析器函数的更多信息，请查阅 [custom 和 ref 指令](#custom-and-ref-directive) 章节。注意，struct 指令中的 custom 指令只能封装单个变量。

###### `custom` 和 `ref` 指令

`custom` 指令用于分派对用户定义打印器和解析器函数的调用。例如，假设我们有以下类型：

```tablegen
let parameters = (ins "int":$foo, "int":$bar);
let assemblyFormat = "custom<Foo>($foo) custom<Bar>($bar, ref($foo))";
```

`custom` 指令 `custom<Foo>($foo)` 将在解析器和打印器中分别生成对以下函数的调用：

```c++
ParseResult parseFoo(AsmParser &parser, int &foo);
void printFoo(AsmPrinter &printer, int foo);
```

如你所见，默认情况下参数通过引用传入 parse 函数。这只有在 C++ 类型是默认可构造的情况下才可能。如果 C++ 类型不是默认可构造的，则参数被包装在 `FailureOr` 中。因此，给定以下定义：

```tablegen
let parameters = (ins "NotDefaultConstructible":$foobar);
let assemblyFormat = "custom<Fizz>($foobar)";
```

它将生成需要以下 `parseFizz` 签名的调用：

```c++
ParseResult parseFizz(AsmParser &parser, FailureOr<NotDefaultConstructible> &foobar);
```

先前绑定的变量可以通过将其包装在 `ref` 指令中作为参数传递给 `custom` 指令。在前面的示例中，`$foo` 由第一个指令绑定。第二个指令引用它，并期望以下打印器和解析器签名：

```c++
ParseResult parseBar(AsmParser &parser, int &bar, int foo);
void printBar(AsmPrinter &printer, int bar, int foo);
```

更复杂的 C++ 类型可以与 `custom` 指令一起使用。唯一的注意事项是解析器的参数必须使用参数的存储类型。例如，`StringRefParameter` 期望解析器和打印器签名为：

```c++
ParseResult parseStringParam(AsmParser &parser, std::string &value);
void printStringParam(AsmPrinter &printer, StringRef value);
```

如果自定义解析器返回失败或任何绑定参数之后具有失败值，则认为自定义解析器已失败。

C++ 代码的字符串可以用作 `custom` 指令参数。生成自定义解析器和打印器调用时，字符串作为函数参数粘贴。例如，可以用常量整数重用 `parseBar` 和 `printBar`：

```tablegen
let parameters = (ins "int":$bar);
let assemblyFormat = [{ custom<Bar>($foo, "1") }];
```

字符串被逐字粘贴，但会替换 `$_builder` 和 `$_ctxt`。字符串字面量可用于参数化自定义指令。

### 验证

如果设置了 `genVerifyDecl` 字段，将在类上生成额外的验证方法。

- `static LogicalResult verify(function_ref<InFlightDiagnostic()> emitError, parameters...)`

这些方法用于在构建时验证提供给属性或类型类的参数，并发出任何必要的诊断信息。此方法从属性或类型类的构建器自动调用。

- `AttrOrType getChecked(function_ref<InFlightDiagnostic()> emitError, parameters...)`

如[构建器](#Builders)章节所述，这些方法是可能失败的 `get` 构建器的伴侣。如果调用这些方法时 `verify` 调用失败，它们返回 nullptr 而不是断言。

### 存储类

上述章节中隐约提到了"存储类"（通常缩写为"storage"）的概念。存储类包含构建和唯一化属性或类型实例所需的所有数据。这些类是在 MLIRContext 中唯一化的"不朽"对象，被 `Attribute` 和 `Type` 类包装。每个 Attribute 或 Type 类都有一个对应的存储类，可以通过 protected 的 `getImpl()` 方法访问。

在大多数情况下，存储类是自动生成的，但如果需要，可以通过将 `genStorageClass` 字段设置为 0 来手动定义。名称和命名空间（默认为 `detail`）还可以通过 `storageClass` 和 `storageNamespace` 字段进行控制。

#### 定义存储类

用户定义的存储类必须遵循以下规则：

- 继承自 `AttributeStorage` 或 `TypeStorage` 的基本类型存储类。
- 定义一个类型别名 `KeyTy`，映射到唯一标识派生类型实例的类型。例如，这可以是所有存储参数的 `std::tuple`。
- 提供用于分配存储类新实例的构造方法。
  - `static Storage *construct(StorageAllocator &allocator, const KeyTy &key)`
- 提供存储实例与 `KeyTy` 之间的比较方法。
  - `bool operator==(const KeyTy &) const`
- 提供从传递给唯一化器（在构建 Attribute 或 Type 时）的参数列表生成 `KeyTy` 的方法。（注意：只有在 `KeyTy` 不能从这些参数默认构造时才需要）。
  - `static KeyTy getKey(Args...&& args)`
- 提供哈希 `KeyTy` 实例的方法。（注意：如果存在 `llvm::DenseMapInfo<KeyTy>` 特化，则不需要）。
  - `static llvm::hash_code hashKey(const KeyTy &)`
- 提供从存储类实例生成 `KeyTy` 的方法。
  - `static KeyTy getAsKey()`

让我们来看一个示例：

```c++
/// 这里我们为 ComplexType 定义一个存储类，它保存一个非零整数和一个整数类型。
struct ComplexTypeStorage : public TypeStorage {
  ComplexTypeStorage(unsigned nonZeroParam, Type integerType)
      : nonZeroParam(nonZeroParam), integerType(integerType) {}

  /// 此存储的哈希键是整数和类型参数的一对。
  using KeyTy = std::pair<unsigned, Type>;

  /// 定义键类型的比较函数。
  bool operator==(const KeyTy &key) const {
    return key == KeyTy(nonZeroParam, integerType);
  }

  /// 定义键类型的哈希函数。
  /// 注意：这不是必需的，因为 std::pair、unsigned 和 Type 都已有可用的哈希函数。
  static llvm::hash_code hashKey(const KeyTy &key) {
    return llvm::hash_combine(key.first, key.second);
  }

  /// 定义键类型的构造函数。
  /// 注意：这不是必需的，因为 KeyTy 可以直接用给定参数构造。
  static KeyTy getKey(unsigned nonZeroParam, Type integerType) {
    return KeyTy(nonZeroParam, integerType);
  }

  /// 定义创建此存储新实例的构造方法。
  static ComplexTypeStorage *construct(StorageAllocator &allocator, const KeyTy &key) {
    return new (allocator.allocate<ComplexTypeStorage>())
        ComplexTypeStorage(key.first, key.second);
  }

  /// 从此存储类构建键实例。
  KeyTy getAsKey() const {
    return KeyTy(nonZeroParam, integerType);
  }

  /// 存储类保存的参数数据。
  unsigned nonZeroParam;
  Type integerType;
};
```

### 可变属性和类型

属性和类型是在 MLIRContext 中唯一化的不可变对象。话虽如此，某些参数可以被视为"可变"的，并在构建后修改。可变参数应保留给在构建时无法合理初始化的参数。由于可变组件的存在，这些参数不参与属性或类型的唯一化。

TODO：可变参数目前在属性和类型的声明式规范中不受支持，因此需要在 C++ 中定义 Attribute 或 Type 类。

#### 定义可变存储

除了存储类的基本要求外，具有可变组件的实例还必须额外遵循以下规则：

- 可变组件不得参与存储 `KeyTy`。
- 提供用于修改现有存储实例的变更方法。此方法根据参数修改可变组件，对任何新动态分配的存储使用 `allocator`，并指示修改是否成功。
  - `LogicalResult mutate(StorageAllocator &allocator, Args ...&& args)`

让我们为递归类型定义一个简单的存储，其中类型由名称标识，可以包含另一种类型（包括其自身）。

```c++
/// 这里我们为 RecursiveType 定义一个存储类，由名称标识并包含另一种类型。
struct RecursiveTypeStorage : public TypeStorage {
  /// 类型通过名称唯一标识。注意，包含的类型
  /// 不是键的一部分。
  using KeyTy = StringRef;

  /// 从类型名称构建存储。显式将
  /// containedType 初始化为 nullptr，用作可变组件
  /// 尚未初始化的标记。
  RecursiveTypeStorage(StringRef name) : name(name), containedType(nullptr) {}

  /// 定义比较函数。
  bool operator==(const KeyTy &key) const { return key == name; }

  /// 定义创建存储新实例的构造方法。
  static RecursiveTypeStorage *construct(StorageAllocator &allocator,
                                         const KeyTy &key) {
    // 注意，键字符串被复制到分配器中，以确保它
    // 与存储本身一样长。
    return new (allocator.allocate<RecursiveTypeStorage>())
        RecursiveTypeStorage(allocator.copyInto(key));
  }

  /// 定义创建后更改类型的变更方法。在许多情况下，
  /// 我们只想设置可变组件一次并拒绝任何进一步的修改，
  /// 这可以通过从此函数返回 failure 来实现。
  LogicalResult mutate(StorageAllocator &, Type body) {
    // 如果包含的类型已经初始化，并且调用尝试更改它，
    // 则拒绝更改。
    if (containedType && containedType != body)
      return failure();

    // 成功更改主体。
    containedType = body;
    return success();
  }

  StringRef name;
  Type containedType;
};
```

#### 类型类定义

定义好存储类后，我们可以定义类型类本身。`Type::TypeBase` 提供了一个 `mutate` 方法，将其参数转发给存储的 `mutate` 方法，并确保变更安全发生。

```c++
class RecursiveType : public Type::TypeBase<RecursiveType, Type,
                                            RecursiveTypeStorage> {
public:
  /// 继承父类构造函数。
  using Base::Base;

  /// 创建 Recursive 类型实例。这只接受类型名称
  /// 并返回主体未初始化的类型。
  static RecursiveType get(MLIRContext *ctx, StringRef name) {
    // 调用基类获取此类型的唯一化实例。参数
    //（名称）在上下文之后传递。
    return Base::get(ctx, name);
  }

  /// 现在我们可以更改类型的可变组件。这是一个可以在
  /// 已存在的 RecursiveType 上调用的实例方法。
  void setBody(Type body) {
    // 调用基类来变更类型。
    LogicalResult result = Base::mutate(body);

    // 大多数类型期望变更总是成功的，但类型可以实现
    // 自定义逻辑来处理变更失败。
    assert(succeeded(result) &&
           "尝试更改已初始化类型的主体");

    // 在不带断言的构建中避免未使用变量警告。
    (void) result;
  }

  /// 返回包含的类型，如果尚未初始化则可能为 null。
  Type getBody() { return getImpl()->containedType; }

  /// 返回名称。
  StringRef getName() { return getImpl()->name; }
};
```

### 额外声明

声明式 Attribute 和 Type 定义尝试尽可能多地自动生成逻辑和方法。话虽如此，总会有无法覆盖的长尾情况。对于这些情况，可以使用 `extraClassDeclaration` 和 `extraClassDefinition`。`extraClassDeclaration` 字段中的代码将被字面复制到生成的 C++ Attribute 或 Type 类中。`extraClassDefinition` 中的代码将添加到生成的源文件中，位于类的 C++ 命名空间内。替换 `$cppClass` 将被替换为 Attribute 或 Type 的 C++ 类名。

注意，这些是为高级用户处理长尾情况的机制；对于尚未实现但广泛适用的情况，改进基础设施更为可取。

### 汇编中的助记符别名

属性和类型可以在汇编中使用别名来减少冗余。在这种情况下，可以使用 `OpAsmAttrInterface` 和 `OpAsmTypeInterface` 来生成别名。通常，简单的助记符别名就足够了；启用 `genMnemonicAlias` 会自动使用属性或类型的助记符生成 `getAlias` 实现。

### 向方言注册

一旦定义了属性和类型，就必须向父 `Dialect` 注册它们。这通过 `addAttributes` 和 `addTypes` 方法完成。注意，注册时必须能看到存储类的完整定义。

```c++
void MyDialect::initialize() {
    /// 向方言添加定义的属性。
  addAttributes<
#define GET_ATTRDEF_LIST
#include "MyDialect/Attributes.cpp.inc"
  >();

    /// 向方言添加定义的类型。
  addTypes<
#define GET_TYPEDEF_LIST
#include "MyDialect/Types.cpp.inc"
  >();
}
```
