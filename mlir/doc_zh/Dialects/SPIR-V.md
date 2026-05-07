# SPIR-V 方言（Dialect）

本文档描述了 MLIR 中 SPIR-V 方言（dialect）的设计。文中列举了我们在建模不同 SPIR-V 机制时所做的各种设计选择及其理由。

本文档还以较高层次的方式说明了各个组件在代码中的组织和实现方式，并给出了扩展这些组件的步骤。

本文档假设读者熟悉 SPIR-V。[SPIR-V][Spirv] 是 Khronos Group 用于表示图形着色器（shader）和计算内核（compute kernel）的二进制中间语言（intermediate language）。它被多个 Khronos Group 的 API 采用，包括 Vulkan 和 OpenCL。其完整定义见[人类可读规范][SpirvSpec]；各种 SPIR-V 指令的语法编码在[机器可读语法文件][SpirvGrammar]中。

[TOC]

## 设计准则

SPIR-V 是一种服务双重目的的二进制中间语言：一方面，它是高层语言用于表示图形着色器和计算内核的中间语言（IR）；另一方面，它定义了供硬件驱动程序使用的稳定二进制格式。因此，SPIR-V 的设计原则不仅涉及中间语言，还涉及二进制格式。例如，规整性（regularity）是 SPIR-V 的设计目标之一。所有概念都以 SPIR-V 指令的形式表示，包括声明扩展和能力、定义类型和常量、定义函数、为计算结果附加额外属性等。这种方式有利于驱动程序的二进制编解码，但不一定利于编译器转换。

### 方言设计原则

SPIR-V 方言的主要目标是成为一种合适的中间表示（IR，intermediate representation），以便于编译器转换。尽管我们仍然出于各种原因支持序列化（serializing）和反序列化（deserializing）二进制格式，但在 SPIR-V 方言的设计中，二进制格式及其相关问题所占的比重较小：在偏向 IR 和支持二进制格式之间需要权衡时，我们倾向于前者。

在 IR 方面，SPIR-V 方言旨在以相同的语义层次对 SPIR-V 进行建模。它的目的不是比 SPIR-V 规范更高级或更低级的抽象。这些抽象很容易超出 SPIR-V 的领域，应使用其他合适的方言来建模，以便在各种编译路径之间共享。由于 SPIR-V 的双重目的，SPIR-V 方言与 SPIR-V 规范保持相同语义层次，这也意味着对于大多数功能，我们仍然可以进行直接的序列化和反序列化。

总结来说，SPIR-V 方言遵循以下设计原则：

*   与 SPIR-V 规范保持相同语义层次，对大多数概念和实体进行一对一映射。
*   尽可能采用 SPIR-V 规范的语法，但在利用 MLIR 机制能够带来更好的表示和转换收益时，有意进行偏离。
*   直接支持与 SPIR-V 二进制格式之间的序列化和反序列化。

SPIR-V 被设计为供硬件驱动程序使用，因此其表示非常清晰，但在某些情况下也相当冗长。允许表示上的偏离使我们能够利用 MLIR 机制来减少冗余。

### 方言作用域

SPIR-V 支持由客户端 API 指定的多种执行环境（execution environment）。其主要采用者包括 Vulkan 和 OpenCL。因此，SPIR-V 方言若要在 MLIR 系统中成为 SPIR-V 的合适代理，就应支持多种执行环境。SPIR-V 方言在设计时考虑到了这些因素：它对版本、扩展和能力（capability）提供了适当的支持，并且与 SPIR-V 规范同样具有可扩展性。

## 约定

SPIR-V 方言采用以下 IR 约定：

*   所有 SPIR-V 类型和操作（op）的前缀均为 `spirv.`。
*   扩展指令集中的所有指令将进一步以扩展指令集的前缀加以限定。例如，GLSL 扩展指令集中的所有操作以 `spirv.GL.` 为前缀。
*   直接镜像规范中指令的操作（op）使用 `CamelCase`（驼峰式）命名，名称与指令操作名相同（省略 `Op` 前缀）。例如，`spirv.FMul` 是规范中 `OpFMul` 的直接镜像。此类操作将被序列化和反序列化为一条 SPIR-V 指令。
*   使用 `snake_case`（蛇形命名法）命名的操作，其表示与规范中对应指令（或概念）不同。这些操作主要用于定义 SPIR-V 结构，例如 `spirv.module` 和 `spirv.Constant`。在（反）序列化时，它们可能对应一条或多条指令。
*   使用 `mlir.snake_case` 命名的操作，在二进制格式中没有对应的指令（或概念）。它们是为满足 MLIR 结构性要求而引入的，例如 `spirv.mlir.merge`。在（反）序列化时，它们不对应任何指令。

（TODO：考虑合并后两种情况，并为其采用 `spirv.mlir.` 前缀。）

## 模块（Module）

SPIR-V 模块通过 `spirv.module` 操作定义，该操作包含一个区域（region），区域中有一个块（block）。模块级别的指令，包括函数定义，均放置在该块内。函数使用内置的 `func` 操作定义。

我们选择用专用的 `spirv.module` 操作来建模 SPIR-V 模块，基于以下考虑：

*   它与规范中的 SPIR-V 模块清晰对应。
*   我们可以在模块级别执行 SPIR-V 特定的验证。
*   我们可以附加额外的模块级属性（attribute）。
*   我们可以控制自定义汇编格式。

`spirv.module` 操作的区域不能从外部捕获 SSA 值，无论是隐式还是显式。`spirv.module` 操作的区域对内部可以出现的操作是封闭的：除内置的 `func` 操作外，只能包含 SPIR-V 方言中的操作。`spirv.module` 操作的验证器会强制执行此规则。这有意义地保证了 `spirv.module` 可以作为序列化的入口点和边界。

### 模块级操作

SPIR-V 二进制格式定义了以下[节（section）][SpirvLogicalLayout]：

1.  模块所需的能力（capability）。
1.  模块所需的扩展（extension）。
1.  模块所需的扩展指令集（extended instruction set）。
1.  寻址和内存模型规范。
1.  入口点（entry point）规范。
1.  执行模式（execution mode）声明。
1.  调试指令。
1.  注解/修饰（annotation/decoration）指令。
1.  类型、常量、全局变量。
1.  函数声明。
1.  函数定义。

基本上，一个 SPIR-V 二进制模块包含多条模块级指令，后跟一组函数。这些模块级指令不可或缺，它们可以生成被函数引用的结果 id，尤其是声明与执行环境交互的资源变量。

与二进制格式相比，我们在 SPIR-V 方言中调整了这些模块级 SPIR-V 指令的表示方式：

#### 使用 MLIR 属性（attribute）表示元数据

