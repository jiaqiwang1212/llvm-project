# 第七章：向 Toy 添加复合类型

[TOC]

在[上一章](Ch-6.md)中，我们演示了从 Toy 前端到 LLVM IR 的端到端编译流程。在本章中，我们将扩展 Toy 语言，以支持一种新的复合 `struct` 类型。

## 在 Toy 中定义 `struct`

我们首先需要在 `toy` 源语言中定义该类型的接口。Toy 中 `struct` 类型的通用语法如下：

```toy
# A struct is defined by using the `struct` keyword followed by a name.
struct MyStruct {
  # Inside of the struct is a list of variable declarations without initializers
  # or shapes, which may also be other previously defined structs.
  var a;
  var b;
}
```

现在可以在函数中将结构体（struct）用作变量或参数，只需使用结构体的名称代替 `var`。结构体的成员通过 `.` 访问运算符访问。`struct` 类型的值可以用复合初始化器初始化，即用 `{}` 包围的、以逗号分隔的其他初始化器列表。示例如下：

```toy
struct Struct {
  var a;
  var b;
}

# User defined generic function may operate on struct types as well.
def multiply_transpose(Struct value) {
  # We can access the elements of a struct via the '.' operator.
  return transpose(value.a) * transpose(value.b);
}

def main() {
  # We initialize struct values using a composite initializer.
  Struct value = {[[1, 2, 3], [4, 5, 6]], [[1, 2, 3], [4, 5, 6]]};

  # We pass these arguments to functions like we do with variables.
  var c = multiply_transpose(value);
  print(c);
}
```

## 在 MLIR 中定义 `struct`

在 MLIR 中，我们同样需要为结构体类型提供一种表示。MLIR 没有提供完全符合我们需求的类型，因此我们需要定义自己的类型。我们将简单地把 `struct` 定义为一组元素类型的无名容器。`struct` 的名称及其元素仅对我们 `toy` 编译器的 AST 有用，因此我们不需要在 MLIR 表示中编码它。

### 定义类型类

#### 定义类型类

