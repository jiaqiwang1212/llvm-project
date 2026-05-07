# 约束

[TOC]

## 属性/类型约束

在 TableGen 中定义操作的参数时，用户可以指定普通的属性/类型，或者使用属性/类型约束对属性值或操作数类型施加额外的要求。

```tablegen
def My_Type1 : MyDialect_Type<"Type1", "type1"> { ... }
def My_Type2 : MyDialect_Type<"Type2", "type2"> { ... }

// Plain type
let arguments = (ins MyType1:$val);
// Type constraint
let arguments = (ins AnyTypeOf<[MyType1, MyType2]>:$val);
```

`AnyTypeOf` 是类型约束的一个示例。许多有用的类型约束可以在 `mlir/IR/CommonTypeConstraints.td` 中找到。为类型/属性约束生成额外的验证代码。类型约束不仅可以在定义操作参数时使用，也可以在定义类型参数时使用。

可以选择生成 C++ 函数，以便可以从 C++ 检查类型/属性约束。C++ 函数的名称必须在 `cppFunctionName` 字段中指定。如果未指定函数名，则不生成 C++ 函数。

```tablegen
// Example: Element type constraint for VectorType
def Builtin_VectorTypeElementType : AnyTypeOf<[AnyInteger, Index, AnyFloat]> {
  let cppFunctionName = "isValidVectorTypeElementType";
}
```

上面的示例转换为以下 C++ 代码：
```c++
bool isValidVectorTypeElementType(::mlir::Type type) {
  return (((::llvm::isa<::mlir::IntegerType>(type))) || ((::llvm::isa<::mlir::IndexType>(type))) || ((::llvm::isa<::mlir::FloatType>(type))));
}
```

需要一个额外的 TableGen 规则来为类型/属性约束生成 C++ 代码。这只会生成指定 `.td` 文件中定义的类型/属性约束的声明/定义，而不包括包含的 `.td` 文件中的那些。

```cmake
mlir_tablegen(<Your Dialect>TypeConstraints.h.inc -gen-type-constraint-decls)
mlir_tablegen(<Your Dialect>TypeConstraints.cpp.inc -gen-type-constraint-defs)
mlir_tablegen(<Your Dialect>AttrConstraints.h.inc -gen-attr-constraint-decls)
mlir_tablegen(<Your Dialect>AttrConstraints.cpp.inc -gen-attr-constraint-defs)
```

生成的 `<Your Dialect>TypeConstraints.h.inc` 以及 `<Your Dialect>AttrConstraints.h.inc` 需要在 C++ 中引用类型/属性约束的任何地方包含。请注意，代码生成器不会生成 C++ 命名空间。`.h.inc`/`.cpp.inc` 文件的 `#include` 语句应由用户包装在 C++ 命名空间中。
