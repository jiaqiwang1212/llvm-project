# MLIR C API

**当前状态：开发中，API 不稳定，默认构建。**

[TOC]

## 设计

许多语言可以与 C 进行互操作，但由于名称修饰（name mangling）和内存模型的差异，与 C++ 的互操作往往更为困难。尽管 MLIR 的 C API 可以直接在 C 中使用，但其主要目的是封装在更高层次的特定语言或库构造中。因此，该 API 倾向于简洁性和功能极简主义。

**注意：** 虽然 C API 预期比 C++ API 更为稳定，但目前不提供任何稳定性保证。

### 范围

该 API 为核心 IR 组件（属性、块、操作、region、类型、值）、Pass 以及一些基本的类型和属性种类提供支持。核心 IR API 有意保持低层次，例如，它暴露操作数和属性的普通列表，而不尝试为其赋予"语义"名称。特定方言的用户应以方言特定的方式封装核心 API，例如通过实现 ODS 后端。

### 对象模型

核心 IR 组件作为 C++ 中存在的 IR 对象的不透明_句柄_（handles）暴露。API 用户不打算检查这些句柄（在许多情况下，也无法有意义地检查）。用户应将句柄传递给适当的操作函数。

句柄_可能拥有也可能不拥有_底层对象。

### 命名约定和所有权模型

所有对象都以 `Mlir` 为前缀。它们是 typedef，使用时不加 `struct`。

所有函数都以 `mlir` 为前缀。

主要操作 `MlirX` 实例的函数以 `mlirX` 为前缀。它们将被操作的实例作为第一个参数（创建函数除外）。例如，`mlirOperationGetNumOperands` 检查 `MlirOperation`，并将其作为第一个操作数。

_所有权_模型在命名约定中编码如下：

-   默认情况下，所有权不转移。
-   将结果所有权转移给调用者的函数有以下两种形式之一：
    *   创建新对象的函数命名为 `mlirXCreate<...>`，例如 `mlirOperationCreate`；
    *   将对象从父对象分离的函数命名为 `mlirYTake<...>`，例如 `mlirOperationStateTakeRegion`。
-   获取某些参数所有权的函数形式为 `mlirY<...>OwnedX<...>`，其中 `X` 可以指代类型或其他足够唯一描述该参数的名称，该参数的所有权将由被调用者获取，例如 `mlirRegionAppendOwnedBlock`。
-   默认情况下不将所有权转移给调用者的对象创建函数（即作为参数传入的其他对象之一保留所有权）形式为 `mlirX<...>Get`。例如，`mlirTypeParseGet`。
-   销毁调用者所拥有的对象的函数形式为 `mlirXDestroy`。

如果代码拥有一个对象，则负责在不再需要该对象时销毁它。如果拥有其他对象的对象被销毁，则指向这些对象的任何句柄都将失效。请注意，类型和属性由创建它们的 `MlirContext` 所拥有。

### 空值

句柄可能指向_空_对象。调用者有责任通过使用 `mlirXIsNull(MlirX)` 检查对象是否为空。API 函数除非明确说明，否则_不_期望空对象作为参数。API 函数_可能_返回空对象。

### 类型层次结构

MLIR 对象在 C++ 中可以形成类型层次结构。例如，所有表示类型的 IR 类都派生自 `mlir::Type`，其中一些还可以派生自公共基类，例如 `mlir::ShapedType` 或特定方言的基类。类型层次结构通过以下命名约定暴露给 C API：

-   每个层次结构中只暴露顶层类，例如 `MlirType` 被定义为类型，但 `MlirShapedType` 不被定义。这避免了在将派生类型的对象传递给期望基类型的函数时需要显式向上转型（在核心/标准 API 中这种情况更为常见，而向下转型通常涉及进一步的检查）。
-   派生自 `X` 的类型 `Y` 提供函数 `int mlirXIsAY(MlirX)`，如果给定的 `X` 动态实例也是 `Y` 的实例，则返回非零值。例如，`int MlirTypeIsAInteger(MlirType)`。
-   期望派生类型作为第一个参数的函数改为接受基类型，并在名称中使用 `Y` 来记录期望：`MlirY<...>(MlirX, ...)`。此函数断言其第一个参数的动态实例是 `Y`，调用者有责任确保确实如此。

### 辅助类型

#### `StringRef`

许多 MLIR 函数返回 `StringRef` 实例，以引用字符串中不拥有所有权的片段。该片段不一定以空字符结尾。在 C API 中，这些被表示为 `MlirStringRef` 结构的实例，该结构包含指向字符串片段第一个字符的指针（`str`）和片段长度（`length`）。请注意，该片段_不一定_以空字符结尾，必须使用 `length` 字段来确定最后一个字符。`MlirStringRef` 是一个不拥有所有权的指针，调用者负责执行复制或确保被指向的内容在 `MlirStringRef` 的所有使用期间都有效。

