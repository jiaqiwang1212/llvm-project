# SPIR-V 方言（Dialect）到 LLVM 方言转换手册

本手册描述从 [SPIR-V 方言](Dialects/SPIR-V.md)到 [LLVM 方言](Dialects/LLVM.md)的转换过程。阅读本手册前，需要对两者均有一定了解。手册将介绍在 LLVM 方言中对 SPIR-V 概念建模时的设计决策。此转换工作仍在进行中，随着更多功能的实现，内容将持续扩充。

可通过调用相应的转换通道（pass）来执行转换：

```shell
mlir-opt -convert-spirv-to-llvm <filename.mlir>
```

该通道将按本文档所述，对 SPIR-V 操作（op）的类型和操作进行转换。

[TOC]

## 类型转换

本节描述 SPIR-V 方言类型如何映射到 LLVM 方言。

### 标量类型

SPIR-V 方言          | LLVM 方言
:------------:       | :-----------------:
`i<bitwidth>`        | `!llvm.i<bitwidth>`
`si<bitwidth>`       | `!llvm.i<bitwidth>`
`ui<bitwidth>`       | `!llvm.i<bitwidth>`
`f16`                | `f16`
`f32`                | `f32`
`f64`                | `f64`

### 向量类型

SPIR-V 方言                        | LLVM 方言
:-------------------------------:  | :-------------------------------:
`vector<<count> x <scalar-type>>` | `vector<<count> x <scalar-type>>`

### 指针类型

SPIR-V 指针同时包含一个存储类（Storage Class）。目前转换**不**考虑存储类。

SPIR-V 方言                                            | LLVM 方言
:-------------------------------------------:          | :-------------------------:
`!spirv.ptr< <element-type>, <storage-class> >`        | `!llvm.ptr`

### 数组类型

SPIR-V 区分数组类型与运行时数组类型，后者的长度在编译时未知。在 LLVM 中，可以对数组末尾之后的地址进行索引，因此运行时数组可用零长度数组类型来实现。

此外，SPIR-V 支持数组步长（array stride）的概念。目前仅支持自然步长（基于 [`VulkanLayoutUtils`][VulkanLayoutUtils]），同样映射到 LLVM 数组。

SPIR-V 方言                                  | LLVM 方言
:------------------------------------:       | :-------------------------------------:
`!spirv.array<<count> x <element-type>>`     | `!llvm.array<<count> x <element-type>>`
`!spirv.rtarray< <element-type> >`           | `!llvm.array<0 x <element-type>>`

### 结构体类型

