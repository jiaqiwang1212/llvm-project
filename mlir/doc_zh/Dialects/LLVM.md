# 'llvm' 方言

该方言通过定义相应的操作和类型，将 [LLVM IR](https://llvm.org/docs/LangRef.html) 映射到 MLIR 中。LLVM IR 的元数据通常表示为 MLIR 属性，从而提供额外的结构验证。

我们使用"LLVM IR"来指代 [LLVM 的中间表示](https://llvm.org/docs/LangRef.html)，使用"LLVM _方言_"或"LLVM IR _方言_"来指代这个 MLIR 方言。

除非另有明确说明，LLVM 方言操作的语义必须与 LLVM IR 指令的语义对应，任何差异都被视为错误。该方言还包含一些辅助操作，用于弥合 IR 结构上的差异，例如，MLIR 没有 `phi` 操作，而 LLVM IR 没有 `constant` 操作。这些辅助操作系统地带有 `mlir` 前缀，例如 `llvm.mlir.constant`，其中 `llvm.` 是方言命名空间前缀。

[TOC]

## 对 LLVM IR 的依赖

LLVM 方言不应依赖任何需要 `LLVMContext` 的对象，例如 LLVM IR 指令或类型。MLIR 提供了与其余基础设施兼容的线程安全替代方案。该方言允许依赖不需要上下文的 LLVM IR 对象，例如数据布局和三元组描述。

## 模块结构

IR 模块使用内置的 MLIR `ModuleOp` 并支持其所有功能。特别是，模块可以被命名、嵌套，并受符号可见性约束。模块可以包含任何操作，包括 LLVM 函数和全局变量。

### 数据布局与三元组

IR 模块可以选择性地使用 MLIR 属性 `llvm.data_layout` 和 `llvm.triple` 附加数据布局和三元组信息。这两者都是字符串属性，具有与 LLVM IR 相同的[语法](https://llvm.org/docs/LangRef.html#data-layout)，并经过验证以确保正确性。它们可以如下定义：

```mlir
module attributes {llvm.data_layout = "e",
                   llvm.target_triple = "aarch64-linux-android"} {
  // module contents
}
```

### 函数

LLVM 函数由一个特殊操作 `llvm.func` 表示，其语法与内置函数操作类似，但支持与 LLVM 相关的特性，如链接和可变参数列表。详细描述见下方操作列表中的 [llvm.func](#llvmfunc-llvmllvmfuncop)。

### PHI 节点与块参数

MLIR 使用块参数而不是 PHI 节点在块之间传递值。因此，LLVM 方言没有直接等价于 LLVM IR 中 `phi` 的操作。相反，所有终止符都可以将值作为后继操作数传递，当控制流转移时，这些值将作为块参数被转发。

例如：

```mlir
^bb1:
  %0 = llvm.addi %arg0, %cst : i32
  llvm.br ^bb2[%0: i32]

// If the control flow comes from ^bb1, %arg1 == %0.
^bb2(%arg1: i32)
  // ...
```

等价于 LLVM IR：

```llvm
%0:
  %1 = add i32 %arg0, %cst
  br %3

%3:
  %arg1 = phi [%1, %0], //...
```

由于不需要使用块标识符来区分不同值的来源，LLVM 方言支持将控制流转移到同一块但使用不同参数的终止符。例如：

```mlir
^bb1:
  llvm.cond_br %cond, ^bb2[%0: i32], ^bb2[%1: i32]

^bb2(%arg0: i32):
  // ...
```

### 上下文级别的值

LLVM IR 中的某些值类型（如常量和 undef）在上下文中是唯一的，并直接用于相关操作。MLIR 出于线程安全和概念简约的原因不支持此类值。相反，常规值由具有相应语义的专用操作生成：[`llvm.mlir.constant`](#llvmmlirconstant-llvmconstantop)、[`llvm.mlir.undef`](#llvmmlirundef-llvmundefop)、[`llvm.mlir.poison`](#llvmmlirpoison-llvmpoisonop)、[`llvm.mlir.zero`](#llvmmlirzero-llvmzeroop)。注意这些操作带有 `mlir.` 前缀，表明它们不属于 LLVM IR，只是为了在 MLIR 中对其建模而必需的。这些操作产生的值可以像任何其他值一样使用。

示例：

```mlir
// Create an undefined value of structure type with a 32-bit integer followed
// by a float.
%0 = llvm.mlir.undef : !llvm.struct<(i32, f32)>

// Null pointer.
%1 = llvm.mlir.zero : !llvm.ptr

// Create an zero initialized value of structure type with a 32-bit integer
// followed by a float.
%2 = llvm.mlir.zero :  !llvm.struct<(i32, f32)>

// Constant 42 as i32.
%3 = llvm.mlir.constant(42 : i32) : i32

// Splat dense vector constant.
%3 = llvm.mlir.constant(dense<1.0> : vector<4xf32>) : vector<4xf32>
```

注意常量将类型列出两次。这是 LLVM 方言不使用内置类型（内置类型用于带类型的 MLIR 属性）的产物。在考虑复合常量后，语法将被重新评估。

### 全局变量

全局变量也使用一个特殊操作 [`llvm.mlir.global`](#llvmmlirglobal-llvmglobalop) 定义，位于模块级别。全局变量是 MLIR 符号，由其名称标识。

由于函数需要与外部隔离，即函数外部定义的值不能直接在函数内部使用，因此提供了一个额外的操作 [`llvm.mlir.addressof`](#llvmmliraddressof-llvmaddressofop)，以在本地定义一个包含全局变量_地址_的值。然后可以从该指针加载实际值，或者如果全局变量未声明为常量，可以向其存储新值。这与 LLVM IR 类似，在 LLVM IR 中，全局变量通过名称访问并具有指针类型。

### 链接

LLVM 方言中的模块级命名对象（即函数和全局变量）具有一个可选的_链接_属性，派生自 LLVM IR 的[链接类型](https://llvm.org/docs/LangRef.html#linkage-types)。链接使用与 LLVM IR 中相同的关键字指定，位于操作名称（`llvm.func` 或 `llvm.global`）与符号名称之间。如果没有链接关键字，默认假定为 `external` 链接。链接与 MLIR 符号可见性_不同_。

### 属性透传

**警告：** 此功能绝不能用于任何实际工作负载。它专门用于快速原型开发。之后，属性必须作为方言中适当的一等概念引入。

LLVM 方言提供了一种机制，使用 `passthrough` 属性将函数级属性转发到 LLVM IR。这是一个数组属性，包含字符串属性或数组属性。在前一种情况下，字符串的值被解释为 LLVM IR 函数属性的名称。在后一种情况下，数组预期恰好包含两个字符串属性，第一个对应 LLVM IR 函数属性的名称，第二个对应其值。注意，即使是整数 LLVM IR 函数属性，其值也以字符串形式表示。

示例：

```mlir
llvm.func @func() attributes {
  passthrough = ["readonly",           // value-less attribute
                 ["alignstack", "4"],  // integer attribute with value
                 ["other", "attr"]]    // attribute unknown to LLVM
} {
  llvm.return
}
```

如果属性对 LLVM IR 未知，它将作为字符串属性附加。

## 类型

LLVM 方言尽可能使用内置类型，并定义一组补充类型，对应于无法直接用内置类型表示的 LLVM IR 类型。与其他 MLIR 上下文所拥有的对象类似，LLVM 方言类型的创建和操作是线程安全的。

MLIR 不支持模块作用域的命名类型声明，例如 LLVM IR 中的 `%s = type {i32, i32}`。相反，类型必须在每次使用时完整指定，递归类型除外，其中只有对命名类型的第一个引用需要完整指定。MLIR [类型别名](../LangRef.md/#type-aliases)可用于实现更紧凑的语法。

LLVM 方言类型的通用语法是 `!llvm.`，后跟类型种类标识符（例如，`ptr` 表示指针，`struct` 表示结构体），然后是尖括号中的可选类型参数列表。该方言遵循 MLIR 风格，对带有嵌套尖括号和关键字说明符的类型使用不同的括号样式来区分类型。尖括号内的类型可以省略 `!llvm.` 前缀以简洁起见：解析器首先尝试找到类型（以 `!` 或内置类型开头），如果失败则接受关键字。例如，`!llvm.struct<(!llvm.ptr, f32)>` 和 `!llvm.struct<(ptr, f32)>` 是等价的，后者是规范形式，表示包含一个指针和一个浮点数的结构体。

### 内置类型兼容性

LLVM 方言接受内置类型的一个子集，这些类型被称为 _LLVM 方言兼容类型_。以下类型是兼容的：

-   无符号性整数 - `iN`（`IntegerType`）。
-   浮点类型 - `bfloat`、`half`、`float`、`double`、`f80`、`f128`（`FloatType`）。
-   无符号性整数或浮点类型的一维向量 - `vector<NxT>`（`VectorType`）。

注意，只有给定类可以表示的类型的一个子集是兼容的。例如，有符号和无符号整数不兼容。LLVM 提供了一个函数 `bool LLVM::isCompatibleType(Type)`，可以用作兼容性检查。

每个 LLVM IR 类型恰好对应*一个* MLIR 类型，要么是内置类型，要么是 LLVM 方言类型。例如，因为 `i32` 是 LLVM 兼容的，所以没有 `!llvm.i32` 类型。然而，`!llvm.struct<(T, ...)>` 在 LLVM 方言中定义，因为没有对应的内置类型。

### 附加简单类型

以下从 LLVM IR 派生的非参数化类型在 LLVM 方言中可用：

-   `!llvm.ppc_fp128`（`LLVMPPCFP128Type`）- 128 位浮点值（两个 64 位）。
-   `!llvm.token`（`LLVMTokenType`）- 与操作关联的不可检查值。
-   `!llvm.metadata`（`LLVMMetadataType`）- LLVM IR 元数据，仅在元数据无法表示为结构化 MLIR 属性时使用。
-   `!llvm.void`（`LLVMVoidType`）- 不表示任何值；只能出现在函数结果中。

这些类型表示单个值（或在 `void` 的情况下表示值的缺失），并与其 LLVM IR 对应项对应。

### 附加参数化类型

这些类型由它们包含的类型参数化，例如指向的类型或元素类型，可以是兼容的内置类型或 LLVM 方言类型。

#### 指针类型

指针类型指定内存中的一个地址。

指针是[不透明的](https://llvm.org/docs/OpaquePointers.html)，即不指示所指向数据的类型，旨在通过将与指向类型相关的行为编码到操作中而不是类型中来简化 LLVM IR。指针可以选择用地址空间参数化。地址空间是一个整数，但如果 MLIR 实现了命名地址空间，这个选择可能会被重新考虑。指针类型的语法如下：

```
  llvm-ptr-type ::= `!llvm.ptr` (`<` integer-literal `>`)?
```

其中包含整数字面量的可选组对应于地址空间。所有情况在内部都由 `LLVMPointerType` 表示。

#### 数组类型

数组类型表示内存中的元素序列。数组元素可以用编译时未知的值寻址，并且可以嵌套。但只允许一维数组。

数组类型由固定大小和元素类型参数化。在语法上，其表示如下：

```
  llvm-array-type ::= `!llvm.array<` integer-literal `x` type `>`
```

内部表示为 `LLVMArrayType`。

#### 函数类型

函数类型表示函数的类型，即其签名。

函数类型由结果类型、参数类型列表和可选的"可变参数"标志参数化。与内置的 `FunctionType` 不同，LLVM 方言函数（`LLVMFunctionType`）始终有单个结果，如果函数不返回任何内容，结果可以是 `!llvm.void`。语法如下：

```
  llvm-func-type ::= `!llvm.func<` type `(` type-list (`,` `...`)? `)` `>`
```

例如：

```mlir
!llvm.func<void ()>           // a function with no arguments;
!llvm.func<i32 (f32, i32)>    // a function with two arguments and a result;
!llvm.func<void (i32, ...)>   // a variadic function with at least one argument.
```

在 LLVM 方言中，函数不是一等对象，不能有函数类型的值。相反，可以获取函数的地址并对函数指针进行操作。

### 向量类型

向量类型表示元素序列，通常是多个数据元素由单条指令处理时（SIMD）。向量被认为存储在寄存器中，因此向量元素只能通过常量索引寻址。

向量类型由大小（可以是_固定的_，也可以是_可扩展_向量情况下某个固定大小的倍数）和元素类型参数化。向量不能嵌套，只支持一维向量。可扩展向量仍被视为一维。

LLVM 方言使用内置向量类型。

以下函数用于操作与 LLVM 方言兼容的任何类型的向量：

-   `bool LLVM::isCompatibleVectorType(Type)` - 检查类型是否是与 LLVM 方言兼容的向量类型；
-   `llvm::ElementCount LLVM::getVectorNumElements(Type)` - 返回与 LLVM 方言兼容的任何向量类型中的元素数量；

#### 兼容向量类型的示例

```mlir
vector<42 x i32>                   // Vector of 42 32-bit integers.
vector<42 x !llvm.ptr>             // Vector of 42 pointers.
vector<[4] x i32>                  // Scalable vector of 32-bit integers with
                                   // size divisible by 4.
!llvm.array<2 x vector<2 x i32>>   // Array of 2 vectors of 2 32-bit integers.
!llvm.array<2 x vec<2 x ptr>> // Array of 2 vectors of 2 pointers.
```

### 结构体类型

结构体类型用于在内存中将一组数据成员放在一起表示。结构体的元素可以是任何有大小的类型。

结构体类型在单个专用类 mlir::LLVM::LLVMStructType 中表示。在内部，结构体类型存储一个（可能为空的）名称、一个（可能为空的）包含类型列表，以及一个位掩码，指示结构体是否有名称、不透明、打包或未初始化。没有名称的结构体类型称为_字面量_结构体，此类结构体由其内容唯一标识。_具名_结构体则由其名称唯一标识。

#### 具名结构体类型

具名结构体类型在给定上下文中使用其名称进行唯一化。尝试用与上下文中已存在的结构体相同的名称构造具名结构体，*将导致返回现有结构体*。**MLIR 不会在名称冲突的情况下自动重命名具名结构体**，因为没有与 LLVM IR 中的模块等价的命名作用域，而 MLIR 模块可以任意嵌套。

以编程方式，具名结构体可以在_未初始化_状态下构造。在这种情况下，它们被赋予名称，但必须通过后续调用使用 MLIR 的类型变更机制来设置主体。此类未初始化类型可以用于类型构造，但最终必须初始化才能使 IR 有效。该机制允许构造_递归_或相互引用的结构体类型：未初始化的类型可以用于其自身的初始化。

一旦类型初始化，其主体就不能再更改。任何进一步修改主体的尝试都将失败，并向调用者返回失败，_除非类型以完全相同的主体初始化_。类型初始化是线程安全的；然而，如果并发线程在当前线程之前初始化了类型，初始化可能会返回失败。

具名结构体类型的语法如下：

```
llvm-ident-struct-type ::= `!llvm.struct<` string-literal, `opaque` `>`
                         | `!llvm.struct<` string-literal, `packed`?
                           `(` type-or-ref-list  `)` `>`
type-or-ref-list ::= <maybe empty comma-separated list of type-or-ref>
type-or-ref ::= <any compatible type with optional !llvm.>
              | `!llvm.`? `struct<` string-literal `>`
```

#### 字面量结构体类型

字面量结构体根据其包含的元素列表进行唯一化，可以选择打包。此类结构体的语法如下：

```
llvm-literal-struct-type ::= `!llvm.struct<` `packed`? `(` type-list `)` `>`
type-list ::= <maybe empty comma-separated list of types with optional !llvm.>
```

字面量结构体不能递归，但可以包含其他结构体。因此，它们必须在一步中构造，并提供所有包含元素的完整列表。

#### 结构体类型的示例

```mlir
!llvm.struct<>                  // NOT allowed
!llvm.struct<()>                // empty, literal
!llvm.struct<(i32)>             // literal
!llvm.struct<(struct<(i32)>)>   // struct containing a struct
!llvm.struct<packed (i8, i32)>  // packed struct
!llvm.struct<"a">               // recursive reference, only allowed within
                                // another struct, NOT allowed at top level
!llvm.struct<"a", ()>           // empty, named (necessary to differentiate from
                                // recursive reference)
!llvm.struct<"a", opaque>       // opaque, named
!llvm.struct<"a", (i32, ptr)>        // named
!llvm.struct<"a", packed (i8, i32)>  // named, packed
```

### 不支持的类型

LLVM IR 的 `label` 类型在 LLVM 方言中没有对应项，因为在 MLIR 中，块不是值，不需要类型。

## 操作

LLVM IR 方言中的所有操作在 MLIR 中都有自定义形式。操作的助记符是在 LLVM IR 中使用的名称加上前缀 "`llvm.`"。

[include "Dialects/LLVMOps.md"]

## LLVM IR 内联函数的操作

MLIR 操作系统是开放的，因此不需要在"核心"操作和"内联函数"之间引入硬性界限。通用 LLVM IR 内联函数作为 LLVM 方言中的一等操作建模。目标特定的 LLVM IR 内联函数（例如 NVVM 或 ROCDL）作为独立的方言建模。

[include "Dialects/LLVMIntrinsicOps.md"]

### 调试信息

LLVM 方言中的调试信息使用位置与一组属性的组合来表示，这些属性映射了 LLVM IR 中调试信息元数据定义的 DINode 结构。调试作用域信息使用融合位置（`FusedLoc`）附加到 LLVM IR 方言操作，其元数据保存表示调试作用域的 DIScopeAttr。类似地，LLVM IR 方言 `FuncOp` 操作的子程序使用融合位置附加，其元数据是 DISubprogramAttr。
