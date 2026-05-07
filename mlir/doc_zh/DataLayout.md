# 数据布局建模

数据布局信息允许编译器回答与特定类型的值如何存储在内存中相关的问题。例如，值的大小或其地址对齐要求。它可以实现（但不限于）为抽象类型容器生成各种线性内存寻址方案，以及对向量进行更深入的推理。

数据布局子系统被设计为可扩展到 MLIR 的开放类型和操作系统。在顶层，它由以下部分组成：

*   可由具体数据布局规范实现的属性接口；
*   应由受数据布局约束的类型实现的类型接口；
*   必须由可充当数据布局作用域的操作（例如模块）实现的操作接口；
*   以及用于与特定类型无关的数据布局属性的方言接口。

内置类型被特殊处理，以降低整体查询开销。类似地，内置 `ModuleOp` 无需通过接口即可支持数据布局。

[TOC]

## 用法

### 作用域

遵循 MLIR 的嵌套结构，数据布局属性被_限定作用域_到属于实现了 `DataLayoutOpInterface` 的操作或 `ModuleOp` 操作的 region 中。这类作用域操作部分控制数据布局属性，并可能拥有影响这些属性的属性，通常组织为数据布局规范。

类型在不同作用域中可能具有不同的数据布局，包括嵌套在其他作用域中的作用域（例如包含在其他模块中的模块）。同时，在给定作用域（不包括任何嵌套作用域）内，给定类型具有固定的数据布局属性。类型还应具有默认的"自然"数据布局，以防它们在任何提供数据布局作用域的操作之外被使用。这确保了数据布局查询始终有有效结果。

### 兼容性与转换

计算布局属性所需的信息可以从嵌套作用域中合并。例如，外层作用域可以为类型子集定义布局属性，而内层作用域为不相交的子集定义，或者作用域可以逐步放宽某个类型的对齐要求。这种机制通过数据布局_兼容性_的概念来支持：嵌套作用域中定义的布局预期与外层作用域的布局兼容。MLIR 不规定具体的 op 和类型的兼容性含义，但为它们提供钩子以提供目标和类型特定的检查。例如，可以只允许在嵌套模块中放宽对齐约束（即，较小的对齐），或者，也可以要求嵌套模块完全重新定义外层作用域的所有约束。

数据布局兼容性在 IR 转换期间也很重要。任何影响数据布局作用域操作的转换都应维持数据布局兼容性。确保确实如此是转换的责任。

### 查询

数据布局属性查询可以在特殊对象——`DataLayout`——上执行，该对象可以为给定的作用域操作创建。这些对象允许与数据布局基础设施进行交互，并在对象作用域内查询给定类型的属性。`DataLayout` 类的签名如下：

```c++
class DataLayout {
public:
  explicit DataLayout(DataLayoutOpInterface scope);

  llvm::TypeSize getTypeSize(Type type) const;
  llvm::TypeSize getTypeSizeInBits(Type type) const;
  uint64_t getTypeABIAlignment(Type type) const;
  uint64_t getTypePreferredAlignment(Type type) const;
  std::optional<uint64_t> getTypeIndexBitwidth(Type type) const;
};
```

用户可以为感兴趣的作用域构造 `DataLayout` 对象。由于数据布局属性在作用域内是固定的，它们只会在第一次请求时计算一次，并被缓存以供后续使用。因此，`DataLayout(op.getParentOfType<DataLayoutOpInterface>()).getTypeSize(type)` 被认为是一种反模式，因为它在使用后丢弃了缓存。由于缓存的原因，只要封闭作用域的数据布局属性保持不变（即只要没有任何祖先操作以影响数据布局的方式被修改），`DataLayout` 对象就能返回有效结果。在这样的修改之后，用户应该创建一个新的 `DataLayout` 对象。为了提供帮助，如果 MLIR 在启用断言的情况下编译，`DataLayout` 会断言作用域保持不变。

## 自定义实现

通过一组 MLIR [接口](Interfaces.md)提供数据布局建模的可扩展性。

### 数据布局规范