SPIR-V 结构体类型的成员可能带有装饰（decoration）和偏移量信息。目前**不**支持结构体成员装饰的转换。更多信息请参见[装饰转换](#Decorations-conversion)一节。

通常情况下，我们期望每个结构体成员都具有自然大小和对齐方式。然而，在某些场景（例如图形领域）中，可能需要在特定偏移量处显式放置结构体成员。这种情况目前**不**受支持。因此，我们遵循以下映射规则：

*   没有偏移量的结构体被建模为 LLVM 紧凑结构体（packed structures）。

*   具有自然偏移量（即偏移量等于前面所有结构体成员的累计大小，或为自然对齐值）的结构体映射为自然填充结构体。

*   具有非自然偏移量（即偏移量不等于前面成员累计大小）的结构体**不**受支持。在这种情况下，可以用填充字段（如整数）来模拟偏移量，但这样的设计在转换涉及内存寻址的操作（op）时需要重新计算索引。

SPIR-V 结构体转换示例如下：```mlir !spirv.struct<i8, i32> =>
!llvm.struct<packed (i8, i32)> !spirv.struct<i8 [0], i32 [4]> => !llvm.struct<(i8,
i32)>

// error !spirv.struct<i8 [0], i32 [8]> ```

### 未实现的类型

上述未明确列出的其他类型不受转换支持，包括 `ImageType` 和 `MatrixType`。

## 操作转换

本节描述 SPIR-V 方言操作如何转换为 LLVM 方言，列出了已可用的转换模式，以及仍在进行中的工作。

此外，还有多个操作在 LLVM 中没有明确的对应关系，需要在社区中逐案讨论其转换方案。

### 算术操作

SPIR-V 算术操作在 LLVM 方言中大多有直接对应，例外情况（如 `spirv.SMod` 和 `spirv.FMod`）较少。

SPIR-V 方言操作 | LLVM 方言操作
:---------------: | :-------------:
`spirv.FAdd`        | `llvm.fadd`
`spirv.FDiv`        | `llvm.fdiv`
`spirv.FNegate`     | `llvm.fneg`
`spirv.FMul`        | `llvm.fmul`
`spirv.FRem`        | `llvm.frem`
`spirv.FSub`        | `llvm.fsub`
`spirv.IAdd`        | `llvm.add`
`spirv.IMul`        | `llvm.mul`
`spirv.ISub`        | `llvm.sub`
`spirv.SDiv`        | `llvm.sdiv`
`spirv.SRem`        | `llvm.srem`
`spirv.UDiv`        | `llvm.udiv`
`spirv.UMod`        | `llvm.urem`

### 位操作

SPIR-V 提供了一系列位操作，映射到 LLVM 方言操作、内建函数（intrinsics），或具有特定转换模式。

#### 直接转换

与算术操作类似，大多数位操作在 LLVM 中都有语义等价的操作：

SPIR-V 方言操作 | LLVM 方言操作
:---------------: | :-------------:
`spirv.BitwiseAnd`  | `llvm.and`
`spirv.BitwiseOr`   | `llvm.or`
`spirv.BitwiseXor`  | `llvm.xor`

部分位操作可以用 LLVM 内建函数建模：

SPIR-V 方言操作 | LLVM 方言内建函数
:---------------: | :--------------------:
`spirv.BitCount`    | `llvm.intr.ctpop`
`spirv.BitReverse`  | `llvm.intr.bitreverse`

#### `spirv.Not`

`spirv.Not` 用全位置 1 的掩码与操作数执行 `xor` 操作来建模。

```mlir
                            %mask = llvm.mlir.constant(-1 : i32) : i32
%0 = spirv.Not %op : i32  =>  %0  = llvm.xor %op, %mask : i32
```

#### 位字段操作

SPIR-V 方言包含三个位字段操作：`spirv.BitFieldInsert`、`spirv.BitFieldSExtract` 和 `spirv.BitFieldUExtract`。本节将先概述这些操作的转换模式设计，再逐一描述各自的转换。

这三个操作均以 `base`、`offset` 和 `count`（`spirv.BitFieldInsert` 还有 `insert`）为参数。有两个重要事项需要注意：

*   `offset` 和 `count` 始终是标量。这意味着可能出现以下情况：

    ```mlir
    %0 = spirv.BitFieldSExtract %base, %offset, %count : vector<2xi32>, i8, i8
    ```

    为了能够使用下述转换算法，所有操作数必须具有相同的类型和位宽。这需要将 `offset` 和 `count` 广播为向量，例如对上例：

    ```mlir
    // Broadcasting offset
    %offset0 = llvm.mlir.undef : vector<2xi8>
    %zero = llvm.mlir.constant(0 : i32) : i32
    %offset1 = llvm.insertelement %offset, %offset0[%zero : i32] : vector<2xi8>
    %one = llvm.mlir.constant(1 : i32) : i32
    %vec_offset = llvm.insertelement  %offset, %offset1[%one : i32] : vector<2xi8>

    // Broadcasting count
    // ...
    ```

*   `offset` 和 `count` 的位宽可能与 `base` 不同。在这种情况下，两个操作数都需要进行零扩展（因为按规范它们被视为无符号数）或截断。对上例：

    ```mlir
    // Zero extending offset after broadcasting
    %res_offset = llvm.zext %vec_offset: vector<2xi8> to vector<2xi32>
    ```

    另外请注意，即使 `offset` 或 `count` 的位宽大于 `base`，截断仍然允许。这是因为这些操作在 `offset` 和 `count` 小于 `base` 大小时具有明确定义的行为，自然地限制了 `offset` 和 `count` 可取的值上界为 64，这可以用不到 8 位来表达。

了解以上两种情况后，我们可以继续进行操作及其操作数的转换。

##### `spirv.BitFieldInsert`

该操作实现为一系列 LLVM 方言操作。第一步是创建一个在 [`offset`, `offset` + `count` - 1] 之外所有位都置 1 的掩码，然后从 `base` 中提取该范围之外的未变更位，最后与左移后的 `insert` 做 `or` 运算得到结果。

```mlir
// Create mask
// %minus_one = llvm.mlir.constant(-1 : i32) : i32
// %t0        = llvm.shl %minus_one, %count : i32
// %t1        = llvm.xor %t0, %minus_one : i32
// %t2        = llvm.shl %t1, %offset : i32
// %mask      = llvm.xor %t2, %minus_one : i32

// Extract unchanged bits from the Base
// %new_base  = llvm.and %base, %mask : i32

// Insert new bits
// %sh_insert = llvm.shl %insert, %offset : i32
// %res       = llvm.or %new_base, %sh_insert : i32
%res = spirv.BitFieldInsert %base, %insert, %offset, %count : i32, i32, i32
```

##### `spirv.BitFieldSExtract`

实现 `spirv.BitFieldSExtract` 时，将 `base` 向左移动 [sizeof(`base`) - (`count` + `offset`)] 位，使得位于 `offset` + `count` - 1 处的位成为最高有效位，然后再向右移动，用符号位填充高位。

```mlir
// Calculate the amount to shift left.
// %size    = llvm.mlir.constant(32 : i32) : i32
// %t0      = llvm.add %count, %offset : i32
// %t1      = llvm.sub %size, %t0 : i32

// Shift left and then right to extract the bits
// %sh_left = llvm.shl %base, %t1 : i32
// %t2      = llvm.add %offset, %t1 : i32
// %res     = llvm.ashr %sh_left, %t2 : i32
%res = spirv.BitFieldSExtract %base, %offset, %count : i32, i32, i32
```

##### `spirv.BitFieldUExtract`

该操作采用与 `spirv.BitFieldInsert` 类似的模式。首先创建一个在 [0, `count` - 1] 处置 1 的掩码，然后将 `base` 右移 `offset` 位并应用掩码。

```mlir
// Create a mask
// %minus_one = llvm.mlir.constant(-1 : i32) : i32
// %t0        = llvm.shl %minus_one, %count : i32
// mask       = llvm.xor  %t0, %minus_one : i32

// Shift Base and apply mask
// %sh_base   = llvm.lshr %base, %offset : i32
// %res       = llvm.and %sh_base, %mask : i32
%res = spirv.BitFieldUExtract %base, %offset, %count : i32, i32, i32
```

### 类型转换操作

#### 直接转换

SPIR-V 方言操作 | LLVM 方言操作
:---------------: | :-------------:
`spirv.ConvertFToS` | `llvm.fptosi`
`spirv.ConvertFToU` | `llvm.fptoui`
`spirv.ConvertSToF` | `llvm.sitofp`
`spirv.ConvertUToF` | `llvm.uitofp`

#### spirv.Bitcast

该操作在 LLVM 中有直接对应：`llvm.bitcast`。由于它除了支持常规的数值标量或向量类型的位模式保留转换外，还支持指针到指针的位模式保留类型转换，因此需要单独处理。

#### 特殊情况

特殊情况包括 `spirv.FConvert`、`spirv.SConvert` 和 `spirv.UConvert`。这些操作本质上是截断或扩展操作。设操作数组件位宽为 A，结果组件位宽为 R，则使用以下映射：

##### `spirv.FConvert`

情况  | LLVM 方言操作
:---: | :-------------:
A < R | `llvm.fpext`
A > R | `llvm.fptrunc`

##### `spirv.SConvert`

情况  | LLVM 方言操作
:---: | :-------------:
A < R | `llvm.sext`
A > R | `llvm.trunc`

##### `spirv.UConvert`

情况  | LLVM 方言操作
:---: | :-------------:
A < R | `llvm.zext`
A > R | `llvm.trunc`

A = R 的情况不可能出现，基于 SPIR-V 方言规范：

> 组件位宽不能等于结果类型的组件位宽。

### 比较操作

SPIR-V 比较操作映射到 LLVM 的 `icmp` 和 `fcmp` 操作。

SPIR-V 方言操作                  | LLVM 方言操作
:--------------------------:     | :---------------:
`spirv.IEqual`                   | `llvm.icmp "eq"`
`spirv.INotEqual`                | `llvm.icmp "ne"`
`spirv.FOrdEqual`                | `llvm.fcmp "oeq"`
`spirv.FOrdGreaterThan`          | `llvm.fcmp "ogt"`
`spirv.FOrdGreaterThanEqual`     | `llvm.fcmp "oge"`
`spirv.FOrdLessThan`             | `llvm.fcmp "olt"`
`spirv.FOrdLessThanEqual`        | `llvm.fcmp "ole"`
`spirv.FOrdNotEqual`             | `llvm.fcmp "one"`
`spirv.FUnordEqual`              | `llvm.fcmp "ueq"`
`spirv.FUnordGreaterThan`        | `llvm.fcmp "ugt"`
`spirv.FUnordGreaterThanEqual`   | `llvm.fcmp "uge"`
`spirv.FUnordLessThan`           | `llvm.fcmp "ult"`
`spirv.FUnordLessThanEqual`      | `llvm.fcmp "ule"`
`spirv.FUnordNotEqual`           | `llvm.fcmp "une"`
`spirv.SGreaterThan`             | `llvm.icmp "sgt"`
`spirv.SGreaterThanEqual`        | `llvm.icmp "sge"`
`spirv.SLessThan`                | `llvm.icmp "slt"`
`spirv.SLessThanEqual`           | `llvm.icmp "sle"`
`spirv.UGreaterThan`             | `llvm.icmp "ugt"`
`spirv.UGreaterThanEqual`        | `llvm.icmp "uge"`
`spirv.ULessThan`                | `llvm.icmp "ult"`
`spirv.ULessThanEqual`           | `llvm.icmp "ule"`

### 复合操作

目前转换支持 `spirv.CompositeExtract` 和 `spirv.CompositeInsert` 的重写模式。对于这两个操作，我们区分两种情况：复合对象为向量时，以及复合对象为非向量类型（即结构体、数组或运行时数组）时。

复合类型     | SPIR-V 方言操作         | LLVM 方言操作
:----------: | :--------------------:  | :-------------------:
向量         | `spirv.CompositeExtract`| `llvm.extractelement`
向量         | `spirv.CompositeInsert` | `llvm.insertelement`
非向量       | `spirv.CompositeExtract`| `llvm.extractvalue`
非向量       | `spirv.CompositeInsert` | `llvm.insertvalue`

### `spirv.EntryPoint` 和 `spirv.ExecutionMode`

首先需要指出，LLVM 中没有入口点（entry point）的直接表示。目前采用以下方式处理：

*   `spirv.EntryPoint` 直接被移除。

*   相比之下，`spirv.ExecutionMode` 可能包含关于入口点的重要信息。例如，`LocalSize` 提供了可复用的工作组大小信息。

    为保留这些信息，`spirv.ExecutionMode` 被转换为一个存储执行模式 ID 及其关联变量的结构体全局变量。在 C 中，该结构体的结构如下所示：

    ```c
    // No values are associated      // There are values that are associated
    // with this entry point.        // with this entry point.
    struct {                         struct {
      int32_t executionMode;             int32_t executionMode;
    };                                   int32_t values[];
                                     };
    ```

    ```mlir
    // spirv.ExecutionMode @empty "ContractionOff"
    llvm.mlir.global external constant @{{.*}}() : !llvm.struct<(i32)> {
      %0   = llvm.mlir.undef : !llvm.struct<(i32)>
      %1   = llvm.mlir.constant(31 : i32) : i32
      %ret = llvm.insertvalue %1, %0[0] : !llvm.struct<(i32)>
      llvm.return %ret : !llvm.struct<(i32)>
    }
    ```

### 逻辑操作

逻辑操作遵循与位操作类似的模式，区别在于它们作用于 `i1` 或 `i1` 向量值。以下映射用于模拟 SPIR-V 操作的行为：

SPIR-V 方言操作           | LLVM 方言操作
:-------------------:     | :--------------:
`spirv.LogicalAnd`        | `llvm.and`
`spirv.LogicalOr`         | `llvm.or`
`spirv.LogicalEqual`      | `llvm.icmp "eq"`
`spirv.LogicalNotEqual`   | `llvm.icmp "ne"`

`spirv.LogicalNot` 与位操作 `spirv.Not` 具有相同的转换模式，用全位置 1 的掩码执行 `xor` 操作来建模：

```mlir
                                  %mask = llvm.mlir.constant(-1 : i1) : i1
%0 = spirv.LogicalNot %op : i1  =>  %0    = llvm.xor %op, %mask : i1
```

### 内存操作

本节描述 SPIR-V 方言中涉及内存的操作的转换模式。

#### `spirv.AccessChain`

`spirv.AccessChain` 映射到 `llvm.getelementptr` 操作。为了创建有效的 LLVM 操作，我们还在 `spirv.AccessChain` 的索引列表中添加一个 0 索引以穿越指针。

```mlir
// Access the 1st element of the array
%i   = spirv.Constant 1: i32
%var = spirv.Variable : !spirv.ptr<!spirv.struct<f32, !spirv.array<4xf32>>, Function>
%el  = spirv.AccessChain %var[%i, %i] : !spirv.ptr<!spirv.struct<f32, !spirv.array<4xf32>>, Function>, i32, i32

// Corresponding LLVM dialect code
%i   = ...
%var = ...
%0   = llvm.mlir.constant(0 : i32) : i32
%el  = llvm.getelementptr %var[%0, %i, %i] : (!llvm.ptr, i32, i32, i32), !llvm.struct<packed (f32, array<4 x f32>)>
```

#### `spirv.Load` 和 `spirv.Store`

这两个操作被转换为其 LLVM 对应项：`llvm.load` 和 `llvm.store`。如果操作带有内存访问属性（memory access attribute），则根据属性值有以下几种情况：

*   **Aligned**：对齐值传递给 LLVM 操作构建器，例如：`mlir
    // llvm.store %ptr, %val {alignment = 4 : i64} : !llvm.ptr spirv.Store
    "Function" %ptr, %val ["Aligned", 4] : f32`
*   **None**：与没有内存访问属性的情况相同。

*   **Nontemporal**：设置 `nontemporal` 标志，例如：`mlir // %res =
    llvm.load %ptr {nontemporal} : !llvm.ptr %res = spirv.Load "Function"
    %ptr ["Nontemporal"] : f32`

*   **Volatile**：将操作标记为 `volatile`，例如：`mlir // %res =
    llvm.load volatile %ptr : !llvm.ptr f32> %res = spirv.Load "Function" %ptr
    ["Volatile"] : f32` 否则转换失败，因为其他情况（`MakePointerAvailable`、`MakePointerVisible`、`NonPrivatePointer`）尚不支持。

#### `spirv.GlobalVariable` 和 `spirv.mlir.addressof`

`spirv.GlobalVariable` 用 `llvm.mlir.global` 操作建模。但需要指出一个差异。

在 SPIR-V 方言中，全局变量返回一个指针，而在 LLVM 方言中，全局变量保存实际的值。这一差异由 `spirv.mlir.addressof` 和 `llvm.mlir.addressof` 操作处理，两者都返回指针并用于引用全局变量。

```mlir
// Original SPIR-V module
spirv.module Logical GLSL450 {
  spirv.GlobalVariable @struct : !spirv.ptr<!spirv.struct<f32, !spirv.array<10xf32>>, Private>
  spirv.func @func() -> () "None" {
    %0 = spirv.mlir.addressof @struct : !spirv.ptr<!spirv.struct<f32, !spirv.array<10xf32>>, Private>
    spirv.Return
  }
}

// Converted result
module {
  llvm.mlir.global private @struct() : !llvm.struct<packed (f32, [10 x f32])>
  llvm.func @func() {
    %0 = llvm.mlir.addressof @struct : !llvm.ptr
    llvm.return
  }
}
```

SPIR-V 到 LLVM 的转换不涉及工作组（workgroup）的建模。因此，转换范围仅限于当前调用（invocation）。这意味着支持 `Input`、`Output` 和 `Private` 存储类的全局变量指针。此外，`StorageBuffer` 存储类被允许用于执行 [SPIR-V CPU Runner 测试](#spir-v-cpu-runner-tests)。

另外，用于指定描述符集和绑定编号的 `bind` 以及指定 SPIR-V `BuiltIn` 装饰的 `built_in` 没有对应的 LLVM 方言转换。

目前 `llvm.mlir.global` 的创建规则是：对 `Private` 存储类使用 `private` 链接，对其他存储类使用 `External` 链接，基于 SPIR-V 规范：

> 默认情况下，函数和全局变量对模块私有，不能被其他模块访问。但模块可以被编写为导出或导入函数及全局（模块范围）变量。

如果全局变量的指针具有 `Input` 存储类，则在 LLVM 操作中添加 `constant` 标志：

```mlir
spirv.GlobalVariable @var : !spirv.ptr<f32, Input>    =>    llvm.mlir.global external constant @var() : f32
```

#### `spirv.Variable`

根据 SPIR-V 方言规范，`spirv.Variable` 在内存中分配一个对象并返回指向它的指针，该指针可与 `spirv.Load` 和 `spirv.Store` 配合使用。它也是一个函数级别的变量。

`spirv.Variable` 被建模为 `llvm.alloca` 操作。如果有初始化，则使用附加的 store 指令。注意对于数组和结构体，没有初始化，因为 LLVM 方言不支持这些类型的常量（TODO）。此外，目前只能通过 `spirv.Constant` 进行初始化。

```mlir
// Conversion of VariableOp without initialization
                                                               %size = llvm.mlir.constant(1 : i32) : i32
%res = spirv.Variable : !spirv.ptr<vector<3xf32>, Function>   =>   %res  = llvm.alloca  %size x vector<3xf32> : (i32) -> !llvm.ptr

// Conversion of VariableOp with initialization
                                                               %c    = llvm.mlir.constant(0 : i64) : i64
%c   = spirv.Constant 0 : i64                                    %size = llvm.mlir.constant(1 : i32) : i32
%res = spirv.Variable init(%c) : !spirv.ptr<i64, Function>    =>   %res  = llvm.alloca %[[SIZE]] x i64 : (i32) -> !llvm.ptr
                                                               llvm.store %c, %res : i64, !llvm.ptr
```

注意，如果代码存在某些作用域，单纯转换为 `alloca` 可能不够。例如，如果将循环中执行的操作转换为 `alloca`，可能发生栈溢出。对于这种情况，可以使用 `stacksave`/`stackrestore` 对（TODO）。

### 可直接转换的杂项操作

有多个 SPIR-V 操作不属于特定类别，但可以直接转换到 LLVM 方言，本节对其转换进行介绍。

SPIR-V 方言操作 | LLVM 方言操作
:---------------: | :---------------:
`spirv.Select`      | `llvm.select`
`spirv.Undef`       | `llvm.mlir.undef`

### 移位操作

移位操作作用于两个操作数：`shift`（移位量）和 `base`（基础值）。

在 SPIR-V 方言中，`shift` 和 `base` 可以具有不同的位宽。而在 LLVM 方言中，`base` 和 `shift` 必须具有相同的位宽。这导致以下转换：

*   如果 `base` 与 `shift` 位宽相同，转换直接进行。

*   如果 `base` 的位宽大于 `shift`，则先对 `shift` 进行符号扩展或零扩展，然后将扩展后的值传递给移位操作。

*   否则，转换被认为是非法的。

```mlir
// Shift without extension
%res0 = spirv.ShiftRightArithmetic %0, %2 : i32, i32  =>  %res0 = llvm.ashr %0, %2 : i32

// Shift with extension
                                                        %ext  = llvm.sext %1 : i16 to i32
%res1 = spirv.ShiftRightArithmetic %0, %1 : i32, i16  =>  %res1 = llvm.ashr %0, %ext: i32
```

### `spirv.Constant`

目前 `spirv.Constant` 转换**仅**支持标量和向量常量。

#### 映射

`spirv.Constant` 映射到 `llvm.mlir.constant`。这是一个直接的转换模式，其中参数为有符号或无符号整数时需要特殊处理。

#### 特殊情况

SPIR-V 常量可以是有符号或无符号整数。由于 LLVM 方言没有符号性语义，需要单独处理这种情况。

转换将常量值属性（attribute）强制转换为无符号整数或无符号整数向量。这是正确的，因为在 SPIR-V 和 LLVM 中，如何解释一个整数数字同样由操作码（opcode）决定。然而在实际硬件实现中，可能会出现意外行为。因此最好逐案处理，因为转换的目的并不是覆盖所有可能的边界情况。

```mlir
// %0 = llvm.mlir.constant(0 : i8) : i8
%0 = spirv.Constant  0 : i8

// %1 = llvm.mlir.constant(dense<[2, 3, 4]> : vector<3xi32>) : vector<3xi32>
%1 = spirv.Constant dense<[2, 3, 4]> : vector<3xui32>
```

### 未实现的操作

以下操作目前不支持：

*   所有原子操作
*   所有组操作
*   所有矩阵操作
*   所有 CL 操作

以及：

*   spirv.CompositeConstruct
*   spirv.ControlBarrier
*   spirv.CopyMemory
*   spirv.FMod
*   spirv.GL.Acos
*   spirv.GL.Asin
*   spirv.GL.Atan
*   spirv.GL.Cosh
*   spirv.GL.FSign
*   spirv.GL.SAbs
*   spirv.GL.Sinh
*   spirv.GL.SSign
*   spirv.MemoryBarrier
*   spirv.mlir.referenceof
*   spirv.SMod
*   spirv.SpecConstant
*   spirv.Unreachable
*   spirv.VectorExtractDynamic

## 控制流转换

### 分支操作

`spirv.Branch` 和 `spirv.BranchConditional` 映射到 `llvm.br` 和 `llvm.cond_br`。`spirv.BranchConditional` 的分支权重映射到 `llvm.cond_br` 对应的 `branch_weights` 属性。当转换为正式的 LLVM 代码时，`branch_weights` 会转换为与条件分支关联的 LLVM 元数据。

### `spirv.FunctionCall`

`spirv.FunctionCall` 映射到 `llvm.call`。例如：

```mlir
%0 = spirv.FunctionCall @foo() : () -> i32    =>    %0 = llvm.call @foo() : () -> f32
spirv.FunctionCall @bar(%0) : (i32) -> ()     =>    llvm.call @bar(%0) : (f32) -> ()
```

### `spirv.mlir.selection` 和 `spirv.mlir.loop`

`spirv.mlir.selection` 和 `spirv.mlir.loop` 内的控制流通过分支操作直接降级（lower）到 LLVM。转换只能应用于所有基本块（block）均可达的选择或循环结构。此外，选择和循环控制属性（如 `Flatten` 或 `Unroll`）目前不受支持。

```mlir
// Conversion of selection
%cond = spirv.Constant true                               %cond = llvm.mlir.constant(true) : i1
spirv.mlir.selection {
  spirv.BranchConditional %cond, ^true, ^false            llvm.cond_br %cond, ^true, ^false

^true:                                                                                              ^true:
  // True block code                                    // True block code
  spirv.Branch ^merge                             =>      llvm.br ^merge

^false:                                               ^false:
  // False block code                                   // False block code
  spirv.Branch ^merge                                     llvm.br ^merge

^merge:                                               ^merge:
  spirv.mlir.merge                                            llvm.br ^continue
}
// Remaining code                                                                           ^continue:
                                                        // Remaining code
```

```mlir
// Conversion of loop
%cond = spirv.Constant true                               %cond = llvm.mlir.constant(true) : i1
spirv.mlir.loop {
  spirv.Branch ^header                                    llvm.br ^header

^header:                                              ^header:
  // Header code                                        // Header code
  spirv.BranchConditional %cond, ^body, ^merge    =>      llvm.cond_br %cond, ^body, ^merge

^body:                                                ^body:
  // Body code                                          // Body code
  spirv.Branch ^continue                                  llvm.br ^continue

^continue:                                            ^continue:
  // Continue code                                      // Continue code
  spirv.Branch ^header                                    llvm.br ^header

^merge:                                               ^merge:
  spirv.mlir.merge                                            llvm.br ^remaining
}
// Remaining code                                     ^remaining:
                                                        // Remaining code
```

## 装饰转换

**注意：这些转换尚未实现**

## GLSL 扩展指令集

本节描述 GLSL 扩展指令集中的 SPIR-V 操作如何映射到 LLVM 方言。

### 直接转换

SPIR-V 方言操作 | LLVM 方言操作
:---------------: | :----------------:
`spirv.GL.Ceil`     | `llvm.intr.ceil`
`spirv.GL.Cos`      | `llvm.intr.cos`
`spirv.GL.Exp`      | `llvm.intr.exp`
`spirv.GL.FAbs`     | `llvm.intr.fabs`
`spirv.GL.Floor`    | `llvm.intr.floor`
`spirv.GL.FMax`     | `llvm.intr.maxnum`
`spirv.GL.FMin`     | `llvm.intr.minnum`
`spirv.GL.Log`      | `llvm.intr.log`
`spirv.GL.Sin`      | `llvm.intr.sin`
`spirv.GL.Sqrt`     | `llvm.intr.sqrt`
`spirv.GL.SMax`     | `llvm.intr.smax`
`spirv.GL.SMin`     | `llvm.intr.smin`

### 特殊情况

`spirv.InverseSqrt` 映射为：

```mlir
                                           %one  = llvm.mlir.constant(1.0 : f32) : f32
%res = spirv.InverseSqrt %arg : f32    =>    %sqrt = "llvm.intr.sqrt"(%arg) : (f32) -> f32
                                           %res  = fdiv %one, %sqrt : f32
```

`spirv.Tan` 映射为：

```mlir
                                   %sin = "llvm.intr.sin"(%arg) : (f32) -> f32
%res = spirv.Tan %arg : f32    =>    %cos = "llvm.intr.cos"(%arg) : (f32) -> f32
                                   %res = fdiv %sin, %cos : f32
```

`spirv.Tanh` 使用等式 `tanh(x) = {exp(2x) - 1}/{exp(2x) + 1}` 建模：

```mlir
                                     %two   = llvm.mlir.constant(2.0: f32) : f32
                                     %2xArg = llvm.fmul %two, %arg : f32
                                     %exp   = "llvm.intr.exp"(%2xArg) : (f32) -> f32
%res = spirv.Tanh %arg : f32     =>    %one   = llvm.mlir.constant(1.0 : f32) : f32
                                     %num   = llvm.fsub %exp, %one : f32
                                     %den   = llvm.fadd %exp, %one : f32
                                     %res   = llvm.fdiv %num, %den : f32
```

## 函数转换及相关操作

本节描述从 SPIR-V 到 LLVM 方言的函数相关操作的转换。

### `spirv.func`

该操作声明或定义一个 SPIR-V 函数，转换为 `llvm.func`。此转换处理函数签名转换，以及将函数控制属性重映射为 LLVM 方言函数的 [`passthrough` 属性](Dialects/LLVM.md/#attribute-pass-through)。

以下映射用于将 [SPIR-V 函数控制属性][SPIRVFunctionAttributes] 转换为 [LLVM 函数属性][LLVMFunctionAttributes]：

SPIR-V 函数控制属性 | LLVM 函数属性
:--------------------------------: | :---------------------------:
None                               | 不传递任何函数属性
Inline                             | `alwaysinline`
DontInline                         | `noinline`
Pure                               | `readonly`
Const                              | `readnone`

### `spirv.Return` 和 `spirv.ReturnValue`

在 LLVM IR 中，函数可以返回 1 个或 0 个值。因此，两个操作都映射到带有或不带有返回值的 `llvm.return`。

## 模块操作

SPIR-V 中的模块（module）具有一个包含单个块的区域（region）。它通过 `spirv.module` 操作定义，同时接受一系列属性（attribute）：

*   寻址模型
*   内存模型
*   版本-能力-扩展（Version-Capability-Extension）属性

`spirv.module` 被转换为 `ModuleOp`，作为 LLVM 操作的封闭范围。目前，SPIR-V 模块属性被忽略。

## SPIR-V CPU Runner 测试

`mlir-runner` 支持通过 SPIR-V 到 LLVM 方言的转换在 CPU 上执行 `gpu` 方言内核，这被称为"SPIR-V CPU Runner"。需要传递 `--link-nested-modules` 标志。目前仅支持单线程内核。

要构建所需的运行时库，请在 `cmake` 中添加以下选项：`-DMLIR_ENABLE_SPIRV_CPU_RUNNER=1`

### 流水线

带有内核和宿主代码的 `gpu` 模块经历以下转换：

*   将 `gpu` 模块转换为 SPIR-V 方言，降低 ABI 属性并更新版本、能力和扩展。

*   通过将启动操作转换为普通函数调用来模拟内核调用。数据从宿主端通过复制到全局变量传递到设备端。这些全局变量在宿主代码和内核代码中都被创建，并在嵌套模块被折叠时链接。

*   通过新的转换路径将 SPIR-V 方言内核转换为 LLVM 方言。

经过这些通道之后，IR 转换为嵌套的 LLVM 模块——一个代表宿主代码的主模块和一个内核模块。这些模块被链接并使用 `ExecutionEngine` 执行。

### 演练

本节提供运行 SPIR-V CPU Runner 测试时 IR 变化的详细概述。首先，假设我们有如下 IR（为简洁起见，省略了部分类型标注和函数实现）：

```mlir
gpu.module @foo {
  gpu.func @bar(%arg: memref<8xi32>) {
    // Kernel code.
    gpu.return
  }
}

func.func @main() {
  // Fill the buffer with some data
  %buffer = memref.alloc : memref<8xi32>
  %data = ...
  call fillBuffer(%buffer, %data)

  "gpu.launch_func"(/*grid dimensions*/, %buffer) {
    kernel = @foo::bar
  }
}
```

将 `gpu` 方言降级到 SPIR-V 方言的结果：

```mlir
spirv.module @__spv__foo /*VCE triple and other metadata here*/ {
  spirv.GlobalVariable @__spv__foo_arg bind(0,0) : ...
  spirv.func @bar() {
    // Kernel code.
  }
  spirv.EntryPoint @bar, ...
}

func.func @main() {
  // Fill the buffer with some data.
  %buffer = memref.alloc : memref<8xi32>
  %data = ...
  call fillBuffer(%buffer, %data)

  "gpu.launch_func"(/*grid dimensions*/, %buffer) {
    kernel = @foo::bar
  }
}
```

然后，对宿主代码应用从标准方言到 LLVM 方言的降级：

```mlir
spirv.module @__spv__foo /*VCE triple and other metadata here*/ {
  spirv.GlobalVariable @__spv__foo_arg bind(0,0) : ...
  spirv.func @bar() {
    // Kernel code.
  }
  spirv.EntryPoint @bar, ...
}

// Kernel function declaration.
llvm.func @__spv__foo_bar() : ...

llvm.func @main() {
  // Fill the buffer with some data.
  llvm.call fillBuffer(%buffer, %data)

  // Copy data to the global variable, call kernel, and copy the data back.
  %addr = llvm.mlir.addressof @__spv__foo_arg_descriptor_set0_binding0 : ...
  "llvm.intr.memcpy"(%addr, %buffer) : ...
  llvm.call @__spv__foo_bar()
  "llvm.intr.memcpy"(%buffer, %addr) : ...

  llvm.return
}
```

最后，SPIR-V 模块被转换为 LLVM，并为链接解析符号名称：

```mlir
module @__spv__foo {
  llvm.mlir.global @__spv__foo_arg_descriptor_set0_binding0 : ...
  llvm.func @__spv__foo_bar() {
    // Kernel code.
  }
}

// Kernel function declaration.
llvm.func @__spv__foo_bar() : ...

llvm.func @main() {
  // Fill the buffer with some data.
  llvm.call fillBuffer(%buffer, %data)

  // Copy data to the global variable, call kernel, and copy the data back.
  %addr = llvm.mlir.addressof @__spv__foo_arg_descriptor_set0_binding0 : ...
  "llvm.intr.memcpy"(%addr, %buffer) : ...
  llvm.call @__spv__foo_bar()
  "llvm.intr.memcpy"(%buffer, %addr) : ...

  llvm.return
}
```

[LLVMFunctionAttributes]: https://llvm.org/docs/LangRef.html#function-attributes
[SPIRVFunctionAttributes]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#_a_id_function_control_a_function_control
[VulkanLayoutUtils]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/LayoutUtils.h
