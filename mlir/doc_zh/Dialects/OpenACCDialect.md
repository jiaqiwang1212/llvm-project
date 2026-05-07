# 'acc' 方言

`acc` 方言是用于表示 OpenACC 编程模型的 MLIR 方言。OpenACC 是一种标准化的基于指令的模型，与 C、C++ 和 Fortran 配合使用，使程序员能够暴露代码中的并行性。OpenACC 采用的描述性方法通过赋予编译器针对特定架构并行化的自由，从而能够针对并行多核和 GPU 等加速目标。OpenACC 还提供了通过日益规范化的子句来优化并行性的能力。

该方言对[OpenACC 3.3 规范](https://www.openacc.org/sites/default/files/inline-images/Specification/OpenACC-3.3-final.pdf)中的构造进行建模。

本文档描述了 MLIR 中 OpenACC 方言的设计，列出并解释了设计目标和设计选择及其原理，还描述了 acc 方言操作、类型和属性方面的具体内容。

[TOC]

## 方言设计目标

* 需要完整表示 OpenACC 语言。
	- 前端需要这样做，以便正确生成 MLIR 中可能的 `acc` pragma 的表示。此外，预期该方言在具体化其语义时会进一步降低。如果没有完整的表示，前端可能会选择较低的抽象（例如直接的运行时调用）——但这会影响在方言上进行分析和优化的能力。
* 允许在与 OpenACC 语言相同的语义级别进行表示，同时具备以与语言无关的方式表示源语言语义细微差别（例如 Fortran 描述符）的能力。
	- 使用密切模拟 OpenACC 语言的抽象可以简化前端实现，也便于调试 IR。然而，在具体化 OpenACC 时，有时需要特定于源语言的行为。在这些情况下，例如具有默认构造函数的 C++ 对象的私有化，前端填入 `recipe` 以及 `private` 操作，这些可以与 `acc` 方言操作整洁地打包在一起。
* 能够从方言（包括边界、名称、子句、修饰符等）重新生成与用户 pragma 语义等价的内容。
	- 这是确保方言在语义上无损失的有力衡量标准。它还允许在前端之外生成适当而有用的调试信息的能力。
* 与方言无关，以便可以与其他方言（包括但不限于 `hlfir`、`fir`、`llvm`、`cir`）一起使用和共存。
	- 基于指令的模型（如 OpenACC）始终与源语言一起使用，因此 `acc` 方言与其他方言共存是构造性必要的。通过适当的抽象，`acc` 方言和源语言方言都不应相互依赖；在需要时，应使用接口来确保 `acc` 方言可以验证预期属性。
* 方言必须允许使用 MLIR 现有设施准确且高效地对数据流进行建模。
	- 适当的数据流建模对于分析和 IR 推理很重要——即使是像遍历使用这样简单的操作。因此，操作（如数据操作）预期会生成可用于建模行为的结果。例如，考虑一个 `acc copyin` 子句。在 `acc.copyin` 操作之后，存在于设备上的指针应该与存在于主机内存中的指针可区分。
* 通过实现通用接口对 MLIR 优化传递友好。
	- 接口（如 `MemoryEffects`）是 MLIR 变换和分析设计为与 IR 交互的关键方式。为了使 `acc` 方言中的操作可被优化（直接或间接地通过不阻塞嵌套 IR 的优化），需要实现相关的通用接口。

acc 方言的设计理念是遵守设计目标。当前和计划中的操作、属性、类型必须遵守设计目标。

## 操作类别

OpenACC 方言既包含高层操作（与其 OpenACC 语言等价物保留相同的语义含义），也包含中间层操作（用于将子句从构造中分解）和低层操作（以通用方式编码与源语言相关的细节）。

高层操作列表包含以下 OpenACC 语言构造及其对应的操作：
* `acc parallel` &rarr; `acc.parallel`
* `acc kernels` &rarr; `acc.kernels`
* `acc serial` &rarr; `acc.serial`
* `acc data` &rarr; `acc.data`
* `acc loop` &rarr; `acc.loop`
* `acc enter data` &rarr; `acc.enter_data`
* `acc exit data` &rarr; `acc.exit_data`
* `acc host_data` &rarr; `acc.host_data`
* `acc init` &rarr; `acc.init`
* `acc shutdown` &rarr; `acc.shutdown`
* `acc update` &rarr; `acc.update`
* `acc set` &rarr; `acc.set`
* `acc wait` &rarr; `acc.wait`
* `acc atomic read` &rarr; `acc.atomic.read`
* `acc atomic write` &rarr; `acc.atomic.write`
* `acc atomic update` &rarr; `acc.atomic.update`
* `acc atomic capture` &rarr; `acc.atomic.capture`

第二组包含用于表示分解构造或子句以进行更精确建模的操作：
* `acc routine` &rarr; `acc.routine` + `acc.routine_info` 属性
* `acc declare` &rarr; `acc.declare_enter` + `acc.declare_exit` 或 `acc.declare`
* `acc {construct} copyin` &rarr; `acc.copyin`（区域前）+ `acc.delete`（区域后）
* `acc {construct} copy` &rarr; `acc.copyin`（区域前）+ `acc.copyout`（区域后）
* `acc {construct} copyout` &rarr; `acc.create`（区域前）+ `acc.copyout`（区域后）
* `acc {construct} attach` &rarr; `acc.attach`（区域前）+ `acc.detach`（区域后）
* `acc {construct} create` &rarr; `acc.create`（区域前）+ `acc.delete`（区域后）
* `acc {construct} present` &rarr; `acc.present`（区域前）+ `acc.delete`（区域后）
* `acc {construct} no_create` &rarr; `acc.nocreate`（区域前）+ `acc.delete`（区域后）
* `acc {construct} deviceptr` &rarr; `acc.deviceptr`
* `acc {construct} private` &rarr; `acc.private`
* `acc {construct} firstprivate` &rarr; `acc.firstprivate`
* `acc {construct} reduction` &rarr; `acc.reduction`
* `acc cache` &rarr; `acc.cache`
* `acc update device` &rarr; `acc.update_device`
* `acc update host` &rarr; `acc.update_host`
* `acc host_data use_device` &rarr; `acc.use_device`
* `acc declare device_resident` &rarr; `acc.declare_device_resident`
* `acc declare link` &rarr; `acc.declare_link`
* `acc exit data delete` &rarr; `acc.delete`（`structured` 标志为 false）
* `acc exit data detach` &rarr; `acc.detach`（`structured` 标志为 false）
* `acc {construct} {data_clause}(var[lb:ub])` &rarr; `acc.bounds`

低层操作为：
* `acc.private.recipe`
* `acc.reduction.recipe`
* `acc.firstprivate.recipe`
* `acc.global_ctor`
* `acc.global_dtor`
* `acc.yield`
* `acc.terminator`
低层操作的语义和原理将在以下章节进一步解释。

### 数据操作

#### 数据子句分解
数据子句从其构造中分解，以便在 MLIR 中进行更好的数据流建模。这样做有多种原因，与方言目标一致：
* 正确表示数据流。数据子句在进入区域时和退出区域时具有不同的效果。
* 更容易向单个操作添加诸如 `MemoryEffects` 的属性。这可以更好地反映语义（例如，`acc.copyin` 操作只读取主机内存）。
* 可以单独移动或优化操作（例如 `CSE`）。
* 更容易跟踪调试信息。行位置可以指向表示数据子句的文本，而不是构造。此外，可以使用属性跟踪子句中的变量名称，而无需遍历 IR 树来尝试恢复信息（这使 acc 方言对其与哪个其他方言一起使用更加无关）。
* 清晰的操作顺序，因为所有数据操作都在同一列表中。

每个 `acc` 方言数据操作要么表示数据动作规范的入口部分，要么表示出口部分。因此，`acc.copyin` 表示第 `2.7.7 copyin 子句` 节中定义的语义，其措辞以"在进入区域时"开始。分解的出口操作 `acc.delete` 表示该节的第二部分，其措辞以"在退出区域时"开始。`delete` 动作可以在检查和更新相关引用计数器后执行。

`acc` 数据操作即使在分解后，也会在操作操作数 `dataClause` 中保留其原始数据子句，以便在调试时恢复此信息。例如，`acc copy` 不会翻译为 `acc.copy` 操作，而是翻译为入口的 `acc.copyin` 和出口的 `acc.copyout`。两个分解操作都持有一个 `dataClause` 字段，指定这是一个 `acc copy`。

分解的入口和出口操作之间的链接是入口操作产生的 SSA 值。即，`accPtr` 结果既用于构造操作的 `dataOperands` 中，也用于出口操作的 `accPtr` 操作数中。

#### 边界

OpenACC 数据子句允许根据 `2.7.1 数据子句中的数据规范` 使用边界说明符。但是，如果源语言的类型系统捕获了此信息，则数据的数组维度并不总是必需的——用户只需在数据子句中指定变量名即可。因此，`acc.bounds` 操作是确保用户显式设置维度和隐式基于类型维度统一表示的重要部分。它包含几个关键特性，以允许以灵活且与源语言方言无关的方式正确编码大小：
* 可以使用多个有序的 `acc.bounds` 操作表示多维数组。
* 边界需要零归一化。这与数据子句中的 `PointerLikeType` 要求配合良好——因为下界为 0 意味着查看指针的零偏移处的数据。这一要求也有助于确保 `acc` 方言与源语言方言无关，因为它防止了诸如 Fortran 数组（其下界不是固定值）等情况的歧义。
* 如果源方言没有在类型中编码维度（例如 `!fir.array<?x?xi32>`），而是通过其他方式编码（例如通过描述符），则前端必须用适当的信息（例如从描述符加载）填充 `acc.bounds` 操作数。`acc.bounds` 操作也允许有损的源方言，例如如果前端使用激进的指针衰减并且不能在类型系统中表示维度（例如对数组使用 `!llvm.ptr`）。这两个方面都表明了 `acc.bounds` 操作的灵活性，允许表示与源无关，因为 `acc` 方言不应该知道如何从源方言的类型中提取维度信息。
* OpenACC 规范允许在数据子句中使用范围或上界，取决于是 Fortran 还是 C 和 C++。`acc.bounds` 操作足够丰富，可以接受其中一个或两个——为了方便降低到方言，以及能够精确捕获子句中的含义。
* 步幅（以单位或字节为单位）也可以在 `acc.bounds` 操作中捕获。这也是一个重要部分，能够接受源语言的数组而不强迫前端以某种方式规范化它们。例如，考虑在父函数中，整个数组被映射到设备。然后只有一个非 1 步幅的视图传递给子函数（例如步幅非 1 的 Fortran 数组切片）。子函数中对此数据的 `copy` 操作应该能够避免重新映射此数组。如果操作要求规范化数组（例如使其连续），则同一主机数据的意外不相交映射将容易出错，因为它会导致对设备的多次映射。

#### 计数器

数据操作还维护 OpenACC 规范中描述的与运行时计数器相关的语义。更具体地，考虑第 2.7.7 节中 `acc copyin` 入口部分的规范：
```
At entry to a region, the structured reference counter is used. On an
enter data directive, the dynamic reference counter is used.
- If var is present and is not a null pointer, a present increment
action with the appropriate reference counter is performed.
- If var is not present, a copyin action with the appropriate reference
counter is performed.
- If var is a pointer reference, an attach action is performed.
```
`acc.copyin` 操作包含这些语义，包括与 attach 相关的语义，通过 `varPtrPtr` 操作数指定。操作上的 `structured` 标志很重要，因为当标志为 true 时应使用`结构化引用计数器`；当标志为 false 时应使用`动态引用计数器`。

在退出结构化区域（`acc data`、`acc kernels`）时，`acc copyin` 操作被分解为 `acc.delete`（`structured` 标志为 true）。`acc.delete` 的语义也与 OpenACC 规范中 `acc copyin` 子句出口部分的说明一致：
```
At exit from the region:
- If the structured reference counter for var is zero, no action is
taken.
- Otherwise, a detach action is performed if var is a pointer reference,
and a present decrement action with the structured reference counter is
performed if var is not a null pointer. If both structured and dynamic
reference counters are zero, a delete action is performed.
```

### 类型

由于 `acc 方言` 旨在与表示源语言的其他方言一起使用，因此适当使用类型和类型接口是确保兼容性的关键。本节描述这些考虑因素。

#### 数据子句操作类型

数据子句操作（例如 `acc.copyin`）依赖以下类型考虑：
* acc 数据子句操作输入 `var` 的类型
	- `var` 的类型必须是附加了 `PointerLikeType` 或 `MappableType` 接口的类型。前者 `PointerLikeType` 很有用，因为 OpenACC 内存模型明确区分主机内存和设备内存——两者之间的映射通过指针完成。因此，通过在方言中明确要求它，适当的语言前端必须创建存储或使用满足映射约束的类型。第二种可能性 `MappableType` 被添加，是因为内存/存储概念是一种较低层次的抽象，并非所有方言都选择使用指针抽象，特别是在语义更复杂的情况下（例如 `fir.box`，它表示 Fortran 描述符，定义在 `flang` 使用的 `fir` 方言中）。
* acc 数据子句操作结果的类型
	- acc 数据子句操作的类型与 `var` 完全相同。这是故意这样做的，而不是引入特定的 `acc` 输出类型，以便保持 IR 兼容性和方言的现有强类型检查。这是必需的，因为 `acc` 方言必须存在于其类型系统对其未知的另一个方言中。
* 在 `varType` 中捕获的变量类型
	- 当 `var` 的类型是 `PointerLikeType` 时，目标变量的实际类型可能会丢失。更具体地说，像 `llvm` 这样使用不透明指针的方言不记录目标变量的类型。使用此字段可以弥合这一差距。
* 分解子句的类型
	- 分解子句（例如 `acc.bounds` 和 `acc.declare_enter`）产生类型，以允许其结果仅在特定操作中使用。这些是仅用于正确 IR 构造的合成类型。

#### 指针类型要求

acc 方言中需要指针类型要求，原因有几个：
- `hlfir`、`fir`、`cir`、`llvm` 等现有方言使用指针表示来表示变量。
- 数据子句的引用计数器以内存术语描述。在 OpenACC 规范 3.3 第 2.6.7 节中，它说："当进入包含该内存部分的显式数据子句或隐式确定数据属性的每个数据或计算区域时，结构化引用计数器递增"。这意味着内存的可寻址性。
- Attach 语义（2.6.8 附加计数器）使用"地址"术语指定："每当指针附加到新目标地址时，指针的附加计数器设置为 1，每当对该指针执行针对同一目标地址的附加动作时递增。

#### 类型接口

`acc` 方言描述了两种不同的类型接口，必须实现并附加到源方言的类型上，以允许使用数据子句操作（例如 `acc.copyin`）。它们如下：
* `PointerLikeType`
  - 这个接口背后的想法是，变量最终在许多方言中以指针的形式表示。更具体地，`fir`、`cir`、`llvm` 用某种方言特定形式的 `alloca` 操作表示用户声明的局部变量，该操作产生指针。全局变量同样通过某种形式的 `address_of` 操作通过其地址引用。此外，OpenACC 运行时的实现需要区分设备内存和主机内存——通常也通过谈论指针来完成。因此，这个类型接口要求与 OpenACC 规范自然契合。数据映射操作语义通常可以简单地通过指针和它指向的数据大小来描述。
* `MappableType`
   - 引入此接口是因为 `PointerLikeType` 要求无法表示源方言不使用指针的情况。此外，某些情况（例如 Fortran 描述符支持的数组和 Fortran 可选参数）需要分解为多个步骤。例如，在描述符情况下，需要描述符的映射、数据的映射以及到设备描述符的隐式附加。为了允许用单个数据子句操作捕获所有这些复杂性，引入了 `MappableType` 接口。这与方言的目标一致，包括"能够重新生成与用户 pragma 语义等价的内容"。

意图是方言的类型系统实现这两个接口之一。精确地说，类型应该只实现其中一个（而不是两者）——因为将它们分开可以避免对实际需要映射的内容产生歧义。当 `var` 是 `PointerLikeType` 时，假设将映射所指向的数据。如果指针类型也实现了 `MappableType` 接口，则对于是否映射所指向的数据或指针本身变得不明确。

### Recipe

Recipe 是一种表达特定源语言语义的通用方式。

目前有两类 recipe，但 recipe 概念可以扩展到任何需要捕获的附加低层信息，以便成功降低 OpenACC。这两类是：
* 用于与构造关联的私有化上下文中的 recipe
* 用于数据语义附加规范上下文中的 recipe

recipe 的目的是指定在解释和降低动作（例如私有化）的语义时应如何具体化，例如在生成 LLVM 方言之前。

用于私有化的 recipe 提供了一种与源语言无关的方式来指定该类型本地变量的创建。这意味着使用适当的 `alloca` 指令，并能够指定默认初始化或默认构造函数。

### Routine

routine 指令用于注意某个过程应该以与其修饰符（如描述并行性的修饰符）一致的方式为加速器提供。在 acc 方言中，acc routine 通过两个联合部分表示——一个属性和一个操作：
* `acc.routine` 操作只是一个说明符，注明了哪个符号（或字符串）需要 acc routine，以及相关的并行性。这定义了一个可以在属性中引用的符号。
* `acc.routine_info` 属性是一个属性，用于指定一个或多个 `acc.routine` 符号的源方言特定操作。通常，这附加到 `func.func`，该函数提供声明（外部情况下）或提供 acc routine 在源语言翻译到的方言中的实际主体。

### Declare

OpenACC `declare` 是一种机制，它将全局或局部变量的定义声明为可被加速器访问，其生命周期与声明它的作用域的生命周期相同。因此，`declare` 语义通过多个操作和属性表示：
* `acc.declare` - 这是一个结构化操作，包含一个 MLIR 区域，可以类似于 acc.data 使用，以指定具有特定过程生命周期的隐式数据区域。通常在变量声明之后在 `func.func` 内部使用。
* `acc.declare_enter` - 这是一个非结构化操作，用作 `acc declare` 的分解形式。它有效地允许入口操作存在于与出口操作不同的作用域中。它也可以与 `acc.declare_exit` 一起使用，后者消耗其令牌，以定义不使用 MLIR 区域的范围区域。此操作也用于 `acc.global_ctor` 中。
* `acc.declare_exit` - `acc.declare_enter` 的匹配等价物，但指定出口语义。此操作通常在 `func.func` 的出口点或与 `acc.global_dtor` 一起使用。
* `acc.global_ctor` - 与源方言全局变量处于同一级别，用于指定在程序入口处要完成的数据动作。与生命周期不仅限于单个过程的源方言全局变量一起使用。
* `acc.global_dtor` - 定义在程序退出时应完成的出口数据动作。通常用于撤销 `acc.global_ctor` 的动作。

属性：
* `acc.declare` - 这是一个工具，用于更容易地确定哪些变量被 `acc declare` 了。此属性用于生成全局变量的操作以及生成局部变量的操作（例如方言特定的 `alloca`）。要出现在与任何 `acc.declare*` 操作关联的数据映射操作中，需要具有此属性。
* `acc.declare_action` - 由于 OpenACC 规范允许声明尚未分配的变量，此属性在分配和释放点使用。更具体地，此属性捕获要调用的函数符号，以在预分配、后分配、预释放或后释放时执行动作。在降低 OpenACC 语义时，应该具体化对这些函数的调用，以确保在分配/释放之后执行正确的数据动作。

## OpenACC 变换和分析

`acc` 方言的设计目标是对 MLIR 优化传递（包括 CSE 和 LICM）友好。此外，由于它设计为能够恢复原始子句，因此可以在前端之外在 MLIR 框架中进行后期验证和分析。

本节描述 `acc` 方言设计应该对其友好的几个 MLIR 级别传递。本节目前仅概述设计意图的可能性，不一定是现有的传递。

### 验证

由于 OpenACC 方言对其表示没有损失，因此可以在 MLIR 级别进行 OpenACC 语言语义检查。以下是各种所需语义检查的列表。

第一个列表需要在前端完成，因为 `acc` 方言操作在构造时必须是有效的：
* 确保每个指令只允许列出的子句。
* 确保每个子句只允许列出的修饰符。

然而，以下是可以在 MLIR 级别完成的语义检查（在单独的传递中或作为操作验证器的一部分）：
* 指定每个修饰符需要的有效性检查。（例如 num_gangs 可能需要正整数）。
* 确保有效的子句嵌套。
* 验证不能与其他子句一起出现的子句限制。
* 验证变量上没有使用冲突的子句。

请注意，当在 MLIR 级别完成时，这些检查可以更加精确，因为内联和常量传播等优化会暴露在前端不可见的细节。

### 隐式数据属性

OpenACC 规范包含关于 `2.6.2 具有隐式确定数据属性的变量`的章节。该章节描述了用户未指定数据动作的变量应该应用的数据动作。动作取决于所使用的构造以及默认子句。然而，需要注意的是，进入 acc 区域时存活的变量必须采用某种数据映射，以便数据可以传递给加速器。

影响所需数据属性的一个可能优化是`聚合的标量替换（SROA）`。`acc` 方言不应阻止这在源方言上发生。

由于旨在能够跨 `acc` 区域应用优化，应用隐式数据属性的分析/变换传递应该尽可能晚运行——理想情况下在使用 `acc` 区域主体创建加速器过程的任何轮廓提取过程之前。预期现有 MLIR 设施（例如 `mlir::Liveness`）将适用于 `acc` 区域，因此可以用于执行此分析。

### 冗余子句消除

数据操作的建模方式使数据入口操作看起来像 load，数据出口操作看起来像 store。因此，这些操作旨在以以下方式进行优化：
* 能够消除冗余操作，例如当一个 `acc.copyin` 支配另一个时。
* 能够将此类操作提升/下沉到循环外。

## 操作目录

[include "Dialects/OpenACCDialectOps.md"]