*   对能力（capability）、扩展（extension）、扩展指令集（extended instruction set）、寻址模型和内存模型的需求，通过 `spirv.module` 的属性（attribute）传达。这样更为合适，因为这些信息是关于执行环境的，直接放在模块操作本身上更易于查询。
*   注解/修饰指令被"折叠"进其所修饰的指令中，以这些操作上的属性（attribute）形式表示。这消除了 SSA 值的潜在前向引用，提升了 IR 的可读性，并使查询注解更加直接。更多讨论见[修饰（Decorations）](#decorations)章节。

#### 使用 MLIR 自定义类型建模类型

*   类型使用 MLIR 内置类型和 SPIR-V 方言特定类型来表示。SPIR-V 方言中没有类型声明操作。更多讨论见后续的[类型（Types）](#types)章节。

#### 统一并本地化常量

*   各种普通常量指令由同一个 `spirv.Constant` 操作表示。这些指令只是针对不同类型的常量；使用一个操作来表示它们可以减少 IR 冗余，并使转换不那么繁琐。
*   普通常量不放置在 `spirv.module` 的区域中；它们被本地化到函数中。这是为了使 SPIR-V 方言中的函数实现隔离和显式捕获。由于属性（attribute）在 `MLIRContext` 中是唯一的，常量的复制代价很低。

#### 采用基于符号（symbol）的全局变量和特化常量

*   全局变量使用 `spirv.GlobalVariable` 操作定义。它们不生成 SSA 值，而是具有符号，并应通过符号引用。若要在函数块中使用全局变量，需要使用 `spirv.mlir.addressof` 将符号转换为 SSA 值。
*   特化常量（specialization constant）使用 `spirv.SpecConstant` 操作定义。与全局变量类似，它们也不生成 SSA 值，也通过符号引用。需要 `spirv.mlir.referenceof` 将符号转换为 SSA 值，以便在函数块中使用。

上述选择使 SPIR-V 方言中的函数实现了隔离和显式捕获。

#### 禁止函数中的隐式捕获

*   在 SPIR-V 规范中，函数支持隐式捕获：它们可以引用模块中定义的 SSA 值。在 SPIR-V 方言中，函数使用 `func` 操作定义，该操作不允许隐式捕获。这对编译器分析和转换更为友好。更多讨论见后续的[函数（Function）](#function)章节。

#### 将入口点和执行模型建模为普通操作

*   一个 SPIR-V 模块可以有多个入口点。这些入口点引用函数和接口变量，不适合将其建模为 `spirv.module` 操作的属性。我们可以将其建模为使用符号引用的普通操作。
*   类似地，执行模式与入口点相关联，我们可以将其建模为 `spirv.module` 区域中的普通操作。

## 修饰（Decorations）

注解/修饰为结果 id 提供额外信息。在 SPIR-V 中，所有指令都可以生成结果 id，包括值计算和类型定义指令。

对于值结果 id 上的修饰，我们只需将对应属性（attribute）附加到生成该 SSA 值的操作上即可。例如，对于以下 SPIR-V：

```spirv
OpDecorate %v1 RelaxedPrecision
OpDecorate %v2 NoContraction
...
%v1 = OpFMul %float %0 %0
%v2 = OpFMul %float %1 %1
```

可以在 SPIR-V 方言中表示为：

```mlir
%v1 = "spirv.FMul"(%0, %0) {RelaxedPrecision: unit} : (f32, f32) -> (f32)
%v2 = "spirv.FMul"(%1, %1) {NoContraction: unit} : (f32, f32) -> (f32)
```

这种方法有利于转换。本质上，这些修饰只是结果 id（及其定义指令）的额外属性。在 SPIR-V 二进制格式中，它们以指令的形式表示。严格遵循 SPIR-V 二进制格式意味着我们需要通过定义-使用链来查找修饰指令并从中查询信息。

对于类型结果 id 上的修饰，请注意，实际上只有由复合类型（如 `OpTypeArray`、`OpTypeStruct`）生成的结果 id 需要出于内存布局目的进行修饰（例如 `ArrayStride`、`Offset` 等）；标量/向量类型在 SPIR-V 中要求唯一。因此，我们可以直接将它们编码在方言特定类型中。

## 类型（Types）

理论上我们可以使用 MLIR 可扩展类型系统定义所有 SPIR-V 类型，但除了表示上的纯粹性，这并不能带来更多好处。相反，我们需要维护代码并投入精力进行美观打印。因此，我们倾向于尽可能使用内置类型。

SPIR-V 方言复用内置的整数、浮点和向量类型：

规范                                  | 方言
:----------------------------------: | :-------------------------------:
`OpTypeBool`                         | `i1`
`OpTypeFloat <bitwidth>`             | `f<bitwidth>`
`OpTypeVector <scalar-type> <count>` | `vector<<count> x <scalar-type>>`

对于整数类型，SPIR-V 方言支持所有符号语义（无符号语义、有符号、无符号），以便于从更高层次方言进行转换。然而，SPIR-V 规范只定义了两种符号语义状态：0 表示无符号或无符号语义，1 表示有符号语义。因此，`iN` 和 `uiN` 都会被序列化为相同的 `OpTypeInt N 0`。在反序列化时，我们始终将 `OpTypeInt N 0` 视为 `iN`。

`mlir::NoneType` 用于 SPIR-V 的 `OpTypeVoid`；内置函数类型用于 SPIR-V 的 `OpTypeFunction` 类型。

SPIR-V 方言定义了以下方言特定类型：

```
spirv-type ::= array-type
             | image-type
             | pointer-type
             | runtime-array-type
             | sampled-image-type
             | struct-type
```

### 数组类型（Array type）

对应 SPIR-V [数组类型][ArrayType]。其语法为：

```
element-type ::= integer-type
               | floating-point-type
               | vector-type
               | spirv-type

array-type ::= `!spirv.array` `<` integer-literal `x` element-type
               (`,` `stride` `=` integer-literal)? `>`
```

例如：

```mlir
!spirv.array<4 x i32>
!spirv.array<4 x i32, stride = 4>
!spirv.array<16 x vector<4 x f32>>
```

### 图像类型（Image type）

对应 SPIR-V [图像类型][ImageType]。其语法为：

```
dim ::= `1D` | `2D` | `3D` | `Cube` | <and other SPIR-V Dim specifiers...>

depth-info ::= `NoDepth` | `IsDepth` | `DepthUnknown`

arrayed-info ::= `NonArrayed` | `Arrayed`

sampling-info ::= `SingleSampled` | `MultiSampled`

sampler-use-info ::= `SamplerUnknown` | `NeedSampler` | `NoSampler`

format ::= `Unknown` | `Rgba32f` | <and other SPIR-V Image Formats...>

image-type ::= `!spirv.image<` element-type `,` dim `,` depth-info `,`
                           arrayed-info `,` sampling-info `,`
                           sampler-use-info `,` format `>`
```

例如：

```mlir
!spirv.image<f32, 1D, NoDepth, NonArrayed, SingleSampled, SamplerUnknown, Unknown>
!spirv.image<f32, Cube, IsDepth, Arrayed, MultiSampled, NeedSampler, Rgba32f>
```

### 指针类型（Pointer type）

对应 SPIR-V [指针类型][PointerType]。其语法为：

```
storage-class ::= `UniformConstant`
                | `Uniform`
                | `Workgroup`
                | <and other storage classes...>

pointer-type ::= `!spirv.ptr<` element-type `,` storage-class `>`
```

例如：

```mlir
!spirv.ptr<i32, Function>
!spirv.ptr<vector<4 x f32>, Uniform>
```

### 运行时数组类型（Runtime array type）

对应 SPIR-V [运行时数组类型][RuntimeArrayType]。其语法为：

```
runtime-array-type ::= `!spirv.rtarray` `<` element-type (`,` `stride` `=` integer-literal)? `>`
```

例如：

```mlir
!spirv.rtarray<i32>
!spirv.rtarray<i32, stride=4>
!spirv.rtarray<vector<4 x f32>>
```

### 采样图像类型（Sampled image type）

对应 SPIR-V [采样图像类型][SampledImageType]。其语法为：

```
sampled-image-type ::= `!spirv.sampled_image<!spirv.image<` element-type `,` dim `,` depth-info `,`
                                                        arrayed-info `,` sampling-info `,`
                                                        sampler-use-info `,` format `>>`
```

例如：

```mlir
!spirv.sampled_image<!spirv.image<f32, Dim1D, NoDepth, NonArrayed, SingleSampled, NoSampler, Unknown>>
!spirv.sampled_image<!spirv.image<i32, Rect, DepthUnknown, Arrayed, MultiSampled, NeedSampler, R8ui>>
```

### 结构类型（Struct type）

对应 SPIR-V [结构类型][StructType]。其语法为：

```
struct-member-decoration ::= integer-literal? spirv-decoration*
struct-type ::= `!spirv.struct<` spirv-type (`[` struct-member-decoration `]`)?
                     (`, ` spirv-type (`[` struct-member-decoration `]`)? `>`
```

例如：

```mlir
!spirv.struct<f32>
!spirv.struct<f32 [0]>
!spirv.struct<f32, !spirv.image<f32, 1D, NoDepth, NonArrayed, SingleSampled, SamplerUnknown, Unknown>>
!spirv.struct<f32 [0], i32 [4]>
```

## 函数（Function）

在 SPIR-V 中，函数构造由多条指令组成，涉及 `OpFunction`、`OpFunctionParameter`、`OpLabel`、`OpFunctionEnd`。

```spirv
// int f(int v) { return v; }
%1 = OpTypeInt 32 0
%2 = OpTypeFunction %1 %1
%3 = OpFunction %1 %2
%4 = OpFunctionParameter %1
%5 = OpLabel
%6 = OpReturnValue %4
     OpFunctionEnd
```

这种构造非常清晰，但相当冗长。它是为驱动程序消费而设计的。在 SPIR-V 方言中逐字复制这种构造几乎没有什么益处。相反，我们复用内置的 `func` 操作来更简洁地表达函数：

```mlir
func.func @f(%arg: i32) -> i32 {
  "spirv.ReturnValue"(%arg) : (i32) -> (i32)
}
```

SPIR-V 函数最多只有一个结果。它不能包含嵌套函数或非 SPIR-V 操作。`spirv.module` 会验证这些要求。

SPIR-V 方言与 SPIR-V 规范在函数方面的一个主要区别是，前者是隔离的，需要显式捕获，而后者允许隐式捕获。在 SPIR-V 规范中，函数可以引用模块中定义的 SSA 值（由常量、全局变量等生成）。SPIR-V 方言调整了常量和全局变量的建模方式，以支持隔离函数。隔离函数对编译器分析和转换更为友好。这也使 SPIR-V 方言能够更好地利用核心基础设施：核心基础设施中的许多功能要求操作是隔离的，例如[贪婪模式重写器（greedy pattern rewriter）][GreedyPatternRewriter]只能作用于从上方隔离的操作。

（TODO：为 SPIR-V 函数创建专用的 `spirv.fn` 操作。）

## 操作（Operations）

在 SPIR-V 中，指令是一个广义概念；SPIR-V 模块只是一系列指令的集合。声明类型、表达计算、注解结果 id、表达控制流等都以指令的形式表示。

这里我们只讨论表达计算的指令，这些指令可以通过 SPIR-V 方言操作来表示。用于声明和定义的模块级指令在 SPIR-V 方言中有不同的表示方式，如前面[模块级操作](#module-level-operations)章节所述。

一条指令从零个或多个操作数（operand）计算零个或一个结果，结果是一个新的结果 id。操作数可以是前面指令生成的结果 id、立即值或枚举类型的某个值。我们可以用 MLIR SSA 值来建模结果 id 操作数和结果；对于立即值和枚举值，可以用 MLIR 属性（attribute）来建模。

例如：

```spirv
%i32 = OpTypeInt 32 0
%c42 = OpConstant %i32 42
...
%3 = OpVariable %i32 Function 42
%4 = OpIAdd %i32 %c42 %c42
```

可以在方言中表示为：

```mlir
%0 = "spirv.Constant"() { value = 42 : i32 } : () -> i32
%1 = "spirv.Variable"(%0) { storage_class = "Function" } : (i32) -> !spirv.ptr<i32, Function>
%2 = "spirv.IAdd"(%0, %0) : (i32, i32) -> i32
```

操作文档使用 TableGen 中的操作定义规范（Op Definition Spec）编写，在每个操作的 ODS 定义中。文档的 Markdown 版本可使用 `mlir-tblgen -gen-doc` 生成，附在[操作定义](#operation-definitions)章节中。

### 扩展指令集中的操作

类似地，扩展指令集是一种在另一个命名空间内导入 SPIR-V 指令的机制。[`GLSL.std.450`][GlslStd450] 是一个提供常用数学例程的扩展指令集，应被支持。我们不将 `OpExtInstImport` 建模为单独的操作，也不使用单个操作来建模所有扩展指令的 `OpExtInst`，而是将扩展指令集中的每条 SPIR-V 指令建模为带有适当名称前缀的独立操作。例如，对于：

```spirv
%glsl = OpExtInstImport "GLSL.std.450"

%f32 = OpTypeFloat 32
%cst = OpConstant %f32 ...

%1 = OpExtInst %f32 %glsl 28 %cst
%2 = OpExtInst %f32 %glsl 31 %cst
```

我们可以表示为：

```mlir
%1 = "spirv.GL.Log"(%cst) : (f32) -> (f32)
%2 = "spirv.GL.Sqrt"(%cst) : (f32) -> (f32)
```

## 控制流（Control Flow）

SPIR-V 二进制格式使用合并指令（`OpSelectionMerge` 和 `OpLoopMerge`）来声明结构化控制流（structured control flow）。它们在控制流分叉前显式声明一个头块（header block），并在控制流随后汇聚的地方声明一个合并块（merge block）。这些块划定了必须嵌套的构造边界，只能以结构化的方式进入和退出。

在 SPIR-V 方言中，我们使用区域（region）来标记结构化控制流构造的边界。通过这种方式，可以更容易地发现属于某个结构化控制流构造的所有块，也更符合 MLIR 系统的习惯。

我们分别引入了 `spirv.mlir.selection` 和 `spirv.mlir.loop` 操作，用于结构化选择（selection）和循环（loop）。合并目标是紧随它们之后的下一个操作。在其区域内部，引入了一个特殊的终止符（terminator）`spirv.mlir.merge`，用于跳转到合并目标并产出值。

### 选择（Selection）

`spirv.mlir.selection` 定义一个选择构造。它包含一个区域。该区域应至少包含两个块：一个选择头块和一个合并块。

*   选择头块应为第一个块，其中应包含 `spirv.BranchConditional` 或 `spirv.Switch` 操作。
*   合并块应为最后一个块，其中只应包含一个 `spirv.mlir.merge` 操作。任何块都可以跳转到合并块以提前退出。

```
               +--------------+
               | header block |                 (may have multiple outgoing branches)
               +--------------+
                    / | \
                     ...


   +---------+   +---------+   +---------+
   | case #0 |   | case #1 |   | case #2 |  ... (may have branches between each other)
   +---------+   +---------+   +---------+


                     ...
                    \ | /
                      v
               +-------------+
               | merge block |                  (may have multiple incoming branches)
               +-------------+
```

例如，对于以下函数：

```c++
void if(bool cond) {
  int x = 0;
  if (cond) {
    x = 1;
  } else {
    x = 2;
  }
  // ...
}
```

将被表示为：

```mlir
func.func @selection(%cond: i1) -> () {
  %zero = spirv.Constant 0: i32
  %one = spirv.Constant 1: i32
  %two = spirv.Constant 2: i32
  %x = spirv.Variable init(%zero) : !spirv.ptr<i32, Function>

  spirv.mlir.selection {
    spirv.BranchConditional %cond, ^then, ^else

  ^then:
    spirv.Store "Function" %x, %one : i32
    spirv.Branch ^merge

  ^else:
    spirv.Store "Function" %x, %two : i32
    spirv.Branch ^merge

  ^merge:
    spirv.mlir.merge
  }

  // ...
}
```

类似地，对于带有 `switch` 语句的函数：

```c++
void switch(int selector) {
  int x = 0;
  switch (selector) {
  case 0:
    x = 2;
    break;
  case 1:
    x = 3;
    break;
  default:
    x = 1;
    break;
  }
  // ...
}
```

将被表示为：

```mlir
func.func @selection(%selector: i32) -> () {
  %zero = spirv.Constant 0: i32
  %one = spirv.Constant 1: i32
  %two = spirv.Constant 2: i32
  %three = spirv.Constant 3: i32
  %var = spirv.Variable init(%zero) : !spirv.ptr<i32, Function>

  spirv.mlir.selection {
    spirv.Switch %selector : i32, [
      default: ^default,
      0: ^case0,
      1: ^case1
    ]
  ^default:
    spirv.Store "Function" %var, %one : i32
    spirv.Branch ^merge

  ^case0:
    spirv.Store "Function" %var, %two : i32
    spirv.Branch ^merge

  ^case1:
    spirv.Store "Function" %var, %three : i32
    spirv.Branch ^merge

  ^merge:
    spirv.mlir.merge
  }

  // ...
}
```

选择可以通过 `spirv.mlir.merge` 产出值。此机制允许在选择区域内定义的值在其外部使用。若没有此机制，已沉入选择区域但在外部使用的值将无法逃逸出去。

例如：

```mlir
func.func @selection(%cond: i1) -> () {
  %zero = spirv.Constant 0: i32
  %var1 = spirv.Variable init(%zero) : !spirv.ptr<i32, Function>
  %var2 = spirv.Variable init(%zero) : !spirv.ptr<i32, Function>

  %yield:2 = spirv.mlir.selection -> i32, i32 {
    spirv.BranchConditional %cond, ^then, ^else

  ^then:
    %one = spirv.Constant 1: i32
    %three = spirv.Constant 3: i32
    spirv.Branch ^merge(%one, %three : i32, i32)

  ^else:
    %two = spirv.Constant 2: i32
    %four = spirv.Constant 4 : i32
    spirv.Branch ^merge(%two, %four : i32, i32)

  ^merge(%merged_1_2: i32, %merged_3_4: i32):
    spirv.mlir.merge %merged_1_2, %merged_3_4 : i32, i32
  }

  spirv.Store "Function" %var1, %yield#0 : i32
  spirv.Store "Function" %var2, %yield#1 : i32

  spirv.Return
}
```

### 循环（Loop）

`spirv.mlir.loop` 定义一个循环构造。它包含一个区域。该区域应至少包含四个块：一个入口块（entry block）、一个循环头块（loop header block）、一个循环继续块（loop continue block）和一个合并块（merge block）。

*   入口块应为第一个块，它应跳转到作为第二个块的循环头块。
*   合并块应为最后一个块，其中只应包含一个 `spirv.mlir.merge` 操作。除入口块外，任何块都可以跳转到合并块以提前退出。
*   继续块应为倒数第二个块，它应有一个跳转到循环头块的分支。
*   循环继续块应是除入口块外唯一跳转到循环头块的块。

```
    +-------------+
    | entry block |           (one outgoing branch)
    +-------------+
           |
           v
    +-------------+           (two incoming branches)
    | loop header | <-----+   (may have one or two outgoing branches)
    +-------------+       |
                          |
          ...             |
         \ | /            |
           v              |
   +---------------+      |   (may have multiple incoming branches)
   | loop continue | -----+   (may have one or two outgoing branches)
   +---------------+

          ...
         \ | /
           v
    +-------------+           (may have multiple incoming branches)
    | merge block |
    +-------------+
```

之所以使用独立的入口块而不是直接将循环头块作为入口块，是为了满足区域的要求：区域的入口块不能有前驱块。合并块的存在使得分支操作可以将其作为后继块引用。这里的循环继续块对应 SPIR-V 规范术语中的"继续构造（continue construct）"；它不代表 SPIR-V 规范中所定义的"继续块（continue block）"，后者是"包含跳转到 `OpLoopMerge` 指令的 Continue Target 的块"。

例如，对于以下函数：

```c++
void loop(int count) {
  for (int i = 0; i < count; ++i) {
    // ...
  }
}
```

将被表示为：

```mlir
func.func @loop(%count : i32) -> () {
  %zero = spirv.Constant 0: i32
  %one = spirv.Constant 1: i32
  %var = spirv.Variable init(%zero) : !spirv.ptr<i32, Function>

  spirv.mlir.loop {
    spirv.Branch ^header

  ^header:
    %val0 = spirv.Load "Function" %var : i32
    %cmp = spirv.SLessThan %val0, %count : i32
    spirv.BranchConditional %cmp, ^body, ^merge

  ^body:
    // ...
    spirv.Branch ^continue

  ^continue:
    %val1 = spirv.Load "Function" %var : i32
    %add = spirv.IAdd %val1, %one : i32
    spirv.Store "Function" %var, %add : i32
    spirv.Branch ^header

  ^merge:
    spirv.mlir.merge
  }
  return
}
```

与选择类似，循环也可以使用 `spirv.mlir.merge` 产出值。此机制允许在循环区域内定义的值在其外部使用。

例如：

```mlir
%yielded = spirv.mlir.loop -> i32 {
  // ...
  spirv.mlir.merge %to_yield : i32
}
```

### Phi 的块参数（Block argument for Phi）

SPIR-V 方言中没有直接的 Phi 操作；SPIR-V 的 `OpPhi` 指令在 SPIR-V 方言中被建模为块参数（block argument）。（参见[设计原理][Rationale]文档中关于"块参数 vs. Phi 节点"的内容。）每个块参数对应 SPIR-V 二进制格式中的一条 `OpPhi` 指令。例如，对于以下 SPIR-V 函数 `foo`：

```spirv
  %foo = OpFunction %void None ...
%entry = OpLabel
  %var = OpVariable %_ptr_Function_int Function
         OpSelectionMerge %merge None
         OpBranchConditional %true %true %false
 %true = OpLabel
         OpBranch %phi
%false = OpLabel
         OpBranch %phi
  %phi = OpLabel
  %val = OpPhi %int %int_1 %false %int_0 %true
         OpStore %var %val
         OpReturn
%merge = OpLabel
         OpReturn
         OpFunctionEnd
```

将被表示为：

```mlir
func.func @foo() -> () {
  %var = spirv.Variable : !spirv.ptr<i32, Function>

  spirv.mlir.selection {
    %true = spirv.Constant true
    spirv.BranchConditional %true, ^true, ^false

  ^true:
    %zero = spirv.Constant 0 : i32
    spirv.Branch ^phi(%zero: i32)

  ^false:
    %one = spirv.Constant 1 : i32
    spirv.Branch ^phi(%one: i32)

  ^phi(%arg: i32):
    spirv.Store "Function" %var, %arg : i32
    spirv.Return

  ^merge:
    spirv.mlir.merge
  }
  spirv.Return
}
```

## 版本、扩展、能力（Version, extensions, capabilities）

SPIR-V 支持版本（version）、扩展（extension）和能力（capability），作为指示目标硬件上各种特性（类型、操作、枚举值）可用性的方式。例如，非均匀组操作（non-uniform group operation）在 v1.3 之前不存在，使用时需要 `GroupNonUniformArithmetic` 等特殊能力。这些可用性信息与[目标环境（target environment）](#target-environment)相关，并在方言转换（dialect conversion）过程中影响模式（pattern）的合法性。

SPIR-V 操作的可用性需求使用[操作接口（op interface）][MlirOpInterface]建模：

*   `QueryMinVersionInterface` 和 `QueryMaxVersionInterface` 用于版本需求
*   `QueryExtensionInterface` 用于扩展需求
*   `QueryCapabilityInterface` 用于能力需求

这些接口声明从 [`SPIRVBase.td`][MlirSpirvBase] 中的 TableGen 定义自动生成。目前所有 SPIR-V 操作都实现了上述接口。

SPIR-V 操作的可用性实现方法从 TableGen 中每个操作和枚举属性的可用性规范自动合成。一个操作需要查看不仅操作码，还有操作数，才能得出其可用性需求。例如，`spirv.ControlBarrier` 在执行作用域（execution scope）为 `Subgroup` 时不需要特殊能力，但如果作用域为 `QueueFamily`，则需要 `VulkanMemoryModel` 能力。

SPIR-V 类型的可用性实现方法在 SPIR-V [类型层次结构][MlirSpirvTypes]中以覆盖（override）的形式手动编写。

这些可用性需求作为 [`SPIRVConversionTarget`](#spirvconversiontarget) 和 [`SPIRVTypeConverter`](#spirvtypeconverter) 的"原料"，按照[目标环境](#target-environment)中的需求执行操作和类型转换。

## 目标环境（Target environment）

SPIR-V 旨在支持由客户端 API 指定的多种执行环境。这些执行环境影响某些 SPIR-V 特性的可用性。例如，[Vulkan 1.1][VulkanSpirv] 实现必须支持 SPIR-V 的 1.0、1.1、1.2 和 1.3 版本，以及 GLSL 的 SPIR-V 扩展指令的 1.0 版本。进一步的 Vulkan 扩展可能启用更多 SPIR-V 指令。

SPIR-V 编译还应考虑执行环境，以便生成对目标环境有效的 SPIR-V 模块。这通过 `spirv.target_env`（`spirv::TargetEnvAttr`）属性传达。它应为 `#spirv.target_env` 属性类型，定义如下：

```
spirv-version    ::= `v1.0` | `v1.1` | ...
spirv-extension  ::= `SPV_KHR_16bit_storage` | `SPV_EXT_physical_storage_buffer` | ...
spirv-capability ::= `Shader` | `Kernel` | `GroupNonUniform` | ...

spirv-extension-list     ::= `[` (spirv-extension-elements)? `]`
spirv-extension-elements ::= spirv-extension (`,` spirv-extension)*

spirv-capability-list     ::= `[` (spirv-capability-elements)? `]`
spirv-capability-elements ::= spirv-capability (`,` spirv-capability)*

spirv-resource-limits ::= dictionary-attribute

spirv-vce-attribute ::= `#` `spirv.vce` `<`
                            spirv-version `,`
                            spirv-capability-list `,`
                            spirv-extensions-list `>`

spirv-vendor-id ::= `AMD` | `NVIDIA` | ...
spirv-device-type ::= `DiscreteGPU` | `IntegratedGPU` | `CPU` | ...
spirv-device-id ::= integer-literal
spirv-device-info ::= spirv-vendor-id (`:` spirv-device-type (`:` spirv-device-id)?)?

spirv-target-env-attribute ::= `#` `spirv.target_env` `<`
                                  spirv-vce-attribute,
                                  (spirv-device-info `,`)?
                                  spirv-resource-limits `>`
```

该属性包含以下字段：

*   一个 `#spirv.vce`（`spirv::VerCapExtAttr`）属性：
    *   目标 SPIR-V 版本。
    *   目标的 SPIR-V 扩展列表。
    *   目标的 SPIR-V 能力列表。
*   目标资源限制（resource limit）的字典（参见 [Vulkan 规范][VulkanResourceLimits]的说明）：
    *   `max_compute_workgroup_invocations`
    *   `max_compute_workgroup_size`

例如：

```
module attributes {
spirv.target_env = #spirv.target_env<
    #spirv.vce<v1.3, [Shader, GroupNonUniform], [SPV_KHR_8bit_storage]>,
    ARM:IntegratedGPU,
    {
      max_compute_workgroup_invocations = 128 : i32,
      max_compute_workgroup_size = dense<[128, 128, 64]> : vector<3xi32>
    }>
} { ... }
```

方言转换框架（dialect conversion framework）将利用 `spirv.target_env` 中的信息，适当过滤目标执行环境中不可用的模式和操作。在以 SPIR-V 为目标时，需要通过提供此属性来创建一个 [`SPIRVConversionTarget`](#spirvconversiontarget)。

## 着色器接口（ABI）

SPIR-V 本身只表达在 GPU 设备上发生的计算。SPIR-V 程序本身不足以在 GPU 上运行工作负载；还需要一个配套的宿主应用程序来管理 SPIR-V 程序所引用的资源并分发工作负载。对于 Vulkan 执行环境，宿主应用程序将使用 Vulkan API 编写。与 CUDA 不同，SPIR-V 程序和 Vulkan 应用程序通常使用不同的前端语言编写，这将两个世界隔离开来。然而它们仍需匹配_接口（interface）_：SPIR-V 程序中声明的用于引用资源的变量，需要在参数上与应用程序所管理的实际资源相匹配。

仍以 Vulkan 为例，Vulkan 中有两种主要资源类型：缓冲区（buffer）和图像（image）。它们用于支持可能在操作类别（加载、存储、原子操作）上有所不同的各种用途。这些用途通过描述符类型（descriptor type）加以区分。（例如，uniform 存储缓冲区描述符只支持加载操作，而存储缓冲区描述符支持加载、存储和原子操作。）Vulkan 为资源使用绑定模型（binding model）：资源与描述符（descriptor）关联，描述符进一步分组到集合（set）中。每个描述符因此有一个集合编号和一个绑定编号。应用程序中的描述符对应 SPIR-V 程序中的变量，其参数必须匹配，包括但不限于集合编号和绑定编号。

除缓冲区和图像外，还有其他由 Vulkan 设置并在 SPIR-V 程序内部引用的数据，例如推送常量（push constant）。它们也有需要在两个世界之间匹配的参数。

接口需求对于 MLIR 中的 SPIR-V 编译路径是外部信息。此外，每个 Vulkan 应用程序可能希望以不同方式处理资源。为了避免重复并共享通用工具，需要定义一个 SPIR-V 着色器接口规范，为 SPIR-V 编译路径提供外部需求和指导。

### 着色器接口属性（Shader interface attributes）

SPIR-V 方言定义了[若干属性][MlirSpirvAbi]用于指定这些接口：

*   `spirv.entry_point_abi` 是一个结构属性（struct attribute），应附加到入口函数（entry function）上。它包含：
    *   `local_size`，用于指定调度时的本地工作组大小。
*   `spirv.interface_var_abi` 是一个属性，应附加到入口函数的每个操作数和结果上。它应为 `#spirv.interface_var_abi` 属性类型，定义如下：

```
spv-storage-class     ::= `StorageBuffer` | ...
spv-descriptor-set    ::= integer-literal
spv-binding           ::= integer-literal
spv-interface-var-abi ::= `#` `spirv.interface_var_abi` `<(` spv-descriptor-set
                          `,` spv-binding `)` (`,` spv-storage-class)? `>`
```

例如：

```
#spirv.interface_var_abi<(0, 0), StorageBuffer>
#spirv.interface_var_abi<(0, 1)>
```

该属性包含以下字段：

*   对应资源变量的描述符集编号。
*   对应资源变量的绑定编号。
*   对应资源变量的存储类（storage class）。

SPIR-V 方言提供了一个 [`LowerABIAttributesPass`][MlirSpirvPasses]，使用这些信息将入口点函数及其 ABI 降低（lower）为符合 Vulkan 验证规则的形式。具体来说：

*   为参数创建 `spirv.GlobalVariable`，并将该参数的所有使用替换为此变量。用于替换的 SSA 值通过 `spirv.mlir.addressof` 操作获得。
*   将 `spirv.EntryPoint` 和 `spirv.ExecutionMode` 操作添加到入口函数的 `spirv.module` 中。

## 序列化与反序列化（Serialization and deserialization）

尽管 SPIR-V 方言的主要目标是作为编译器转换的合适 IR，但能够序列化和反序列化二进制格式仍然出于许多正当原因非常有价值。序列化使 SPIR-V 编译的产物能够被执行环境消费；反序列化则允许我们导入 SPIR-V 二进制模块并对其运行转换。因此，序列化和反序列化从 SPIR-V 方言开发之初就得到了支持。

序列化库提供两个入口点 `mlir::spirv::serialize()` 和 `mlir::spirv::deserialize()`，用于将 MLIR SPIR-V 模块转换为二进制格式及其逆向操作。[代码组织](#code-organization)章节对此有更多说明。

鉴于重点在于转换（这不可避免地意味着对二进制模块的更改），序列化并非被设计为调查 SPIR-V 二进制模块的通用工具，也不保证往返等价（至少目前如此）。对于后者，请使用 [SPIRV-Tools][SpirvTools] 项目中的汇编器/反汇编器。

在序列化过程中，由于 SPIR-V 方言与二进制格式之间的表示差异，会执行若干转换：

*   `spirv.module` 上的属性被发射（emit）为其对应的 SPIR-V 指令。
*   类型被序列化为 SPIR-V 二进制模块中类型、常量和全局变量节的 `OpType*` 指令。
*   `spirv.Constant` 被统一并放置在 SPIR-V 二进制模块中类型、常量和全局变量节。
*   操作上的属性，如果不是操作二进制编码的一部分，则在 SPIR-V 二进制模块的修饰节中发射为 `OpDecorate*` 指令。
*   `spirv.mlir.selection` 和 `spirv.mlir.loop` 被发射为基本块，并在头块中包含二进制格式所要求的 `Op*Merge` 指令。
*   块参数被物化为对应块开头的 `OpPhi` 指令。

类似地，在反序列化过程中也会执行若干转换：

*   执行环境需求（扩展、能力、扩展指令集等）的指令将作为属性放置在 `spirv.module` 上。
*   `OpType*` 指令将被转换为适当的 `mlir::Type`。
*   `OpConstant*` 指令在每个使用处被物化为 `spirv.Constant`。
*   模块级的 `OpVariable` 指令将被转换为 `spirv.GlobalVariable` 操作；否则将被转换为 `spirv.Variable` 操作。
*   模块级 `OpVariable` 指令的每次使用都将物化一个 `spirv.mlir.addressof` 操作，将对应 `spirv.GlobalVariable` 的符号转换为 SSA 值。
*   `OpSpecConstant` 指令的每次使用都将物化一个 `spirv.mlir.referenceof` 操作，将对应 `spirv.SpecConstant` 的符号转换为 SSA 值。
*   `OpPhi` 指令被转换为块参数。
*   结构化控制流被放置在 `spirv.mlir.selection` 和 `spirv.mlir.loop` 中。

## 转换（Conversions）

MLIR 的主要特性之一是能够从捕获程序员抽象的方言逐步降低（lower）到更接近机器表示的方言，如 SPIR-V 方言。这种通过多个方言的渐进式降低通过 MLIR 中的[方言转换（DialectConversion）][MlirDialectConversion]框架实现。为了简化使用方言转换框架来以 SPIR-V 方言为目标，提供了两个实用类。

（**注意**：尽管 SPIR-V 有一些[验证规则][SpirvShaderValidation]，但 [Vulkan 执行环境][VulkanSpirv]还施加了额外的规则。下面描述的降低同时实现了这两种需求。）

### `SPIRVConversionTarget`

`mlir::spirv::SPIRVConversionTarget` 类继承自 `mlir::ConversionTarget` 类，作为一个工具，用于定义满足给定 [`spirv.target_env`](#target-environment) 的转换目标。它注册了适当的钩子（hook）来检查 SPIR-V 操作的动态合法性。用户可以进一步向返回的 `SPIRVConversionTarget` 注册其他合法性约束。

`spirv::lookupTargetEnvOrDefault()` 是一个便捷工具函数，用于查询输入 IR 中附加的 `spirv.target_env`，或使用默认值来构建 `SPIRVConversionTarget`。

### `SPIRVTypeConverter`

`mlir::SPIRVTypeConverter` 继承自 `mlir::TypeConverter`，为内置类型到 SPIR-V 类型提供类型转换，遵循构造时提供的[目标环境](#target-environment)。如果结果类型所需的扩展/能力在给定目标环境中不可用，`convertType()` 将返回空类型。

内置标量类型被转换为对应的 SPIR-V 标量类型。

（TODO：注意，如果位宽在目标环境中不可用，它将被无条件转换为 32 位。这应改为正确地模拟非 32 位标量类型。）

[内置 index 类型][MlirIndexType]需要特殊处理，因为 SPIR-V 中不直接支持它。目前 `index` 类型被转换为 `i32`。

（TODO：允许配置在 SPIR-V 方言中用于 `index` 类型的整数宽度。）

SPIR-V 只支持 2/3/4 元素的向量；因此这些长度的[内置向量类型][MlirVectorType]可以直接转换。

（TODO：将其他长度的向量转换为标量或数组。）

具有静态形状和步长（stride）的[内置 memref 类型][MlirMemrefType]被转换为 `spirv.ptr<spirv.struct<spirv.array<...>>>`。结果 SPIR-V 数组类型与源 memref 具有相同的元素类型，其元素数量从 memref 的布局规范中获取。指针类型的存储类（storage class）通过 `SPIRVTypeConverter::getStorageClassForMemorySpace()` 从 memref 的内存空间派生。

### 降低的工具函数（Utility functions for lowering）

#### 为着色器接口变量设置布局

SPIR-V 着色器的验证规则要求复合对象（composite object）明确布局。如果 `spirv.GlobalVariable` 没有显式布局，工具方法 `mlir::spirv::decorateType` 会实现与 [Vulkan 着色器需求][VulkanShaderInterface]一致的布局。

#### 创建内置变量（builtin variable）

在 SPIR-V 方言中，内置变量使用 `spirv.GlobalVariable` 表示，使用 `spirv.mlir.addressof` 获取内置变量的 SSA 值句柄。方法 `mlir::spirv::getBuiltinVariableValue` 在当前 `spirv.module` 中为该内置变量创建 `spirv.GlobalVariable`（如果尚不存在），并返回由 `spirv.mlir.addressof` 操作生成的 SSA 值。

### 当前到 SPIR-V 的转换

使用上述基础设施，从以下方言实现了转换：

*   [Arith 方言][MlirArithDialect]
*   [GPU 方言][MlirGpuDialect]：gpu.module 被转换为 `spirv.module`，该模块中的 gpu.function 被降低为入口函数（entry function）。

## 代码组织（Code organization）

我们旨在为 MLIR 中 SPIR-V 相关功能提供多个具有清晰依赖关系的库，以便开发者只选择所需组件，而无需引入整个世界。

### 方言（The dialect）

SPIR-V 方言的代码分布在以下几个位置：

*   公共头文件位于 [include/mlir/Dialect/SPIRV][MlirSpirvHeaders]。
*   库位于 [lib/Dialect/SPIRV][MlirSpirvLibs]。
*   IR 测试位于 [test/Dialect/SPIRV][MlirSpirvTests]。
*   单元测试位于 [unittests/Dialect/SPIRV][MlirSpirvUnittests]。

整个 SPIR-V 方言通过多个头文件对外暴露，以实现更好的组织：

*   [SPIRVDialect.h][MlirSpirvDialect] 定义 SPIR-V 方言。
*   [SPIRVTypes.h][MlirSpirvTypes] 定义所有 SPIR-V 特定类型。
*   [SPIRVOps.h][MlirSPirvOpsH] 定义所有 SPIR-V 操作。
*   [Serialization.h][MlirSpirvSerialization] 定义序列化和反序列化的入口点。

方言本身（包括所有类型和操作）位于 `MLIRSPIRV` 库中。序列化功能位于 `MLIRSPIRVSerialization` 库中。

### 操作定义（Op definitions）

我们使用[操作定义规范（Op Definition Spec，ODS）][ODS]来定义所有 SPIR-V 操作。它们使用 TableGen 语法编写，放置在头文件目录中的各个 `*Ops.td` 文件中。这些 `*Ops.td` 文件按照 SPIR-V 规范中使用的指令类别组织，例如，属于"原子指令（Atomics Instructions）"节的操作放置在 `SPIRVAtomicOps.td` 文件中。

`SPIRVOps.td` 是主要的操作定义文件，包含所有特定类别的文件。

`SPIRVBase.td` 定义了各种操作定义所使用的公共类和工具。它包含 TableGen SPIR-V 方言定义、SPIR-V 版本、已知扩展、各种 SPIR-V 枚举、TableGen SPIR-V 类型和基础操作类等。

`SPIRVBase.td` 中的许多内容（例如操作码和各种枚举）以及所有 `*Ops.td` 文件都可以通过 Python 脚本自动更新，该脚本直接从互联网查询 SPIR-V 规范和语法文件。这大大减轻了支持新操作和与 SPIR-V 规范保持同步的负担。有关此自动化开发流程的更多详情，见[自动化开发流程](#automated-development-flow)章节。

### 方言转换（Dialect conversions）

从其他方言到 SPIR-V 方言的转换代码也分布在以下几个位置：

*   从 GPU 方言：头文件位于 [include/mlir/Conversion/GPUTOSPIRV][MlirGpuToSpirvHeaders]；库位于 [lib/Conversion/GPUToSPIRV][MlirGpuToSpirvLibs]。
*   从 Func 方言：头文件位于 [include/mlir/Conversion/FuncToSPIRV][MlirFuncToSpirvHeaders]；库位于 [lib/Conversion/FuncToSPIRV][MlirFuncToSpirvLibs]。

这些方言到方言的转换有各自专用的库，分别为 `MLIRGPUToSPIRV` 和 `MLIRFuncToSPIRV`。

还有从任何方言以 SPIR-V 为目标时的通用工具：

*   [include/mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h][MlirSpirvConversion] 包含类型转换器和其他工具函数。
*   [include/mlir/Dialect/SPIRV/Transforms/Passes.h][MlirSpirvPasses] 包含 SPIR-V 特定的分析和转换（pass）。

这些通用工具分别在 `MLIRSPIRVConversion` 和 `MLIRSPIRVTransforms` 库中实现。

## 设计原理（Rationale）

### 将 `memref` 降低为 `!spirv.array<..>` 和 `!spirv.rtarray<..>`

LLVM 方言将 `memref` 类型降低为 `MemrefDescriptor`：

```
struct MemrefDescriptor {
  void *allocated_ptr; // Pointer to the base allocation.
  void *aligned_ptr;   // Pointer within base allocation which is aligned to
                       // the value set in the memref.
  size_t offset;       // Offset from aligned_ptr from where to get values
                       // corresponding to the memref.
  size_t shape[rank];  // Shape of the memref.
  size_t stride[rank]; // Strides used while accessing elements of the memref.
};
```

在 SPIR-V 方言中，我们选择不使用 `MemrefDescriptor`。相反，当 `memref` 具有静态形状时，将其直接降低为 `!spirv.ptr<!spirv.array<nelts x elem_type>>`；当 `memref` 具有动态形状时，降低为 `!spirv.ptr<!spirv.rtarray<elem_type>>`。这种选择背后的理由如下。

1.  SPIR-V 内核的输入/输出缓冲区使用[接口存储类（interface storage class）][VulkanShaderInterfaceStorageClass]（如 Uniform、StorageBuffer 等）中的 [`OpVariable`][SpirvOpVariable] 指定，而内核私有变量则位于非接口存储类（如 Function、Workgroup 等）中。默认情况下，Vulkan 风格的 SPIR-V 要求逻辑寻址模式：不能从变量中加载/存储指针，也不能执行指针算术。在接口存储类中表示像 `MemrefDescriptor` 这样的结构需要特殊的寻址模式（[PhysicalStorageBuffer][VulkanExtensionPhysicalStorageBuffer]），在非接口存储类中操作此类结构需要特殊能力（[VariablePointers][VulkanExtensionVariablePointers]）。同时要求这两个扩展将大大限制我们可以针对的 Vulkan 兼容设备，基本上排除了对移动端的支持。

1.  与使用一级间接层（如 `MemrefDescriptor`）的替代方案不同，可以将 `!spirv.array` 或 `!spirv.rtarray` 直接嵌入 `MemrefDescriptor` 中。但在 ABI 边界使用这样的描述符意味着输入/输出缓冲区的前几个字节需要为形状/步长信息预留。这给宿主端带来了不必要的负担。

1.  一种更高性能的方法是将数据作为 `OpVariable`，将形状和步长通过单独的 `OpVariable` 传递。这还有进一步的优势：

    *   `memref` 的所有动态形状/步长信息可以合并到单个描述符中。描述符在许多 [Vulkan 硬件上是有限资源][VulkanGPUInfoMaxPerStageDescriptorStorageBuffers]。因此合并它们有助于使生成的代码在设备间更具可移植性。
    *   如果形状/步长信息足够小，可以使用访问更快且避免缓冲区分配开销的[推送常量（PushConstants）][VulkanPushConstants]来访问。如果所有形状都是静态的，则这些开销是不必要的。在动态形状情况下，通常只需少量参数即可计算内核中使用/引用的所有 `memref` 的形状，从而可以使用推送常量。
    *   形状/步长信息（通常）需要更新的频率低于缓冲区中存储的数据。它们可以属于不同的描述符集。

## 贡献（Contribution）

非常欢迎所有形式的贡献！我们在 GitHub 上有 [issue][GitHubDialectTracking] 用于追踪[方言][GitHubDialectTracking]和[降低][GitHubLoweringTracking]开发。您可以在那里找到待办任务。[代码组织](#code-organization)章节提供了 SPIR-V 相关功能在 MLIR 中实现方式的概述。本章节给出了如何贡献的更具体步骤。

### 自动化开发流程（Automated development flow）

SPIR-V 方言开发的目标之一是利用 SPIR-V 的[人类可读规范][SpirvSpec]和[机器可读语法文件][SpirvGrammar]尽可能多地自动生成内容。具体而言，以下任务可以（部分或完全）自动化：

*   添加对新操作的支持。
*   添加对新 SPIR-V 枚举的支持。
*   新操作的序列化和反序列化。

我们通过 Python 脚本 [`gen_spirv_dialect.py`][GenSpirvUtilsPy] 实现了这一点。它直接从互联网获取人类可读规范和机器可读语法文件，并就地更新各种 SPIR-V `*.td` 文件。该脚本为添加新操作或枚举的支持提供了自动化流程。

此后，我们有 SPIR-V 特定的 `mlir-tblgen` 后端，用于读取操作定义规范并生成各种组件，包括操作的（反）序列化逻辑。结合标准的 `mlir-tblgen` 后端，我们自动生成所有操作类、枚举类等。

在以下子章节中，我们列出了常见任务的详细步骤。

### 添加新操作（Add a new op）

要添加新操作，请调用 utils/spirv 中的 `define_inst.sh` 脚本包装器。`define_inst.sh` 需要几个参数：

```sh
./define_inst.sh <filename> <base-class-name> <opname>
```

例如，要为 `OpIAdd` 定义操作，执行：

```sh
./define_inst.sh SPIRVArithmeticOps.td ArithmeticBinaryOp OpIAdd
```

其中 `SPIRVArithmeticOps.td` 是承载新操作的文件名，`ArithmeticBinaryOp` 是新定义操作将继承的直接基类。

类似地，要为 `OpAtomicAnd` 定义操作：

```sh
./define_inst.sh SPIRVAtomicOps.td AtomicUpdateWithValueOp OpAtomicAnd
```

注意，生成的 SPIR-V 操作定义只是一个尽力而为的模板；仍需更新以具有更准确的 trait、参数和结果。

还需要为新操作定义自定义汇编格式，这将需要提供解析器（parser）和打印器（printer）。自定义汇编的 EBNF 形式应在操作的描述中说明，解析器和打印器应放置在 [`SPIRVOps.cpp`][MlirSpirvOpsCpp] 中，具有以下签名：

```c++
static ParseResult parse<spirv-op-symbol>Op(OpAsmParser &parser,
                                            OperationState &state);
static void print(spirv::<spirv-op-symbol>Op op, OpAsmPrinter &printer);
```

参见任何现有操作作为示例。

应为新操作提供验证（verification），以覆盖 SPIR-V 规范中描述的所有规则。选择适当的 ODS 类型和属性类型（可在 [`SPIRVBase.td`][MlirSpirvBase] 中找到）会有所帮助。但有时仍需要在 [`SPIRVOps.cpp`][MlirSpirvOpsCpp] 中手动编写额外的验证逻辑，函数签名如下：

```c++
LogicalResult spirv::<spirv-op-symbol>Op::verify();
```

参见 [`SPIRVOps.cpp`][MlirSpirvOpsCpp] 中任何此类函数作为示例。

如果不需要额外的验证，需要在操作的操作定义规范中添加以下内容：

```
let hasVerifier = 0;
```

以抑制对上述 C++ 验证函数的要求。

操作的自定义汇编格式和验证的测试应添加到 test/Dialect/SPIRV/ 中的适当文件中。

生成的操作将自动获得（反）序列化的逻辑。但仍需将测试与更改配套，以确保没有意外（见下面的[添加新测试](#add-a-new-test)）。

### 添加新枚举（Add a new enum）

要添加新枚举，请调用 utils/spirv 中的 `define_enum.sh` 脚本包装器。`define_enum.sh` 需要以下参数：

```sh
./define_enum.sh <enum-class-name>
```

例如，要将 SPIR-V 存储类（storage class）定义添加到 `SPIRVBase.td`：

```sh
./define_enum.sh StorageClass
```

### 添加新自定义类型（Add a new custom type）

SPIR-V 特定类型定义在 [`SPIRVTypes.h`][MlirSpirvTypes] 中。参见其中的示例以及定义新自定义类型的[教程][CustomTypeAttrTutorial]。

### 添加新转换（Add a new conversion）

要添加类型的转换，请更新 `mlir::spirv::SPIRVTypeConverter` 以返回转换后的类型（必须是有效的 SPIR-V 类型）。更多详情参见[类型转换（Type Conversion）][MlirDialectConversionTypeConversion]。

要将操作降低到 SPIR-V 方言，请实现一个[转换模式（conversion pattern）][MlirDialectConversionRewritePattern]。如果转换还需要类型转换，该模式必须继承自 `mlir::spirv::SPIRVOpLowering` 类，以获得对 `mlir::spirv::SPIRVTypeConverter` 的访问。如果操作包含区域（region），还可能需要[签名转换（signature conversion）][MlirDialectConversionSignatureConversion]。

**注意**：`spirv.module` 当前的验证规则要求其区域中包含的所有操作都是 SPIR-V 方言中的有效操作。

### 添加新测试（Add a new test）

目前 SPIR-V 方言有三类测试，应根据情况添加或更新：

1.  **方言测试（Dialect tests）** — 这些测试单独检查操作的不同方面。应包括正面和负面用例，并对验证器、解析器和打印器进行测试。方言测试不必构成有效的 SPIR-V 代码，应尽量保持简单。它们使用 `mlir-opt` 运行；也用于测试转换。

2.  **目标测试（Target tests）** — 这些测试旨在测试序列化和反序列化，因此每个模块都应是有效的 SPIR-V 模块。（反）序列化使用 `mlir-translate --test-spirv-roundtrip` 选项进行测试。

    为了确保 SPIR-V MLIR 格式序列化为有效的 SPIR-V，应对序列化后的二进制文件（`--serialize-spirv`）运行 `spirv-val` 工具。这可以通过向测试添加条件验证节来自动化：

    ```
    // RUN: %if spirv-tools %{ rm -rf %t %}
    // RUN: %if spirv-tools %{ mkdir %t %}
    // RUN: %if spirv-tools %{ mlir-translate --no-implicit-module --serialize-spirv --split-input-file --spirv-save-validation-files-with-prefix=%t/module %s %}
    // RUN: %if spirv-tools %{ spirv-val %t %}
    ```

    此序列将每个 MLIR SPIR-V 模块序列化并转储为单独的 SPIR-V 二进制文件（MLIR 允许每个文件包含多个模块，但 SPIR-V 规范限制每个二进制文件只能有一个模块），然后对每个文件运行 `spirv-val`。

3.  **集成测试（Integration tests）** — 这些测试使用 `mlir-runner` 执行 MLIR 代码，以验证其功能正确性。

## 操作定义（Operation definitions）

[include "Dialects/SPIRVOps.md"]

[Spirv]: https://www.khronos.org/registry/spir-v/
[SpirvSpec]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html
[SpirvLogicalLayout]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#_a_id_logicallayout_a_logical_layout_of_a_module
[SpirvGrammar]: https://raw.githubusercontent.com/KhronosGroup/SPIRV-Headers/master/include/spirv/unified1/spirv.core.grammar.json
[SpirvShaderValidation]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#_a_id_shadervalidation_a_validation_rules_for_shader_a_href_capability_capabilities_a
[SpirvOpVariable]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpVariable
[GlslStd450]: https://www.khronos.org/registry/spir-v/specs/1.0/GLSL.std.450.html
[ArrayType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeArray
[ImageType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeImage
[PointerType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypePointer
[RuntimeArrayType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeRuntimeArray
[SampledImageType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#OpTypeSampledImage
[MlirDialectConversion]: ../DialectConversion.md
[StructType]: https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#Structure
[SpirvTools]: https://github.com/KhronosGroup/SPIRV-Tools
[Rationale]: ../Rationale/Rationale.md/#block-arguments-vs-phi-nodes
[ODS]: ../DefiningDialects/Operations.md
[GreedyPatternRewriter]: https://github.com/llvm/llvm-project/blob/main/mlir/lib/Transforms/Utils/GreedyPatternRewriteDriver.cpp
[MlirDialectConversionTypeConversion]: ../DialectConversion.md/#type-converter
[MlirDialectConversionRewritePattern]: ../DialectConversion.md/#conversion-patterns
[MlirDialectConversionSignatureConversion]: ../DialectConversion.md/#region-signature-conversion
[MlirOpInterface]: ../Interfaces/#operation-interfaces
[MlirIntegerType]: Builtin.md/#integertype
[MlirVectorType]: Builtin.md/#vectortype
[MlirMemrefType]: Builtin.md/#memreftype
[MlirIndexType]: Builtin.md/#indextype
[MlirGpuDialect]: GPU.md
[MlirArithDialect]: Arithmetic.md
[MlirSpirvHeaders]: https://github.com/llvm/llvm-project/tree/main/mlir/include/mlir/Dialect/SPIRV
[MlirSpirvLibs]: https://github.com/llvm/llvm-project/tree/main/mlir/lib/Dialect/SPIRV
[MlirSpirvTests]: https://github.com/llvm/llvm-project/tree/main/mlir/test/Dialect/SPIRV
[MlirSpirvUnittests]: https://github.com/llvm/llvm-project/tree/main/mlir/unittests/Dialect/SPIRV
[MlirGpuToSpirvHeaders]: https://github.com/llvm/llvm-project/tree/main/mlir/include/mlir/Conversion/GPUToSPIRV
[MlirGpuToSpirvLibs]: https://github.com/llvm/llvm-project/tree/main/mlir/lib/Conversion/GPUToSPIRV
[MlirFuncToSpirvHeaders]: https://github.com/llvm/llvm-project/tree/main/mlir/include/mlir/Conversion/FuncToSPIRV
[MlirFuncToSpirvLibs]: https://github.com/llvm/llvm-project/tree/main/mlir/lib/Conversion/FuncToSPIRV
[MlirSpirvDialect]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/IR/SPIRVDialect.h
[MlirSpirvTypes]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/IR/SPIRVTypes.h
[MlirSpirvOpsH]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/IR/SPIRVOps.h
[MlirSpirvSerialization]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Target/SPIRV/Serialization.h
[MlirSpirvBase]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/IR/SPIRVBase.td
[MlirSpirvPasses]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/Transforms/Passes.h
[MlirSpirvConversion]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h
[MlirSpirvAbi]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Dialect/SPIRV/IR/TargetAndABI.h
[MlirSpirvOpsCpp]: https://github.com/llvm/llvm-project/blob/main/mlir/lib/Dialect/SPIRV/IR/SPIRVOps.cpp
[GitHubDialectTracking]: https://github.com/tensorflow/mlir/issues/302
[GitHubLoweringTracking]: https://github.com/tensorflow/mlir/issues/303
[GenSpirvUtilsPy]: https://github.com/llvm/llvm-project/blob/main/mlir/utils/spirv/gen_spirv_dialect.py
[CustomTypeAttrTutorial]: ../DefiningDialects/AttributesAndTypes.md
[VulkanExtensionPhysicalStorageBuffer]: https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_physical_storage_buffer.html
[VulkanExtensionVariablePointers]: https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_variable_pointers.html
[VulkanSpirv]: https://renderdoc.org/vkspec_chunked/chap40.html#spirvenv
[VulkanShaderInterface]: https://renderdoc.org/vkspec_chunked/chap14.html#interfaces-resources
[VulkanShaderInterfaceStorageClass]: https://renderdoc.org/vkspec_chunked/chap15.html#interfaces
[VulkanResourceLimits]: https://renderdoc.org/vkspec_chunked/chap36.html#limits
[VulkanGPUInfoMaxPerStageDescriptorStorageBuffers]: https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPerStageDescriptorStorageBuffers&platform=android
[VulkanPushConstants]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdPushConstants.html
