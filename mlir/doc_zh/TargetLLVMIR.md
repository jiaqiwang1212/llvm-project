# LLVM IR 目标

本文档描述从 MLIR 生成 LLVM IR 的机制。整体流程分为两个阶段：

1.  将 IR **转换（conversion）**为可翻译为 LLVM IR 的一组方言（dialect），例如 [LLVM 方言](Dialects/LLVM.md) 或从 LLVM IR 内联函数派生的硬件特定方言，如 [X86](Dialects/X86.md) 或 [ArmNeon](Dialects/ArmNeon.md)；
2.  将 MLIR 方言**翻译（translation）**为 LLVM IR。

此流程允许在 MLIR 内部使用 MLIR API 执行非平凡的变换，使 MLIR 与 LLVM IR 之间的翻译*简单*且可能是双向的。因此，可翻译为 LLVM IR 的方言操作（op）应与对应的 LLVM IR 指令和内联函数紧密对应。这一设计最小化了 MLIR 对 LLVM IR 库的依赖，并减少了因变更导致的额外工作。

请注意，许多不同的方言可以降级（lower）到 LLVM，但它们作为不同的模式集提供，并有不同的流水线（pass）可供 mlir-opt 使用。然而，这主要用于测试和原型验证，强烈建议将这些模式集合在一起使用。这一点在 ControlFlow 方言的分支操作中尤为重要——如果这些操作的类型与其父操作中跳转目标块的类型不匹配，将无法应用。

SPIR-V 到 LLVM 方言的转换有[专门文档](SPIRVToLLVMDialectConversion.md)。

[TOC]

## 转换为 LLVM 方言

从其他方言转换为 LLVM 方言是生成 LLVM IR 的第一步。所有非平凡的 IR 修改都应在此阶段或之前完成。转换是*渐进式（progressive）*的：大多数流水线将一个方言转换为 LLVM 方言，同时保持其他方言的操作不变。例如，`-finalize-memref-to-llvm` 流水线仅转换 `memref` 方言的操作，不会转换其他方言的操作，即使这些操作使用或产生 `memref` 类型的值。