数据布局规范是一个[属性](LangRef.md/#attributes)，概念上是称为数据布局规范_条目_的键值对集合。数据布局规范属性实现了 `DataLayoutSpecInterface`，如下所述。每个条目本身是一个实现了 `DataLayoutEntryInterface` 的属性。条目有一个键（`Type` 或 `StringAttr`）和一个值。键用于将条目与特定类型或方言关联：在处理数据布局属性请求时，类型或方言只能看到与之相关的规范条目，并且必须通过提供的 `DataLayout` 对象进行任何递归查询。这支持并强制了更好的可组合性，因为类型不能（也不应该）理解其他类型的布局细节。条目值是任意属性，特定于该类型。

例如，数据布局规范可以是一个实际的键值对列表，具有类似以下的简单自定义语法：

```mlir
#my_dialect.layout_spec<
  #my_dialect.layout_entry<!my_dialect.type, size=42>,
  #my_dialect.layout_entry<"my_dialect.endianness", "little">,
  #my_dialect.layout_entry<!my_dialect.vector, prefer_large_alignment>>
```

规范和条目属性的确切细节以及它们的语法由实现决定。

我们在整个数据布局子系统中使用_类型类_的概念。它对应于给定类型的 C++ 类，例如内置整数的 `IntegerType`。MLIR 没有在 IR 中表示类型类的机制。相反，数据布局条目包含类型类的特定_实例_，例如 `IntegerType{signedness=signless, bitwidth=8}`（在 IR 中为 `i8`）或 `IntegerType{signedness=unsigned, bitwidth=32}`（在 IR 中为 `ui32`）。在处理数据布局属性查询时，类型类将被提供属于该类型类的所有键的条目。例如，`IntegerType` 将看到 `i8`、`si16` 和 `ui32` 的条目，但_不会_看到 `f32` 或 `memref<?xi32>` 的条目（`MemRefType` 也不会看到 `i32` 的条目）。这允许类型特定的"插值"行为，其中类型类可以根据其他实例的属性计算_任意_特定类型实例的数据布局属性。再次以整数为例，它们的对齐可以通过取最接近（从上方）的 2 的幂次方位宽的整数类型的对齐来计算。

[include "Interfaces/DataLayoutAttrInterface.md"]

### 数据布局作用域操作

为数据布局查询定义作用域、并可用于创建 `DataLayout` 对象的操作，应实现 `DataLayoutOpInterface`。此类 op 必须至少提供一种获取数据布局规范的方式。规范不必作为属性附加到操作上，可以动态构建；每个 `DataLayout` 对象只获取一次规范并缓存。此类 op 还可以为数据布局查询提供自定义处理程序，以目标或作用域特定的方式提供结果，而无需将查询转发到特定类型或对类型返回的结果进行后处理。这些自定义处理程序使作用域操作能够（重新）定义类型的数据布局属性，而无需修改类型本身，例如，当类型在另一个方言中定义时。

[include "Interfaces/DataLayoutOpInterface.md"]

### 具有数据布局的类型

打算自行处理数据布局查询的类型类应实现 `DataLayoutTypeInterface`。此接口为每个数据布局查询提供可重写的钩子。每个钩子都提供类型实例、适合递归查询的 `DataLayout` 对象，以及与该类型类相关的数据布局查询列表。即使条目列表为空，也应提供有效结果。这些钩子无法访问处理查询所在作用域的操作，应改为使用提供的条目。

[include "Interfaces/DataLayoutTypeInterface.md"]

### 具有数据布局标识符的方言

对于与特定类型类无关的数据布局条目，条目的键是属于某个方言的标识符。在这种情况下，方言应实现 `DataLayoutDialectInterface`。此方言提供用于验证条目值属性有效性以及嵌套条目兼容性的钩子。

### 位与字节

为大小提供了两个版本的钩子：以位为单位和以字节为单位。以字节为单位的版本有一个默认实现，通过将以位为单位的大小除以 8 向上取整来得出以字节为单位的大小。专门针对具有不同假设的架构的类型可以重写此行为。操作可以为所有类型重新定义这一行为，为字节大小不是 8 的情况提供作用域版本，而无需修改类型（包括内置类型）。

### 查询分发

数据布局属性查询的整体流程如下：

1.  用户在给定作用域构造 `DataLayout`。构造函数获取数据布局规范并将其与封闭作用域的规范合并（布局应兼容）。
2.  用户调用 `DataLayout::query(Type ty)`。
3.  如果 `DataLayout` 有缓存的响应，立即返回该响应。
4.  否则，`DataLayout` 将查询转交给最近的布局作用域操作。如果它实现了 `DataLayoutOpInterface`，则查询被转发到 `DataLayoutOpInterface::query(ty, *this, relevantEntries)`，其中相关条目按上述方式计算。如果它没有实现 `DataLayoutOpInterface`，则它必须是 `ModuleOp`，且在将 `ty` 转换为类型接口后，查询被转发到 `DataLayoutTypeInterface::query(dataLayout, relevantEntries)`。
5.  除非 op 接口重新实现了 `query` 钩子，否则查询在将 `ty` 转换为类型接口后，进一步转发到 `DataLayoutTypeInterface::query(dataLayout, relevantEntries)`。如果类型没有实现该接口，则会产生不可恢复的致命错误。
6.  类型应始终提供响应，该响应沿调用栈返回并由 `DataLayout` 缓存。

## 默认实现

数据布局接口的默认实现直接处理一部分内置类型的查询。

### 内置模块

内置 `ModuleOp` 最多允许一个实现了 `DataLayoutSpecInterface` 的属性。出于效率和分层原因，它没有实现完整接口。相反，可以为 `ModuleOp` 构造 `DataLayout`，并与其他实现该接口的操作一起透明地处理模块。

### 内置类型

以下描述了内置类型的默认属性。

内置整数和浮点数的字节大小计算为 `ceildiv(bitwidth, 8)`。位宽低于 64 的整数类型和浮点类型的 ABI 对齐是最近的（从上方）2 的幂次方字节数。位宽为 64 及以上的整数类型的 ABI 对齐为 4 字节（32 位）。

内置向量的大小计算方式是：首先将其_最内层_维度的元素数量向上取整到最近的 2 的幂次方，然后获取元素总数，最后乘以元素大小。例如，`vector<3xi32>` 和 `vector<4xi32>` 具有相同的大小，`vector<2x3xf32>` 和 `vector<2x4xf32>` 也是如此，但 `vector<3x4xf32>` 和 `vector<4x4xf32>` 具有不同的大小。向量类型的 ABI 和首选对齐通过取向量最内层维度、向上取整到最近的 2 的幂次方、乘以元素字节大小，然后再次向上取整到最近的 2 的幂次方来计算。

注意：这些值的选择是为了与 [LLVM 中的默认数据布局](https://llvm.org/docs/LangRef.html#data-layout)保持一致（MLIR 在引入适当的数据布局建模之前一直假定该布局），以及与 [n 维向量建模](https://mlir.llvm.org/docs/Dialects/Vector/#deeperdive)保持一致。它们**未来可能会发生变化**。

#### `index` 类型

index 类型是一种整数类型，用于 `memref` 操作等中的目标特定大小信息。其数据布局由单个整数数据布局条目参数化，该条目指定其位宽。例如：

```mlir
module attributes { dlti.dl_spec = #dlti.dl_spec<
  #dlti.dl_entry<index, 32>
>} {}
```

指定 `index` 有 32 位，并且索引计算也应以 32 位精度执行。`index` 的所有其他布局属性与上面定义的相同位宽的整数类型的属性相同。

在没有相应条目的情况下，`index` 被假定为 64 位整数。

#### `complex` 类型

默认情况下，complex 类型被视为其给定元素类型的 2 元素结构体。也就是说，其每个元素都与其首选对齐对齐，整个 complex 类型也与此首选对齐对齐，complex 类型的大小包括强制对齐时元素之间可能存在的填充。

### 字节大小

默认数据布局假定 8 位字节。

### DLTI 方言

[DLTI](../Dialects/DLTIDialect/) 方言提供实现 `DataLayoutSpecInterface` 和 `DataLayoutEntryInterface` 的属性，以及可用于将规范附加到给定操作的方言属性。此属性的验证器会触发规范的验证器，并检查嵌套规范的兼容性。