### 打印

IR 对象可以使用 `mlirXPrint(MlirX, MlirStringCallback, void *)` 函数打印。这些函数接受签名为 `void (*)(const char *, intptr_t, void *)` 的回调和指向用户定义数据的指针作为参数。它们调用回调，并将字符串表示的块（以指向第一个字符的指针和长度的形式提供）以及未经修改的用户定义数据传递给回调。分配内存（如果必须存储字符串表示）和执行复制由调用者负责。不保证提供给回调的指针指向以空字符结尾的字符串，应使用 size 参数来确定字符串的结尾。回调可能会被多次调用，每次携带字符串表示的连续块（打印本身是缓冲的）。

*理由*：这种方法允许调用者完全控制分配，并避免打印器内部不必要的分配和复制。

为方便起见，提供了 `mlirXDump(MlirX)` 函数，用于将给定对象打印到标准错误流。

## 常用模式

该 API 为 MLIR 中的重复功能采用以下模式。

### 索引组件

如果一个对象的字段可以使用从零开始的连续整数索引访问（通常是数组），则该对象具有_索引组件_。例如，`MlirBlock` 将其参数作为索引组件。一个对象可以有多个这样的组件。例如，`MlirOperation` 具有属性、操作数、region、结果和后继块。

对于索引组件，提供以下一对函数：

-   `intptr_t mlirXGetNum<Y>s(MlirX)` 返回索引的上界。
-   `MlirY mlirXGet<Y>(MlirX, intptr_t pos)` 返回第 'pos' 个子对象。

大小以有符号指针大小的整数（即 `intptr_t`）接受和返回。此 typedef 在 C99 中可用。

请注意，函数中子对象的名称不一定与子对象的类型匹配。例如，`mlirOperationGetOperand` 返回 `MlirValue`。

### 可迭代组件

如果一个对象具有按某种顺序（而非整数索引）访问其字段的迭代器（通常是链表），则该对象具有_可迭代组件_。例如，`MlirBlock` 具有它所包含的操作的可迭代列表。一个对象可以有多个可迭代组件。

对于可迭代组件，提供以下三元组函数：

-   `MlirY mlirXGetFirst<Y>(MlirX)` 返回列表中的第一个子对象。
-   `MlirY mlirYGetNextIn<X>(MlirY)` 返回包含给定对象的列表中的下一个子对象，如果给定对象是列表中的最后一个，则返回空对象。
-   `int mlirYIsNull(MlirY)` 如果给定对象为空，则返回 1。

请注意，函数中子对象的名称可能与其类型匹配，也可能不匹配。

这种方法使得可以按如下方式进行迭代：

```c++
MlirY iter;
for (iter = mlirXGetFirst<Y>(x); !mlirYIsNull(iter);
     iter = mlirYGetNextIn<X>(iter)) {
  /* 使用 'iter'。 */
}
```

## 扩展 API

### 方言属性和类型的扩展

方言属性和类型可以参照内置属性和类型的示例，前提是实现位于单独的目录中，即 `include/mlir-c/<...>Dialect/` 和 `lib/CAPI/<...>Dialect/`。核心 API 在 `include/mlir/CAPI/IR` 中提供了实现私有的头文件，允许在核心 IR 组件的不透明 C 结构体与其 C++ 对应物之间进行转换。`wrap` 将 C++ 类转换为 C 结构体，`unwrap` 执行逆向转换。一旦 C++ 对象可用，API 实现应依赖 `isa` 来实现 `mlirXIsAY`，并在其他 API 调用中使用 `cast`。

### 接口的扩展

接口可以参照 IR 接口的示例，并应放置在适当的库中（例如，公共接口放在 `mlir-c/Interfaces` 中，特定方言的接口放在其方言库中）。与其他类型层次结构类似，接口不期望拥有自己类型的对象，而是操作顶层对象：`MlirAttribute`、`MlirOperation` 和 `MlirType`。静态接口方法预期将类的规范标识符作为首个参数，对于操作使用名称的 `MlirStringRef`，对于属性和类型使用 `MlirTypeID`，后跟接口注册所在的 `MlirContext`。

各个接口预期提供 `mlir<InterfaceName>TypeID()` 函数，该函数可用于分别通过 `mlir<Attribute/Operation/Type>ImplementsInterface` 或 `mlir<Attribute/Operation?Type>ImplementsInterfaceStatic` 函数来检查对象或类是否实现了此接口。理由：C++ 的 `isa` 只有在对象存在时才有效，静态方法通常通过模板分派；在 `MLIRContext` 中通过 `TypeID` 查找即使没有对象也能工作。