如[第二章](Ch-2.md)所述，MLIR 中的 [`Type`](../../LangRef.md/#type-system) 对象是值类型（value-typed）的，并依赖于一个内部存储对象来保存类型的实际数据。`Type` 类本身充当围绕内部 `TypeStorage` 对象的简单包装器，该对象在 `MLIRContext` 实例中是唯一的（uniqued）。构造 `Type` 时，我们在内部只是构造并唯一化一个存储类实例。

当定义包含参数化数据的新 `Type`（例如需要额外信息来保存元素类型的 `struct` 类型）时，我们需要提供一个派生存储类。不含任何附加数据的"单例"（singleton）类型（例如 [`index` 类型](../../Dialects/Builtin.md/#indextype)）不需要存储类，使用默认的 `TypeStorage` 即可。

##### 定义存储类

类型存储对象（Type storage object）包含构造和唯一化类型实例所需的所有数据。派生存储类必须继承自基类 `mlir::TypeStorage`，并提供一组别名和钩子（hook），供 `MLIRContext` 用于唯一化。以下是我们 `struct` 类型的存储实例定义，并逐一详细说明每项必要要求：

```c++
/// This class represents the internal storage of the Toy `StructType`.
struct StructTypeStorage : public mlir::TypeStorage {
  /// The `KeyTy` is a required type that provides an interface for the storage
  /// instance. This type will be used when uniquing an instance of the type
  /// storage. For our struct type, we will unique each instance structurally on
  /// the elements that it contains.
  using KeyTy = llvm::ArrayRef<mlir::Type>;

  /// A constructor for the type storage instance.
  StructTypeStorage(llvm::ArrayRef<mlir::Type> elementTypes)
      : elementTypes(elementTypes) {}

  /// Define the comparison function for the key type with the current storage
  /// instance. This is used when constructing a new instance to ensure that we
  /// haven't already uniqued an instance of the given key.
  bool operator==(const KeyTy &key) const { return key == elementTypes; }

  /// Define a hash function for the key type. This is used when uniquing
  /// instances of the storage.
  /// Note: This method isn't necessary as both llvm::ArrayRef and mlir::Type
  /// have hash functions available, so we could just omit this entirely.
  static llvm::hash_code hashKey(const KeyTy &key) {
    return llvm::hash_value(key);
  }

  /// Define a construction function for the key type from a set of parameters.
  /// These parameters will be provided when constructing the storage instance
  /// itself, see the `StructType::get` method further below.
  /// Note: This method isn't necessary because KeyTy can be directly
  /// constructed with the given parameters.
  static KeyTy getKey(llvm::ArrayRef<mlir::Type> elementTypes) {
    return KeyTy(elementTypes);
  }

  /// Define a construction method for creating a new instance of this storage.
  /// This method takes an instance of a storage allocator, and an instance of a
  /// `KeyTy`. The given allocator must be used for *all* necessary dynamic
  /// allocations used to create the type storage and its internal.
  static StructTypeStorage *construct(mlir::TypeStorageAllocator &allocator,
                                      const KeyTy &key) {
    // Copy the elements from the provided `KeyTy` into the allocator.
    llvm::ArrayRef<mlir::Type> elementTypes = allocator.copyInto(key);

    // Allocate the storage instance and construct it.
    return new (allocator.allocate<StructTypeStorage>())
        StructTypeStorage(elementTypes);
  }

  /// The following field contains the element types of the struct.
  llvm::ArrayRef<mlir::Type> elementTypes;
};
```

##### 定义类型类

存储类定义好之后，我们可以添加用户可见的 `StructType` 类的定义。这是我们实际进行交互的类。

```c++
/// This class defines the Toy struct type. It represents a collection of
/// element types. All derived types in MLIR must inherit from the CRTP class
/// 'Type::TypeBase'. It takes as template parameters the concrete type
/// (StructType), the base class to use (Type), and the storage class
/// (StructTypeStorage).
class StructType : public mlir::Type::TypeBase<StructType, mlir::Type,
                                               StructTypeStorage> {
public:
  /// Inherit some necessary constructors from 'TypeBase'.
  using Base::Base;

  /// Create an instance of a `StructType` with the given element types. There
  /// *must* be at least one element type.
  static StructType get(llvm::ArrayRef<mlir::Type> elementTypes) {
    assert(!elementTypes.empty() && "expected at least 1 element type");

    // Call into a helper 'get' method in 'TypeBase' to get a uniqued instance
    // of this type. The first parameter is the context to unique in. The
    // parameters after are forwarded to the storage instance.
    mlir::MLIRContext *ctx = elementTypes.front().getContext();
    return Base::get(ctx, elementTypes);
  }

  /// Returns the element types of this struct type.
  llvm::ArrayRef<mlir::Type> getElementTypes() {
    // 'getImpl' returns a pointer to the internal storage instance.
    return getImpl()->elementTypes;
  }

  /// Returns the number of element type held by this struct.
  size_t getNumElementTypes() { return getElementTypes().size(); }
};
```

我们以与注册操作类似的方式，在 `ToyDialect` 初始化器中注册此类型：

```c++
void ToyDialect::initialize() {
  addTypes<StructType>();
}
```

（这里有一个重要注意事项：注册类型时，存储类的定义必须可见。）

有了这些，我们现在可以在从 Toy 生成 MLIR 时使用 `StructType`。更多详情请参见 examples/toy/Ch7/mlir/MLIRGen.cpp。

### 向 ODS 暴露

定义新类型后，我们应该让 ODS（Operation Definition Specification，操作定义规范）框架了解我们的类型，以便在操作定义中使用它，并在方言中自动生成实用工具。下面是一个简单示例：

```tablegen
// Provide a definition for the Toy StructType for use in ODS. This allows for
// using StructType in a similar way to Tensor or MemRef. We use `DialectType`
// to demarcate the StructType as belonging to the Toy dialect.
def Toy_StructType :
    DialectType<Toy_Dialect, CPred<"isa<StructType>($_self)">,
                "Toy struct type">;

// Provide a definition of the types that are used within the Toy dialect.
def Toy_Type : AnyTypeOf<[F64Tensor, Toy_StructType]>;
```

### 解析与打印

此时我们可以在 MLIR 生成和转换期间使用 `StructType`，但还无法输出或解析 `.mlir`。为此我们需要添加对 `StructType` 实例的解析和打印支持。这可以通过在 `ToyDialect` 上重写 `parseType` 和 `printType` 方法来实现。当类型按照上一节所述向 ODS 暴露时，这些方法的声明会自动提供。

```c++
class ToyDialect : public mlir::Dialect {
public:
  /// Parse an instance of a type registered to the toy dialect.
  mlir::Type parseType(mlir::DialectAsmParser &parser) const override;

  /// Print an instance of a type registered to the toy dialect.
  void printType(mlir::Type type,
                 mlir::DialectAsmPrinter &printer) const override;
};
```

这些方法接受一个高级解析器或打印器的实例，可以方便地实现所需功能。在介绍实现之前，让我们先思考一下在打印的 IR 中 `struct` 类型的语法。如 [MLIR 语言参考](../../LangRef.md/#dialect-types)所述，方言类型通常表示为：`! dialect-namespace < type-data >`，在特定情况下也有美化形式（pretty form）可用。我们的 `Toy` 解析器和打印器的职责是提供 `type-data` 部分。我们将把 `StructType` 定义为以下形式：

```
  struct-type ::= `struct` `<` type (`,` type)* `>`
```

#### 解析

以下是解析器的一个实现：

```c++
/// Parse an instance of a type registered to the toy dialect.
mlir::Type ToyDialect::parseType(mlir::DialectAsmParser &parser) const {
  // Parse a struct type in the following form:
  //   struct-type ::= `struct` `<` type (`,` type)* `>`

  // NOTE: All MLIR parser function return a ParseResult. This is a
  // specialization of LogicalResult that auto-converts to a `true` boolean
  // value on failure to allow for chaining, but may be used with explicit
  // `mlir::failed/mlir::succeeded` as desired.

  // Parse: `struct` `<`
  if (parser.parseKeyword("struct") || parser.parseLess())
    return Type();

  // Parse the element types of the struct.
  SmallVector<mlir::Type, 1> elementTypes;
  do {
    // Parse the current element type.
    SMLoc typeLoc = parser.getCurrentLocation();
    mlir::Type elementType;
    if (parser.parseType(elementType))
      return nullptr;

    // Check that the type is either a TensorType or another StructType.
    if (!isa<mlir::TensorType, StructType>(elementType)) {
      parser.emitError(typeLoc, "element type for a struct must either "
                                "be a TensorType or a StructType, got: ")
          << elementType;
      return Type();
    }
    elementTypes.push_back(elementType);

    // Parse the optional: `,`
  } while (succeeded(parser.parseOptionalComma()));

  // Parse: `>`
  if (parser.parseGreater())
    return Type();
  return StructType::get(elementTypes);
}
```

#### 打印

以下是打印器的一个实现：

```c++
/// Print an instance of a type registered to the toy dialect.
void ToyDialect::printType(mlir::Type type,
                           mlir::DialectAsmPrinter &printer) const {
  // Currently the only toy type is a struct type.
  StructType structType = type.cast<StructType>();

  // Print the struct type according to the parser format.
  printer << "struct<";
  llvm::interleaveComma(structType.getElementTypes(), printer);
  printer << '>';
}
```

在继续之前，让我们看一个快速示例，展示我们现在拥有的功能：

```toy
struct Struct {
  var a;
  var b;
}

def multiply_transpose(Struct value) {
}
```

生成以下内容：

```mlir
module {
  toy.func @multiply_transpose(%arg0: !toy.struct<tensor<*xf64>, tensor<*xf64>>) {
    toy.return
  }
}
```

### 在 `StructType` 上进行操作

现在 `struct` 类型已经定义好，并且可以在 IR 中往返（round-trip）。下一步是添加在操作中使用它的支持。

#### 更新现有操作

我们的一些现有操作，例如 `ReturnOp`，需要更新以处理 `Toy_StructType`。

```tablegen
def ReturnOp : Toy_Op<"return", [Terminator, HasParent<"FuncOp">]> {
  ...
  let arguments = (ins Variadic<Toy_Type>:$input);
  ...
}
```

#### 添加新的 `Toy` 操作

除了现有操作之外，我们还将添加一些新操作，以更具体地处理 `struct`。

##### `toy.struct_constant`

这个新操作为结构体实例化一个常量值。在我们当前的建模中，我们只使用一个[数组属性](../../Dialects/Builtin.md/#arrayattr)（array attribute），其中包含每个 `struct` 元素的常量值集合。

```mlir
  %0 = toy.struct_constant [
    dense<[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]> : tensor<2x3xf64>
  ] : !toy.struct<tensor<*xf64>>
```

##### `toy.struct_access`

这个新操作实例化 `struct` 值的第 N 个元素。

```mlir
  // Using %0 from above
  %1 = toy.struct_access %0[0] : !toy.struct<tensor<*xf64>> -> tensor<*xf64>
```

有了这些操作，我们可以重新审视原始示例：

```toy
struct Struct {
  var a;
  var b;
}

# User defined generic function may operate on struct types as well.
def multiply_transpose(Struct value) {
  # We can access the elements of a struct via the '.' operator.
  return transpose(value.a) * transpose(value.b);
}

def main() {
  # We initialize struct values using a composite initializer.
  Struct value = {[[1, 2, 3], [4, 5, 6]], [[1, 2, 3], [4, 5, 6]]};

  # We pass these arguments to functions like we do with variables.
  var c = multiply_transpose(value);
  print(c);
}
```

最终得到完整的 MLIR 模块：

```mlir
module {
  toy.func @multiply_transpose(%arg0: !toy.struct<tensor<*xf64>, tensor<*xf64>>) -> tensor<*xf64> {
    %0 = toy.struct_access %arg0[0] : !toy.struct<tensor<*xf64>, tensor<*xf64>> -> tensor<*xf64>
    %1 = toy.transpose(%0 : tensor<*xf64>) to tensor<*xf64>
    %2 = toy.struct_access %arg0[1] : !toy.struct<tensor<*xf64>, tensor<*xf64>> -> tensor<*xf64>
    %3 = toy.transpose(%2 : tensor<*xf64>) to tensor<*xf64>
    %4 = toy.mul %1, %3 : tensor<*xf64>
    toy.return %4 : tensor<*xf64>
  }
  toy.func @main() {
    %0 = toy.struct_constant [
      dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>,
      dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
    ] : !toy.struct<tensor<*xf64>, tensor<*xf64>>
    %1 = toy.generic_call @multiply_transpose(%0) : (!toy.struct<tensor<*xf64>, tensor<*xf64>>) -> tensor<*xf64>
    toy.print %1 : tensor<*xf64>
    toy.return
  }
}
```

#### 优化 `StructType` 上的操作

现在我们有了一些在 `StructType` 上操作的操作，也就有了许多新的常量折叠（constant folding）机会。

内联（inlining）之后，上一节中的 MLIR 模块看起来类似于：

```mlir
module {
  toy.func @main() {
    %0 = toy.struct_constant [
      dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>,
      dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
    ] : !toy.struct<tensor<*xf64>, tensor<*xf64>>
    %1 = toy.struct_access %0[0] : !toy.struct<tensor<*xf64>, tensor<*xf64>> -> tensor<*xf64>
    %2 = toy.transpose(%1 : tensor<*xf64>) to tensor<*xf64>
    %3 = toy.struct_access %0[1] : !toy.struct<tensor<*xf64>, tensor<*xf64>> -> tensor<*xf64>
    %4 = toy.transpose(%3 : tensor<*xf64>) to tensor<*xf64>
    %5 = toy.mul %2, %4 : tensor<*xf64>
    toy.print %5 : tensor<*xf64>
    toy.return
  }
}
```

我们有多个 `toy.struct_access` 操作访问 `toy.struct_constant`。如[第三章](Ch-3.md)（FoldConstantReshape）所述，我们可以通过在操作定义中设置 `hasFolder` 位并提供 `*Op::fold` 方法的定义，为这些 `toy` 操作添加折叠器（folder）。

```c++
/// Fold constants.
OpFoldResult ConstantOp::fold(FoldAdaptor adaptor) { return value(); }

/// Fold struct constants.
OpFoldResult StructConstantOp::fold(FoldAdaptor adaptor) {
  return value();
}

/// Fold simple struct access operations that access into a constant.
OpFoldResult StructAccessOp::fold(FoldAdaptor adaptor) {
  auto structAttr = dyn_cast_or_null<mlir::ArrayAttr>(adaptor.getInput());
  if (!structAttr)
    return nullptr;

  size_t elementIndex = index().getZExtValue();
  return structAttr[elementIndex];
}
```

为了确保 MLIR 在折叠我们的 `Toy` 操作时生成正确的常量操作，即对 `TensorType` 生成 `ConstantOp`，对 `StructType` 生成 `StructConstant`，我们需要为方言钩子 `materializeConstant` 提供重写。这允许通用 MLIR 操作在必要时为 `Toy` 方言创建常量。

```c++
mlir::Operation *ToyDialect::materializeConstant(mlir::OpBuilder &builder,
                                                 mlir::Attribute value,
                                                 mlir::Type type,
                                                 mlir::Location loc) {
  if (isa<StructType>(type))
    return StructConstantOp::create(builder, loc, type,
                                            cast<mlir::ArrayAttr>(value));
  return ConstantOp::create(builder, loc, type,
                                    cast<mlir::DenseElementsAttr>(value));
}
```

有了这些，我们现在可以生成无需对流水线做任何更改即可下降到 LLVM 的代码。

```mlir
module {
  toy.func @main() {
    %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
    %1 = toy.transpose(%0 : tensor<2x3xf64>) to tensor<3x2xf64>
    %2 = toy.mul %1, %1 : tensor<3x2xf64>
    toy.print %2 : tensor<3x2xf64>
    toy.return
  }
}
```

你可以构建 `toyc-ch7` 并亲自尝试：`toyc-ch7 test/Examples/Toy/Ch7/struct-codegen.toy -emit=mlir`。有关定义自定义类型的更多详情，请参阅 [DefiningAttributesAndTypes](../../DefiningDialects/AttributesAndTypes.md)。