该流程依赖于[方言转换（Dialect Conversion）](DialectConversion.md)基础设施，特别是 `TypeConverter` 的[物化（materialization）](DialectConversion.md/#type-conversion)钩子，通过在已转换和未转换操作之间注入 `unrealized_conversion_cast` 操作来支持渐进式降级。在对 LLVM 方言执行多次部分转换之后，可以通过 `-reconcile-unrealized-casts` 流水线删除那些已变为空操作的强制转换。后者不特定于 LLVM 方言，可删除任何空操作强制转换。

### 内置类型的转换

内置类型到 LLVM 方言类型有 `LLVMTypeConverter` 类提供的默认转换。面向 LLVM 方言的用户可以复用并扩展该类型转换器以支持其他类型。如果覆盖内置类型的转换规则，必须格外小心：所有转换都必须使用相同的类型转换器。

#### 与 LLVM 方言兼容的类型

与 LLVM 方言[兼容](Dialects/LLVM.md/#built-in-type-compatibility)的类型保持不变。

#### Complex 类型

Complex 类型被转换为具有两个元素的 LLVM 方言字面量结构体类型：

-   实部；
-   虚部。

元素类型使用这些规则递归转换。

示例：

```mlir
  complex<f32>
  // ->
  !llvm.struct<(f32, f32)>
```

#### Index 类型

Index 类型被转换为 LLVM 方言整数类型，位宽由最近模块的[数据布局（data layout）](DataLayout.md)指定。例如，在 x86-64 CPU 上转换为 i64。此行为可通过类型转换器配置覆盖，通常由转换流水线作为流水线选项对外暴露。

示例：

```mlir
  index
  // -> on x86_64
  i64
```

#### 有秩 MemRef 类型

有秩（ranked）的 memref 类型被转换为包含 memref 对象动态信息的 LLVM 方言字面量结构体类型，称为*描述符（descriptor）*。只有处于**[步幅形式（strided form）](Dialects/Builtin.md/#strided-memref)**的 memref 才能使用默认描述符格式转换为 LLVM 方言。具有其他较复杂布局的 memref 应首先转换为步幅形式，例如通过将非平凡的地址重映射物化为 `affine.apply` 操作。

默认的 memref 描述符是一个具有以下字段的结构体：

1.  指向已分配数据缓冲区的指针，称为"已分配指针（allocated pointer）"。该指针仅用于释放 memref。
2.  指向 memref 实际索引的正确对齐数据指针，称为"对齐指针（aligned pointer）"。
3.  一个降级后的 `index` 类型整数，表示（对齐）缓冲区起始位置与通过 memref 访问的第一个元素之间的元素数量距离，称为"偏移（offset）"。
4.  一个数组，其中包含与 memref 秩相同数量的 `index` 类型整数：该数组表示 memref 在指定维度上的大小（元素数量）。
5.  第二个数组，其中包含与 memref 秩相同数量的 `index` 类型整数：第二个数组表示"步幅（stride）"（张量抽象意义上），即在底层缓冲区的连续元素中，需要跳过多少个元素才能到达下一个逻辑索引元素。

对于常量 memref 维度，对应的大小条目是一个运行时值与静态值匹配的常量。这种规范化作为 memref 类型与外部链接函数互操作的 ABI。对于秩为 `0` 的 memref，大小和步幅数组被省略，结果是包含两个指针加偏移的结构体。

示例：

```mlir
// Assuming index is converted to i64.

memref<f32> -> !llvm.struct<(ptr , ptr, i64)>
memref<1 x f32> -> !llvm.struct<(ptr, ptr, i64,
                                 array<1 x i64>, array<1 x i64>)>
memref<? x f32> -> !llvm.struct<(ptr, ptr, i64
                                 array<1 x i64>, array<1 x i64>)>
memref<10x42x42x43x123 x f32> -> !llvm.struct<(ptr, ptr, i64
                                               array<5 x i64>, array<5 x i64>)>
memref<10x?x42x?x123 x f32> -> !llvm.struct<(ptr, ptr, i64
                                             array<5 x i64>, array<5 x i64>)>

// Memref types can have vectors as element types
memref<1x? x vector<4xf32>> -> !llvm.struct<(ptr, ptr, i64, array<2 x i64>,
                                             array<2 x i64>)>
```

#### 无秩 MemRef 类型

无秩（unranked）的 memref 类型被转换为包含 memref 对象动态信息的 LLVM 方言字面量结构体类型，称为*无秩描述符（unranked descriptor）*。它包含：

1.  一个 `index` 类型整数，表示 memref 的动态秩；
2.  一个类型擦除指针（`!llvm.ptr`），指向有秩 memref 描述符（如上所述）。

该描述符主要用于与秩多态库函数接口。指向有秩 memref 描述符的指针指向某些*已分配的*内存，这些内存可能位于当前函数的栈上或堆中。生成无秩 memref 的操作的转换模式应负责管理分配。请注意，这可能导致在循环中执行栈分配（`llvm.alloca`），并且直到当前函数结束才会被回收。

#### 函数类型

函数类型按如下规则转换为 LLVM 方言函数类型：

-   函数参数和返回值类型使用这些规则递归转换；
-   如果函数类型有多个返回值，它们被包装成 LLVM 方言字面量结构体类型，因为 LLVM 函数类型必须恰好有一个返回值；
-   如果函数类型没有返回值，对应的 LLVM 方言函数类型将有一个 `!llvm.void` 返回值，因为 LLVM 函数类型必须有返回值；
-   在另一个函数类型的参数中使用的函数类型被包装在 LLVM 方言指针类型中，以符合 LLVM IR 的要求；
-   作为函数参数出现的 `memref` 类型对应的结构体（有秩和无秩）被拆包为单独的函数参数，以允许在各个指针上指定别名信息等元数据；
-   `memref` 类型参数的转换遵从[调用约定（calling conventions）](#calling-conventions)；
-   如果函数类型的布尔属性 `func.varargs` 被设置，则转换后的 LLVM 函数将是可变参数函数。

示例：

```mlir
// Zero-ary function type with no results:
() -> ()
// is converted to a zero-ary function with `void` result.
!llvm.func<void ()>

// Unary function with one result:
(i32) -> (i64)
// has its argument and result type converted, before creating the LLVM dialect
// function type.
!llvm.func<i64 (i32)>

// Binary function with one result:
(i32, f32) -> (i64)
// has its arguments handled separately
!llvm.func<i64 (i32, f32)>

// Binary function with two results:
(i32, f32) -> (i64, f64)
// has its result aggregated into a structure type.
!llvm.func<struct<(i64, f64)> (i32, f32)>

// Function-typed arguments or results in higher-order functions:
(() -> ()) -> (() -> ())
// are converted into opaque pointers.
!llvm.func<ptr (ptr)>

// A memref descriptor appearing as function argument:
(memref<f32>) -> ()
// gets converted into a list of individual scalar components of a descriptor.
!llvm.func<void (ptr, ptr, i64)>

// The list of arguments is linearized and one can freely mix memref and other
// types in this list:
(memref<f32>, f32) -> ()
// which gets converted into a flat list.
!llvm.func<void (ptr, ptr, i64, f32)>

// For nD ranked memref descriptors:
(memref<?x?xf32>) -> ()
// the converted signature will contain 2n+1 `index`-typed integer arguments,
// offset, n sizes and n strides, per memref argument type.
!llvm.func<void (ptr, ptr, i64, i64, i64, i64, i64)>

// Same rules apply to unranked descriptors:
(memref<*xf32>) -> ()
// which get converted into their components.
!llvm.func<void (i64, ptr)>

// However, returning a memref from a function is not affected:
() -> (memref<?xf32>)
// gets converted to a function returning a descriptor structure.
!llvm.func<struct<(ptr, ptr, i64, array<1xi64>, array<1xi64>)> ()>

// If multiple memref-typed results are returned:
() -> (memref<f32>, memref<f64>)
// their descriptor structures are additionally packed into another structure,
// potentially with other non-memref typed results.
!llvm.func<struct<(struct<(ptr, ptr, i64)>,
                   struct<(ptr, ptr, i64)>)> ()>

// If "func.varargs" attribute is set:
(i32) -> () attributes { "func.varargs" = true }
// the corresponding LLVM function will be variadic:
!llvm.func<void (i32, ...)>
```

转换模式可用于使用这些转换规则转换内置函数操作和以这些函数为目标的标准调用操作。

#### 多维向量类型

与 MLIR 不同，LLVM IR 只支持*一维*向量，两种 IR 中的向量类型都不能嵌套。在一维情况下，MLIR 向量被转换为具有相同大小、使用这些转换规则转换元素类型的 LLVM IR 向量。在 n 维情况下，MLIR 向量被转换为一维向量的 (n-1) 维数组类型。

示例：

```
vector<4x8 x f32>
// ->
!llvm.array<4 x vector<8 x f32>>

memref<2 x vector<4x8 x f32>
// ->
!llvm.struct<(ptr, ptr, i64, array<1 x i64>, array<1 x i64>)>
```

#### Tensor 类型

Tensor 类型无法转换为 LLVM 方言。对张量的操作必须在转换之前[缓冲化（bufferized）](Bufferization.md)。

### 含非兼容元素类型的 LLVM 容器类型的转换

渐进式降级可能导致 LLVM 容器类型（如 LLVM 方言结构体）包含不兼容的类型，例如 `!llvm.struct<(index)>`。此类类型使用上述规则递归转换。

具名结构体被转换为*新的*结构体，其标识符前缀为 `_Converted.`，因为具名类型的主体一旦初始化就无法更新。此类名称被视为*保留*名称，不得出现在输入代码中（实际上，C 保留以 `_` 开头并加大写字母的名称，而 `.` 无论如何都不能出现在有效的 C 类型中）。如果它们确实出现且主体与转换结果不同，类型转换将停止。

### 调用约定（Calling Conventions）

调用约定提供了一种机制，用于自定义函数和函数调用操作的转换，而不改变其他地方对各个类型的处理方式。它们由默认类型转换器和相关操作的转换模式同时实现。

#### 函数返回值打包

对于多返回值函数，在返回之前将返回值插入结构体类型值中，并在调用点从中提取。这种转换是转换的一部分，对被返回值的定义和使用透明。

示例：

```mlir
func.func @foo(%arg0: i32, %arg1: i64) -> (i32, i64) {
  return %arg0, %arg1 : i32, i64
}
func.func @bar() {
  %0 = arith.constant 42 : i32
  %1 = arith.constant 17 : i64
  %2:2 = call @foo(%0, %1) : (i32, i64) -> (i32, i64)
  "use_i32"(%2#0) : (i32) -> ()
  "use_i64"(%2#1) : (i64) -> ()
}

// is transformed into

llvm.func @foo(%arg0: i32, %arg1: i64) -> !llvm.struct<(i32, i64)> {
  // insert the values into a structure
  %0 = llvm.mlir.undef : !llvm.struct<(i32, i64)>
  %1 = llvm.insertvalue %arg0, %0[0] : !llvm.struct<(i32, i64)>
  %2 = llvm.insertvalue %arg1, %1[1] : !llvm.struct<(i32, i64)>

  // return the structure value
  llvm.return %2 : !llvm.struct<(i32, i64)>
}
llvm.func @bar() {
  %0 = llvm.mlir.constant(42 : i32) : i32
  %1 = llvm.mlir.constant(17 : i64) : i64

  // call and extract the values from the structure
  %2 = llvm.call @foo(%0, %1)
     : (i32, i64) -> !llvm.struct<(i32, i64)>
  %3 = llvm.extractvalue %2[0] : !llvm.struct<(i32, i64)>
  %4 = llvm.extractvalue %2[1] : !llvm.struct<(i32, i64)>

  // use as before
  "use_i32"(%3) : (i32) -> ()
  "use_i64"(%4) : (i64) -> ()
}
```

#### 有秩 MemRef 的默认调用约定

默认调用约定将 `memref` 类型的函数参数转换为上述[有秩 MemRef 类型](#ranked-memref-types)定义的 LLVM 方言字面量结构体，然后将其拆包为单独的标量参数。

该约定在 `func.func` 和 `func.call` 到 LLVM 方言的转换中实现，前者将描述符拆包为一组单独的值，后者将这些值重新打包到描述符中，使其可透明地被其他操作使用。来自其他方言的转换应将此约定纳入考量。

此特定约定的动机是需要在底层 memref 的原始指针上指定对齐和别名属性。

示例：

```mlir
func.func @foo(%arg0: memref<?xf32>) -> () {
  "use"(%arg0) : (memref<?xf32>) -> ()
  return
}

// Gets converted to the following
// (using type alias for brevity):
!llvm.memref_1d = !llvm.struct<(ptr, ptr, i64, array<1xi64>, array<1xi64>)>

llvm.func @foo(%arg0: !llvm.ptr,       // Allocated pointer.
               %arg1: !llvm.ptr,       // Aligned pointer.
               %arg2: i64,             // Offset.
               %arg3: i64,             // Size in dim 0.
               %arg4: i64) {           // Stride in dim 0.
  // Populate memref descriptor structure.
  %0 = llvm.mlir.undef : !llvm.memref_1d
  %1 = llvm.insertvalue %arg0, %0[0] : !llvm.memref_1d
  %2 = llvm.insertvalue %arg1, %1[1] : !llvm.memref_1d
  %3 = llvm.insertvalue %arg2, %2[2] : !llvm.memref_1d
  %4 = llvm.insertvalue %arg3, %3[3, 0] : !llvm.memref_1d
  %5 = llvm.insertvalue %arg4, %4[4, 0] : !llvm.memref_1d

  // Descriptor is now usable as a single value.
  "use"(%5) : (!llvm.memref_1d) -> ()
  llvm.return
}
```

```mlir
func.func @bar() {
  %0 = "get"() : () -> (memref<?xf32>)
  call @foo(%0) : (memref<?xf32>) -> ()
  return
}

// Gets converted to the following
// (using type alias for brevity):
!llvm.memref_1d = !llvm.struct<(ptr, ptr, i64, array<1xi64>, array<1xi64>)>

llvm.func @bar() {
  %0 = "get"() : () -> !llvm.memref_1d

  // Unpack the memref descriptor.
  %1 = llvm.extractvalue %0[0] : !llvm.memref_1d
  %2 = llvm.extractvalue %0[1] : !llvm.memref_1d
  %3 = llvm.extractvalue %0[2] : !llvm.memref_1d
  %4 = llvm.extractvalue %0[3, 0] : !llvm.memref_1d
  %5 = llvm.extractvalue %0[4, 0] : !llvm.memref_1d

  // Pass individual values to the callee.
  llvm.call @foo(%1, %2, %3, %4, %5) : (!llvm.memref_1d) -> ()
  llvm.return
}
```

#### 无秩 MemRef 的默认调用约定

对于无秩 memref，函数参数列表始终包含两个元素，与无秩 memref 描述符相同：一个整数秩和一个类型擦除的（`!llvm.ptr`）指向有秩 memref 描述符的指针。请注意，虽然*调用约定*本身不需要分配，但*转换（casting）*到无秩 memref 需要分配，因为无法获取包含有秩 memref 的 SSA 值的地址，该值必须存储在某些内存中。调用者负责确保已分配内存的线程安全性和管理，特别是释放。

示例：

```mlir
llvm.func @foo(%arg0: memref<*xf32>) -> () {
  "use"(%arg0) : (memref<*xf32>) -> ()
  return
}

// Gets converted to the following.

llvm.func @foo(%arg0: i64              // Rank.
               %arg1: !llvm.ptr) { // Type-erased pointer to descriptor.
  // Pack the unranked memref descriptor.
  %0 = llvm.mlir.undef : !llvm.struct<(i64, ptr)>
  %1 = llvm.insertvalue %arg0, %0[0] : !llvm.struct<(i64, ptr)>
  %2 = llvm.insertvalue %arg1, %1[1] : !llvm.struct<(i64, ptr)>

  "use"(%2) : (!llvm.struct<(i64, ptr)>) -> ()
  llvm.return
}
```

```mlir
llvm.func @bar() {
  %0 = "get"() : () -> (memref<*xf32>)
  call @foo(%0): (memref<*xf32>) -> ()
  return
}

// Gets converted to the following.

llvm.func @bar() {
  %0 = "get"() : () -> (!llvm.struct<(i64, ptr)>)

  // Unpack the memref descriptor.
  %1 = llvm.extractvalue %0[0] : !llvm.struct<(i64, ptr)>
  %2 = llvm.extractvalue %0[1] : !llvm.struct<(i64, ptr)>

  // Pass individual values to the callee.
  llvm.call @foo(%1, %2) : (i64, !llvm.ptr)
  llvm.return
}
```

**生命周期。** 无秩 memref 描述符的第二个元素指向存储有秩 memref 描述符的某块内存。按照约定，该内存在栈上分配，生命周期与函数相同。（*注意：* 由于函数级别的生命周期，在循环中创建多个无秩 memref 描述符可能导致栈溢出。）如果无秩描述符必须从函数返回，则其指向的有秩描述符会被复制到动态分配的内存中，并相应地更新无秩描述符中的指针。分配在返回前立即发生。调用者有责任释放动态分配的内存。`func.call` 和 `func.call_indirect` 的默认转换将有秩描述符复制到调用者栈上新分配的内存中，因此遵守无秩 memref 描述符所指向的有秩 memref 描述符存储在栈上的约定。

#### 有秩 MemRef 的裸指针调用约定

"裸指针（bare pointer）"调用约定将 `memref` 类型的函数参数转换为指向对齐数据的*单个*指针。请注意，这*不*适用于函数签名之外对 `memref` 的使用，默认描述符结构体仍会被使用。该约定进一步将支持的情况限制为以下几种：

-   具有默认布局的 `memref` 类型；
-   所有维度静态已知的 `memref` 类型；
-   以已分配指针和对齐指针相同的方式分配的 `memref` 值。或者，同一函数必须同时处理分配和释放，因为只有一个指针传递给任何被调用者。

示例：

```
func.func @callee(memref<2x4xf32>)

func.func @caller(%0 : memref<2x4xf32>) {
  call @callee(%0) : (memref<2x4xf32>) -> ()
}

// ->

!descriptor = !llvm.struct<(ptr, ptr, i64,
                            array<2xi64>, array<2xi64>)>

llvm.func @callee(!llvm.ptr)

llvm.func @caller(%arg0: !llvm.ptr) {
  // A descriptor value is defined at the function entry point.
  %0 = llvm.mlir.undef : !descriptor

  // Both the allocated and aligned pointer are set up to the same value.
  %1 = llvm.insertelement %arg0, %0[0] : !descriptor
  %2 = llvm.insertelement %arg0, %1[1] : !descriptor

  // The offset is set up to zero.
  %3 = llvm.mlir.constant(0 : index) : i64
  %4 = llvm.insertelement %3, %2[2] : !descriptor

  // The sizes and strides are derived from the statically known values.
  %5 = llvm.mlir.constant(2 : index) : i64
  %6 = llvm.mlir.constant(4 : index) : i64
  %7 = llvm.insertelement %5, %4[3, 0] : !descriptor
  %8 = llvm.insertelement %6, %7[3, 1] : !descriptor
  %9 = llvm.mlir.constant(1 : index) : i64
  %10 = llvm.insertelement %9, %8[4, 0] : !descriptor
  %11 = llvm.insertelement %10, %9[4, 1] : !descriptor

  // The function call corresponds to extracting the aligned data pointer.
  %12 = llvm.extractelement %11[1] : !descriptor
  llvm.call @callee(%12) : (!llvm.ptr) -> ()
}
```

#### 无秩 MemRef 的裸指针调用约定

"裸指针"调用约定不支持无秩 memref，因为其形状在编译时无法确定。

### 通用分配与释放函数

在转换 Memref 方言时，分配和释放操作被转换为对 `malloc`（如果请求对齐分配则为 `aligned_alloc`）和 `free` 的调用。然而，也可以将它们转换为更通用的函数，这些函数可由运行时库实现，从而允许自定义分配策略或运行时分析。当转换流水线被指示执行此类操作时，被调用者的名称为 `_mlir_memref_to_llvm_alloc`、`_mlir_memref_to_llvm_aligned_alloc` 和 `_mlir_memref_to_llvm_free`，其签名与 `malloc`、`aligned_alloc` 和 `free` 相同。

### C 兼容包装函数生成

在实际场景中，可能需要具有与 MemRef 参数对应的单一属性的外部可见函数。与 C 生成的 LLVM IR 接口时，代码需要遵守对应的调用约定。转换为 LLVM 方言提供了一个选项，可以生成包装函数，这些函数接受与 Clang 编译 C 源代码时生成的数据类型兼容的 memref 描述符指针结构体。可以通过在函数上设置 `llvm.emit_c_interface` 单元属性来额外控制此类包装函数的生成粒度。

更具体地说，在包装函数中，memref 参数被转换为类型为 `{T*, T*, i64, i64[N], i64[N]}*` 的结构体指针参数，其中 `T` 是转换后的元素类型，`N` 是 memref 的秩。该类型与 Clang 为以下 C++ 结构体模板实例化（或其 C 等价物）生成的类型兼容。

```cpp
template<typename T, size_t N>
struct MemRefDescriptor {
  T *allocated;
  T *aligned;
  intptr_t offset;
  intptr_t sizes[N];
  intptr_t strides[N];
};
```

此外，如果重写后的函数结果为结构体类型，我们还会将函数结果重写为指针参数。特殊的结果参数作为第一个参数添加，类型为结构体指针。

如果启用此选项，对于 MLIR 模块中声明的*外部*函数，将执行以下操作：

1.  声明一个新函数 `_mlir_ciface_<original name>`，其中 memref 参数被转换为结构体指针，其余参数按常规转换。如果结果为结构体类型，则将其转换为特殊参数。
2.  为原始函数添加主体（使其变为非外部函数），该主体：
    1.  分配 memref 描述符；
    2.  填充它们；
    3.  可能为结果结构体分配空间；
    4.  将这些指针传递给新声明的接口函数；然后
    5.  收集调用结果（可能来自结果结构体）；
    6.  将结果返回给调用者。

对于 MLIR 模块中定义的（非外部）函数：

1.  定义一个新函数 `_mlir_ciface_<original name>`，其中 memref 参数被转换为结构体指针，其余参数按常规转换。如果结果为结构体类型，则将其转换为特殊参数。
2.  在新定义函数的主体中填充以下 IR：
    1.  从指针加载描述符；
    2.  将描述符拆包为单独的非聚合值；
    3.  将这些值传递给原始函数；
    4.  收集调用结果；
    5.  将结果复制到结果结构体或返回给调用者。

示例：

```mlir

func.func @qux(%arg0: memref<?x?xf32>) attributes {llvm.emit_c_interface}

// Gets converted into the following
// (using type alias for brevity):
!llvm.memref_2d = !llvm.struct<(ptr, ptr, i64, array<2xi64>, array<2xi64>)>

// Function with unpacked arguments.
llvm.func @qux(%arg0: !llvm.ptr, %arg1: !llvm.ptr,
               %arg2: i64, %arg3: i64, %arg4: i64,
               %arg5: i64, %arg6: i64) {
  // Populate memref descriptor (as per calling convention).
  %0 = llvm.mlir.undef : !llvm.memref_2d
  %1 = llvm.insertvalue %arg0, %0[0] : !llvm.memref_2d
  %2 = llvm.insertvalue %arg1, %1[1] : !llvm.memref_2d
  %3 = llvm.insertvalue %arg2, %2[2] : !llvm.memref_2d
  %4 = llvm.insertvalue %arg3, %3[3, 0] : !llvm.memref_2d
  %5 = llvm.insertvalue %arg5, %4[4, 0] : !llvm.memref_2d
  %6 = llvm.insertvalue %arg4, %5[3, 1] : !llvm.memref_2d
  %7 = llvm.insertvalue %arg6, %6[4, 1] : !llvm.memref_2d

  // Store the descriptor in a stack-allocated space.
  %8 = llvm.mlir.constant(1 : index) : i64
  %9 = llvm.alloca %8 x !llvm.memref_2d
     : (i64) -> !llvm.ptr
  llvm.store %7, %9 : !llvm.memref_2d, !llvm.ptr

  // Call the interface function.
  llvm.call @_mlir_ciface_qux(%9) : (!llvm.ptr) -> ()

  // The stored descriptor will be freed on return.
  llvm.return
}

// Interface function.
llvm.func @_mlir_ciface_qux(!llvm.ptr)
```


```cpp
// The C function implementation for the interface function.
extern "C" {
void _mlir_ciface_qux(MemRefDescriptor<float, 2> *input) {
  // detailed impl
}
}
```

```mlir
func.func @foo(%arg0: memref<?x?xf32>) attributes {llvm.emit_c_interface} {
  return
}

// Gets converted into the following
// (using type alias for brevity):
!llvm.memref_2d = !llvm.struct<(ptr, ptr, i64, array<2xi64>, array<2xi64>)>

// Function with unpacked arguments.
llvm.func @foo(%arg0: !llvm.ptr, %arg1: !llvm.ptr,
               %arg2: i64, %arg3: i64, %arg4: i64,
               %arg5: i64, %arg6: i64) {
  llvm.return
}

// Interface function callable from C.
llvm.func @_mlir_ciface_foo(%arg0: !llvm.ptr) {
  // Load the descriptor.
  %0 = llvm.load %arg0 : !llvm.ptr -> !llvm.memref_2d

  // Unpack the descriptor as per calling convention.
  %1 = llvm.extractvalue %0[0] : !llvm.memref_2d
  %2 = llvm.extractvalue %0[1] : !llvm.memref_2d
  %3 = llvm.extractvalue %0[2] : !llvm.memref_2d
  %4 = llvm.extractvalue %0[3, 0] : !llvm.memref_2d
  %5 = llvm.extractvalue %0[3, 1] : !llvm.memref_2d
  %6 = llvm.extractvalue %0[4, 0] : !llvm.memref_2d
  %7 = llvm.extractvalue %0[4, 1] : !llvm.memref_2d
  llvm.call @foo(%1, %2, %3, %4, %5, %6, %7)
    : (!llvm.ptr, !llvm.ptr, i64, i64, i64,
       i64, i64) -> ()
  llvm.return
}
```

```cpp
// The C function signature for the interface function.
extern "C" {
void _mlir_ciface_foo(MemRefDescriptor<float, 2> *input);
}
```

```mlir
func.func @foo(%arg0: memref<?x?xf32>) -> memref<?x?xf32> attributes {llvm.emit_c_interface} {
  return %arg0 : memref<?x?xf32>
}

// Gets converted into the following
// (using type alias for brevity):
!llvm.memref_2d = !llvm.struct<(ptr, ptr, i64, array<2xi64>, array<2xi64>)>

// Function with unpacked arguments.
llvm.func @foo(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: i64,
               %arg3: i64, %arg4: i64, %arg5: i64, %arg6: i64)
    -> !llvm.memref_2d {
  %0 = llvm.mlir.undef : !llvm.memref_2d
  %1 = llvm.insertvalue %arg0, %0[0] : !llvm.memref_2d
  %2 = llvm.insertvalue %arg1, %1[1] : !llvm.memref_2d
  %3 = llvm.insertvalue %arg2, %2[2] : !llvm.memref_2d
  %4 = llvm.insertvalue %arg3, %3[3, 0] : !llvm.memref_2d
  %5 = llvm.insertvalue %arg5, %4[4, 0] : !llvm.memref_2d
  %6 = llvm.insertvalue %arg4, %5[3, 1] : !llvm.memref_2d
  %7 = llvm.insertvalue %arg6, %6[4, 1] : !llvm.memref_2d
  llvm.return %7 : !llvm.memref_2d
}

// Interface function callable from C.
// NOTE: the returned memref becomes the first argument
llvm.func @_mlir_ciface_foo(%arg0: !llvm.ptr, %arg1: !llvm.ptr) {
  %0 = llvm.load %arg1 : !llvm.ptr
  %1 = llvm.extractvalue %0[0] : !llvm.memref_2d
  %2 = llvm.extractvalue %0[1] : !llvm.memref_2d
  %3 = llvm.extractvalue %0[2] : !llvm.memref_2d
  %4 = llvm.extractvalue %0[3, 0] : !llvm.memref_2d
  %5 = llvm.extractvalue %0[3, 1] : !llvm.memref_2d
  %6 = llvm.extractvalue %0[4, 0] : !llvm.memref_2d
  %7 = llvm.extractvalue %0[4, 1] : !llvm.memref_2d
  %8 = llvm.call @foo(%1, %2, %3, %4, %5, %6, %7)
    : (!llvm.ptr, !llvm.ptr, i64, i64, i64, i64, i64) -> !llvm.memref_2d
  llvm.store %8, %arg0 : !llvm.memref_2d, !llvm.ptr
  llvm.return
}
```

```cpp
// The C function signature for the interface function.
extern "C" {
void _mlir_ciface_foo(MemRefDescriptor<float, 2> *output,
                      MemRefDescriptor<float, 2> *input);
}
```

设计理由：引入辅助函数作为 C 兼容接口比修改调用约定更可取，因为这将最小化 C 兼容性对模块内调用或 MLIR 生成函数之间调用的影响。特别是，当在（并行）循环中从 MLIR 模块调用外部函数时，在栈上存储 memref 描述符这一事实可能导致栈耗尽和/或对同一地址的并发访问。辅助接口函数在此情况下充当分配作用域。此外，当面向具有独立内存空间的加速器（如 GPU）时，通过指针传递的栈分配描述符必须传输到设备内存，这会引入显著的开销。在这种情况下，辅助接口函数在主机上执行，仅通过设备函数调用机制传递值。

限制：目前无法为可变参数函数生成 C 接口，无论是非外部的还是外部的。因为 C 函数无法像这样"转发"可变参数：
```c
void bar(int, ...);

void foo(int x, ...) {
  // ERROR: no way to forward variadic arguments.
  void bar(x, ...);
}
```

### 地址计算

对 memref 元素的访问被转换为对描述符所指向缓冲区中元素的访问。元素在缓冲区中的位置通过以行主序（row-major order）线性化 memref 索引来计算（字典序第一个索引变化最慢，类似于 C，但考虑了步幅）。线性地址的计算作为 LLVM IR 方言中的算术操作发出。步幅从 memref 描述符中提取。

示例：

对带有索引的 memref 进行访问：

```mlir
%0 = memref.load %m[%1,%2,%3,%4] : memref<?x?x4x8xf32, offset: ?>
```

被转换为以下等价代码：

```mlir
// Compute the linearized index from strides.
// When strides or, in absence of explicit strides, the corresponding sizes are
// dynamic, extract the stride value from the descriptor.
%stride1 = llvm.extractvalue[4, 0] : !llvm.struct<(ptr, ptr, i64,
                                                   array<4xi64>, array<4xi64>)>
%addr1 = arith.muli %stride1, %1 : i64

// When the stride or, in absence of explicit strides, the trailing sizes are
// known statically, this value is used as a constant. The natural value of
// strides is the product of all sizes following the current dimension.
%stride2 = llvm.mlir.constant(32 : index) : i64
%addr2 = arith.muli %stride2, %2 : i64
%addr3 = arith.addi %addr1, %addr2 : i64

%stride3 = llvm.mlir.constant(8 : index) : i64
%addr4 = arith.muli %stride3, %3 : i64
%addr5 = arith.addi %addr3, %addr4 : i64

// Multiplication with the known unit stride can be omitted.
%addr6 = arith.addi %addr5, %4 : i64

// If the linear offset is known to be zero, it can also be omitted. If it is
// dynamic, it is extracted from the descriptor.
%offset = llvm.extractvalue[2] : !llvm.struct<(ptr, ptr, i64,
                                               array<4xi64>, array<4xi64>)>
%addr7 = arith.addi %addr6, %offset : i64

// All accesses are based on the aligned pointer.
%aligned = llvm.extractvalue[1] : !llvm.struct<(ptr, ptr, i64,
                                                array<4xi64>, array<4xi64>)>

// Get the address of the data pointer.
%ptr = llvm.getelementptr %aligned[%addr7]
     : !llvm.struct<(ptr, ptr, i64, array<4xi64>, array<4xi64>)> -> !llvm.ptr

// Perform the actual load.
%0 = llvm.load %ptr : !llvm.ptr -> f32
```

对于存储操作，地址计算代码完全相同，只有实际的存储操作不同。

注意：转换在发出 memref 访问时不执行任何形式的公共子表达式消除。

### 实用类

许多到 LLVM 方言的转换共用的实用类可在 `lib/Conversion/LLVMCommon` 下找到，包括：

-   `LLVMConversionTarget` 将所有 LLVM 方言操作指定为合法的；
-   `LLVMTypeConverter` 实现上述默认类型转换；
-   `ConvertOpToLLVMPattern` 通过 LLVM 方言特定功能扩展转换模式类；
-   `VectorConvertOpToLLVMPattern` 扩展前一个类，在操作之前自动将高维向量上的操作展开为一维向量操作列表；
-   `StructBuilder` 为构建创建或访问 LLVM 方言结构体类型值的 IR 提供便捷 API；它由 `MemRefDescriptor`、`UrankedMemrefDescriptor` 和 `ComplexBuilder` 为可转换为 LLVM 方言结构体类型的内置类型派生。

## 翻译为 LLVM IR

包含 `llvm.func`、`llvm.mlir.global` 和 `llvm.metadata` 操作的 MLIR 模块可以使用以下方案翻译为 LLVM IR 模块：

-   模块级全局变量被翻译为 LLVM IR 全局值；
-   模块级元数据被翻译为 LLVM IR 元数据，随后可以用特定操作上定义的附加元数据进行扩充；
-   所有函数在模块中声明，以便可以引用它们；
-   每个函数然后单独翻译，并可以访问 MLIR 与 LLVM IR 全局变量、元数据和函数之间的完整映射；
-   在函数内，按拓扑顺序遍历基本块并将其翻译为 LLVM IR 基本块。在每个基本块中，为每个块参数创建 PHI 节点，但不连接到其源块；
-   在每个块内，按顺序翻译操作。每个操作可以访问与函数相同的映射，以及 MLIR 和 LLVM IR 值之间的映射（包括 PHI 节点）。包含区域的操作负责翻译其包含的区域；
-   在翻译函数中的操作之后，此函数中块的 PHI 节点被连接到其源值（现在已可用）。

翻译机制通过方言接口 `LLVMTranslationDialectInterface` 为将自定义操作翻译为 LLVM IR 提供扩展钩子：

-   `convertOperation` 在给定 `IRBuilderBase` 和各种映射的情况下，将属于当前方言的操作翻译为 LLVM IR；
-   `amendOperation` 如果某个操作包含属于当前方言的方言属性，则对该操作执行附加动作，例如设置指令级元数据。

包含想要翻译为 LLVM IR 的操作或属性的方言必须提供此接口的实现并将其注册到系统中。请注意，注册可以在不创建方言的情况下进行，例如在单独的库中，以避免"主"方言库依赖 LLVM IR 库。这些方法的实现可以使用提供给它们的 [`ModuleTranslation`](https://mlir.llvm.org/doxygen/classmlir_1_1LLVM_1_1ModuleTranslation.html) 对象，该对象保存翻译状态并包含众多实用工具。

请注意，此扩展机制是*有意限制的*。LLVM IR 有一组小而相对稳定的指令和类型，MLIR 打算对其进行完整建模。因此，扩展机制仅为更频繁扩展的 LLVM IR 构造（内联函数和元数据）提供。扩展机制的主要目标是支持内联函数集，例如表示特定指令集的内联函数。扩展机制不允许自定义类型或块翻译，也不支持自定义模块级操作。此类转换应在 MLIR 内执行，以对应的 MLIR 构造为目标。

## 从 LLVM IR 翻译

实验性流程允许将 LLVM IR 的相当有限的子集导入 MLIR，生成 LLVM 方言操作。

```
  mlir-translate -import-llvm filename.ll
```
