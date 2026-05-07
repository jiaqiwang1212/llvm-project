# 自定义汇编行为

[TOC]

## 生成别名

`AsmPrinter` 在不以通用形式打印时，可以为频繁使用的类型和属性生成别名。例如，`!my_dialect.type<a=3,b=4,c=5,d=tuple,e=another_type>` 和 `#my_dialect.attr<a=3>` 可以分别别名为 `!my_dialect_type` 和 `#my_dialect_attr`。

挂入 `AsmPrinter` 主要有两种方式：一种是属性/类型接口，另一种是方言接口。

属性/类型接口是首先检查的钩子。如果未找到此类钩子，或钩子返回 `OverridableAlias`（见下方定义），则会涉及方言接口。

一个特定方言的方言接口可以为所有类型/属性生成别名，即使它不"拥有"它们。`AsmPrinter` 根据方言接口的注册顺序检查所有方言接口。例如，`builtin` 属性 `AffineMapAttr` 的默认别名 `map` 可以被 `my_dialect` 的方言接口覆盖，因为自定义方言通常在 `builtin` 方言之后注册。

```cpp
/// 保存 `OpAsm{Dialect,Attr,Type}Interface::getAlias` 钩子调用的结果。
enum class OpAsmAliasResult {
  /// 对象（类型或属性）不受钩子支持，
  /// 未提供别名。
  NoAlias,
  /// 提供了别名，但可能被其他钩子覆盖。
  OverridableAlias,
  /// 提供了别名，应使用该别名
  ///（不会检查其他钩子）。
  FinalAlias
};
```

如果多个类型/属性的 `getAlias` 钩子返回相同的别名，则会在别名后附加数字以避免冲突。

### `OpAsmDialectInterface`

```cpp
#include "mlir/IR/OpImplementation.h"

struct MyDialectOpAsmDialectInterface : public OpAsmDialectInterface {
 public:
  using OpAsmDialectInterface::OpAsmDialectInterface;

  AliasResult getAlias(Type type, raw_ostream& os) const override {
    if (mlir::isa<MyType>(type)) {
      os << "my_dialect_type";
      return AliasResult::FinalAlias;
    }
    return AliasResult::NoAlias;
  }

  AliasResult getAlias(Attribute attr, raw_ostream& os) const override {
    if (mlir::isa<MyAttribute>(attr)) {
      os << "my_dialect_attr";
      return AliasResult::FinalAlias;
    }
    return AliasResult::NoAlias;
  }
};

void MyDialect::initialize() {
  // 向方言注册接口
  addInterface<MyDialectOpAsmDialectInterface>();
}
```

### `OpAsmAttrInterface` 和 `OpAsmTypeInterface`

使用这些接口最简单的方式是在属性/别名的 tablegen 文件中切换 `genMnemonicAlias`。这直接使用助记符作为别名。详情请参阅[定义方言属性和类型](/docs/DefiningDialects/AttributesAndTypes)。

如果需要更自定义的行为，应对属性/类型进行以下修改：

1. 将 `OpAsmAttrInterface` 或 `OpAsmTypeInterface` 添加到其 trait 列表中。
2. 实现 `getAlias` 方法，可在 tablegen 中或其 cpp 文件中实现。

```tablegen
include "mlir/IR/OpAsmInterface.td"

// 添加 OpAsmAttrInterface trait
def MyAttr : MyDialect_Attr<"MyAttr",
         [ OpAsmAttrInterface ] > {

  // 此方法可以放入 cpp 文件中。
  let extraClassDeclaration = [{
    ::mlir::OpAsmAliasResult getAlias(::llvm::raw_ostream &os) const {
      os << "alias_name";
      return ::mlir::OpAsmAliasResult::OverridableAlias;
    }
  }];
}
```

## 建议 SSA/块名称

`Operation` 可以使用 `OpAsmOpInterface` 建议 SSA 名称前缀。

例如，`arith.constant` 将为其结果建议类似 `%c42_i32` 的名称：

```tablegen
include "mlir/IR/OpAsmInterface.td"

def Arith_ConstantOp : Op<Arith_Dialect, "constant",
    [ConstantLike, Pure,
     DeclareOpInterfaceMethods<OpAsmOpInterface, ["getAsmResultNames"]>]> {
...
}
```

以及对应的方法：

```cpp
// 来自 https://github.com/llvm/llvm-project/blob/5ce271ef74dd3325993c827f496e460ced41af11/mlir/lib/Dialect/Arith/IR/ArithOps.cpp#L184
void arith::ConstantOp::getAsmResultNames(
    function_ref<void(Value, StringRef)> setNameFn) {
  auto type = getType();
  if (auto intCst = llvm::dyn_cast<IntegerAttr>(getValue())) {
    auto intType = llvm::dyn_cast<IntegerType>(type);

    // 用 'true' 和 'false' 美化 i1 常量。
    if (intType && intType.getWidth() == 1)
      return setNameFn(getResult(), (intCst.getInt() ? "true" : "false"));

    // 否则，使用值和类型构建复杂名称。
    SmallString<32> specialNameBuffer;
    llvm::raw_svector_ostream specialName(specialNameBuffer);
    specialName << 'c' << intCst.getValue();
    if (intType)
      specialName << '_' << type;
    setNameFn(getResult(), specialName.str());
  } else {
    setNameFn(getResult(), "cst");
  }
}
```

类似地，`Operation` 可以使用 `OpAsmOpInterface` 中的 `getAsmBlockArgumentNames` 方法为其块参数建议名称。

对于自定义块名称，`OpAsmOpInterface` 有一个 `getAsmBlockNames` 方法，使操作可以建议自定义前缀，而不是通用的 `^bb0`。

另外，`OpAsmTypeInterface` 提供了一个 `getAsmName` 方法，适用于名称可以从其类型推断的场景。

## 定义默认方言

`Operation` 可以指示其中的嵌套区域具有默认方言前缀，区域中的操作可以省略方言前缀。

例如，在 `func.func` 操作中，所有 `func` 前缀都可以省略：

```tablegen
include "mlir/IR/OpAsmInterface.td"

def FuncOp : Func_Op<"func", [
  OpAsmOpInterface
  ...
]> {
  let extraClassDeclaration = [{
    /// 允许省略方言前缀。
    static StringRef getDefaultDialect() { return "func"; }
  }];
}
```

```mlir
func.func @main() {
  // 实际上是 func.call
  call @another()
}
```
