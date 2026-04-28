# 使用 LLVM 进行源码级调试

> 原文：https://llvm.org/docs/SourceLevelDebugging.html
> 版本：LLVM 23.0.0git

---

## 简介

本文档是 LLVM 中所有调试信息相关内容的中央知识库。它描述了 LLVM 调试信息的实际格式，对于希望创建前端或直接处理调试信息的人员很有参考价值。此外，本文档还提供了 C/C++ 调试信息的具体示例。

---

## LLVM 调试信息的设计哲学

LLVM 调试信息的核心思想是捕获源语言抽象语法树（AST）中的重要片段如何映射到 LLVM 代码上。若干设计方面影响了当前的解决方案，其中最重要的几点是：

- 调试信息应对编译器其他部分产生极小的影响。任何变换、分析或代码生成器都不应因调试信息而需要修改。
- LLVM 优化应以定义明确且易于描述的方式与调试信息交互。
- 由于 LLVM 被设计为支持任意编程语言，LLVM-to-LLVM 工具不应需要了解源语言的任何语义。
- 源级语言之间往往差异很大。LLVM 不应对源语言的风格施加任何限制，调试信息应适用于任何语言。
- 借助代码生成器支持，应当可以使用 LLVM 编译器将程序编译为本机机器码和标准调试格式，从而与传统机器码级调试器（如 GDB 或 DBX）兼容。

LLVM 实现采用的方法是使用一小组**调试记录（debug records）**来定义 LLVM 程序对象与源级对象之间的映射。源级程序的描述以 LLVM 元数据（metadata）的形式保存，格式由实现定义（C/C++ 前端目前使用 DWARF 3 标准的第 7 号工作草案）。

调试程序时，调试器与用户交互，并将存储的调试信息转化为源语言特定的信息。因此，调试器必须了解源语言，并因此与特定语言或语言家族绑定。

---

## 调试信息消费者

调试信息的作用是提供编译过程中通常被丢弃的元信息。该元信息为 LLVM 用户提供了生成代码与原始程序源代码之间的关联关系。

目前，调试信息有两个后端消费者：**DwarfDebug** 和 **CodeViewDebug**。DwarfDebug 生成适合 GDB、LLDB 及其他基于 DWARF 的调试器使用的 DWARF 信息。CodeViewDebug 生成 CodeView（Microsoft 调试信息格式），可用于 Visual Studio 和 WinDBG 等 Microsoft 调试器。LLVM 的调试信息格式大体上源自并受到 DWARF 的启发，但也可以转换为其他目标调试信息格式（如 STABS）。

**SamplePGO**（也称为 AutoFDO）是基于配置文件引导优化的一种变体，它使用基于硬件采样的性能分析器在生产环境中以低开销收集分支频率数据。它依赖调试信息将性能分析信息与 LLVM IR 关联起来，进而指导优化启发式策略。维护确定性且各不相同的源位置对于最大化硬件采样计数到 LLVM IR 的映射精度至关重要。例如，DWARF 鉴别符（discriminators）允许 SamplePGO 区分映射到同一源代码行的多条执行路径。

调试信息也可合理地用于为性能分析工具提供生成代码的分析数据，或用于从生成代码重建原始源代码的工具。

---

## 调试信息与优化

LLVM 调试信息的一个极高优先级是使其与优化和分析良好交互。具体而言，LLVM 调试信息提供以下保证：

- **LLVM 调试信息始终提供准确读取程序源级状态的信息**，无论运行了哪些 LLVM 优化。[《如何更新调试信息：LLVM Pass 作者指南》](https://llvm.org/docs/HowToUpdateDebugInfo.html)规定了在各类代码变换中应如何更新调试信息以避免破坏此保证，以及如何尽可能保留有用的调试信息。注意，某些优化可能会影响通过调试器修改程序当前状态的能力，例如设置程序变量或调用已被删除的函数。

- 如果需要，LLVM 优化可以升级为感知调试信息，允许它们在执行激进优化时同步更新调试信息。这意味着，经过努力，LLVM 优化器可以像优化非调试代码一样优化调试代码。

- **LLVM 调试信息不会阻止优化的发生**（例如内联、基本块重排/合并/清理、尾部复制等）。

- **LLVM 调试信息会随程序其余部分一起自动优化**，使用现有工具。例如，重复信息会被链接器自动合并，未使用的信息会被自动移除。

基本上，调试信息允许您以 `-O0 -g` 编译程序并获得完整的调试信息，使您可以在调试器中任意修改程序的执行状态。以 `-O3 -g` 编译程序会给您完整的调试信息，这些信息始终可用且可以准确读取（例如，即使进行了尾调用消除和内联，您仍然可以获得准确的堆栈跟踪），但您可能会失去修改程序以及调用已被优化掉或完全内联的函数的能力。

---

## 变量与变量片段

在本文档中，"变量"泛指任何可以具有值的源语言对象，至少包括：

- 变量（Variables）
- 常量（Constants）
- 形式参数（Formal parameters）

> **注意：** 目前 LLVM 中对"真正的"常量没有特殊支持，它们被当作局部变量或全局变量处理。

变量由**局部变量**或**全局变量**元数据节点表示。

**"变量片段"**（或简称"片段"）是变量中连续的比特区间。

引用以 `DW_OP_LLVM_fragment` 操作结尾的 `DIExpression` 的调试记录描述了它所引用变量的一个片段。

`DW_OP_LLVM_fragment` 操作的操作数分别编码片段相对于变量起始处的比特偏移量和片段的比特大小。

> **注意：** `DW_OP_LLVM_fragment` 操作仅用于编码片段信息，不影响包含它的表达式的语义。

---

## 调试信息格式

LLVM 调试信息经过精心设计，使优化器可以在不了解调试信息任何内容的情况下优化程序和调试信息。特别是，使用元数据从一开始就避免了重复的调试信息，全局死代码消除 pass 在决定删除一个函数时会自动删除该函数的调试信息。

为此，大多数调试信息（类型、变量、函数、源文件等的描述符）由语言前端以 LLVM 元数据的形式插入。

调试信息被设计为与目标调试器和调试信息表示（如 DWARF/Stabs 等）无关。它使用一个通用 pass 来解码表示变量、类型、函数、命名空间等的信息：这允许使用任意的源语言语义和类型系统，只要有一个为目标调试器解释信息而编写的模块即可。

为了提供基本功能，LLVM 调试器确实需要对被调试的源级语言做一些假设，但尽量将其保持在最低限度。LLVM 调试器假定存在的唯一公共特性是**源文件**和**程序对象**。这些抽象对象被调试器用来形成堆栈跟踪、显示局部变量信息等。

本节文档首先描述任何源语言通用的表示方面。[C/C++ 前端特定调试信息](#cc-前端特定调试信息)描述了 C 和 C++ 前端使用的数据布局约定。

调试信息描述符是专门的元数据节点，是 `Metadata` 的一级子类。

有两种模型用于在程序不同状态下定义源变量的值，并在优化和代码生成过程中跟踪这些值：**调试记录**（当前默认）和**内部函数调用**（非默认，目前为向后兼容而支持）——这两种模型绝不能在同一个 IR 模块中混用。有关我们为什么切换到新模型、它如何工作以及如何更新旧代码或 IR 以使用调试记录的说明，请参阅 [RemoveDIs 文档](https://llvm.org/docs/RemoveDIsDebugInfo.html)。

---

## 调试记录（Debug Records）

调试记录定义了程序执行期间源变量的值；它们与指令交错出现，但本身不是指令，对编译器生成的代码没有任何影响。

LLVM 使用几种类型的调试记录来定义源变量。这些记录的通用语法为：

```
  #dbg_<kind>([<arg>, ]* <DILocation>)
; 使用内部函数模型时，以上等价于：
call void llvm.dbg.<kind>([metadata <arg>, ]*), !dbg <DILocation>
```

调试记录总是比指令多一级缩进，并始终带有前缀 `#dbg_` 以及括号中以逗号分隔的参数列表（如同函数调用）。

### #dbg_declare

```
#dbg_declare([Value|MDNode], DILocalVariable, DIExpression, DILocation)
```

此记录提供关于局部元素（如变量）的信息。第一个参数是对应于变量地址的 SSA 指针值，通常是函数入口块中的静态 alloca。第二个参数是包含变量描述的局部变量。第三个参数是复杂表达式。第四个参数是源位置。`#dbg_declare` 记录描述了源变量的**地址**。

```llvm
%i.addr = alloca i32, align 4
  #dbg_declare(ptr %i.addr, !1, !DIExpression(), !2)
; ...
!1 = !DILocalVariable(name: "i", ...) ; int i
!2 = !DILocation(...)
; ...
%buffer = alloca [256 x i8], align 8
; i 的地址是 buffer+64
  #dbg_declare(ptr %buffer, !3, !DIExpression(DW_OP_plus, 64), !4)
; ...
!3 = !DILocalVariable(name: "i", ...) ; int i
!4 = !DILocation(...)
```

前端应在源变量声明点恰好生成一条 `#dbg_declare` 记录。将变量从内存完全提升为 SSA 值的优化 pass 会将此记录替换为可能多条的 `#dbg_value` 记录。删除 store 的 pass 实际上是部分提升，它们会插入混合的 `#dbg_value` 记录来跟踪变量值可用时的情况。优化后，可能存在多条 `#dbg_declare` 记录描述变量驻留在内存中的程序点。对同一具体源变量的所有调用必须在内存位置上保持一致。

### #dbg_value

```
#dbg_value([Value|DIArgList|MDNode], DILocalVariable, DIExpression, DILocation)
```

此记录提供用户源变量被设置为新值时的信息。第一个参数是新值。第二个参数是包含变量描述的局部变量。第三个参数是复杂表达式。第四个参数是源位置。

`#dbg_value` 记录直接描述源变量的**值**，而不是其地址。注意此内部函数的值操作数可以是间接的（即指向源变量的指针），前提是对复杂表达式求值后能得到直接值。

### #dbg_declare_value

```
#dbg_declare_value([Value|MDNode], DILocalVariable, DIExpression, DILocation)
```

此记录提供关于局部元素（如变量）的信息。第一个参数用于在整个函数中计算变量的值。第二个参数是包含变量描述的局部变量。第三个参数是复杂表达式。第四个参数是源位置。`#dbg_declare_value` 记录直接描述源变量的**值**，而不是其地址。`#dbg_value` 和 `#dbg_declare_value` 的区别在于：与 `#dbg_declare` 类似，前端应当恰好生成一条 `#dbg_declare_value` 记录。其思想是具备 `#dbg_declare` 的保证，但能够描述值而非值的地址。

### #dbg_assign

```
#dbg_assign( [Value|DIArgList|MDNode] Value,
             DILocalVariable Variable,
             DIExpression ValueExpression,
             DIAssignID ID,
             [Value|MDNode] Address,
             DIExpression AddressExpression,
             DILocation SourceLocation )
```

此记录标记 IR 中发生源赋值的位置。它编码变量的值，引用执行赋值的 store（如果有），以及目标地址。

前三个参数与 `#dbg_value` 相同。第四个参数是用于引用 store 的 `DIAssignID`。第五个是 store 的目标，第六个是修改它的复杂表达式，第七个是源位置。

更多信息请参阅[调试信息赋值追踪（Debug Info Assignment Tracking）](https://llvm.org/docs/AssignmentTracking.html)。

---

## 调试器内部函数（已废弃）

> **警告：** 这些内部函数已废弃，请改用调试记录。更多详情请参阅 [RemoveDIs 文档](https://llvm.org/docs/RemoveDIsDebugInfo.html)。

在内部函数模式下，LLVM 使用若干内部函数（名称以 `llvm.dbg` 为前缀）在优化和代码生成过程中跟踪源局部变量。每个内部函数对应上述调试记录之一，有若干语法差异：调试器内部函数的每个参数必须包装为元数据（即必须以 `metadata` 为前缀），且每条记录中的 `DILocation` 参数必须作为元数据附件附加到调用指令上（即出现在参数列表之后，带有前缀 `!dbg`）。

### llvm.dbg.declare

```llvm
void @llvm.dbg.declare(metadata, metadata, metadata)
```

此内部函数等价于 `#dbg_declare`：

```llvm
  #dbg_declare(i32* %i.addr, !1, !DIExpression(), !2)
call void @llvm.dbg.declare(metadata i32* %i.addr, metadata !1,
                            metadata !DIExpression()), !dbg !2
```

### llvm.dbg.value

```llvm
void @llvm.dbg.value(metadata, metadata, metadata)
```

此内部函数等价于 `#dbg_value`：

```llvm
  #dbg_value(i32 %i, !1, !DIExpression(), !2)
call void @llvm.dbg.value(metadata i32 %i, metadata !1,
                          metadata !DIExpression()), !dbg !2
```

### llvm.dbg.assign

```llvm
void @llvm.dbg.assign(metadata, metadata, metadata, metadata, metadata, metadata)
```

此内部函数等价于 `#dbg_assign`：

```llvm
  #dbg_assign(i32 %i, !1, !DIExpression(), !2,
              ptr %i.addr, !DIExpression(), !3)
call void @llvm.dbg.assign(
  metadata i32 %i, metadata !1, metadata !DIExpression(), metadata !2,
  metadata ptr %i.addr, metadata !DIExpression(), metadata !3), !dbg !3
```

---

## DIExpression

调试表达式表示为[专门化元数据节点（Specialized Metadata Nodes）](https://llvm.org/docs/LangRef.html#specialized-metadata)。

调试表达式从左到右解释：首先将记录的值/地址操作数压入栈，然后依次压入并求值 `DIExpression` 中的操作码，直到产生最终的变量描述。

这些表达式中可用的操作码在 [DWARF 操作码](#dwarf-操作码)和[内部操作码](#内部操作码)中描述。

DWARF 规定了三种简单位置描述：寄存器、内存和隐式位置描述。注意，位置描述在程序的特定范围内定义，即变量的位置可能在程序执行过程中发生变化。寄存器和内存位置描述描述源变量的具体位置（在调试器可能修改其值的意义上），而隐式位置仅描述源变量的实际值，该值可能不存在于寄存器或内存中（参见 `DW_OP_stack_value`）。

`#dbg_declare` 记录描述源变量的间接值（地址）。记录的第一个操作数必须是某种类型的地址。记录的 `DIExpression` 操作数对该地址进行细化，以产生源变量的具体位置。

`#dbg_value` 记录描述源变量的直接值。记录的第一个操作数可以是直接值或间接值。记录的 `DIExpression` 操作数对第一个操作数进行细化以产生直接值。例如，如果第一个操作数是间接值，则可能需要在 `DIExpression` 中插入 `DW_OP_deref` 以产生有效的调试记录。

> **注意：** 无论 `DIExpression` 附加到哪种调试记录，其解释方式都相同。

`DIExpression` 始终以内联形式打印和解析；它们永远不能通过 ID（如 `!1`）引用。

---

## DWARF 操作码

LLVM 在可能的情况下复用 DWARF 操作码，并在 LLVM 表达式中赋予它们与 DWARF 表达式中相同的语义。当前支持的操作码词汇有限，但至少包括：

- **`DW_OP_deref`**：对表达式栈顶进行解引用。
- **`DW_OP_plus`**：弹出表达式栈中最后两项，相加后将结果压回栈。
- **`DW_OP_minus`**：弹出表达式栈中最后两项，用倒数第二项减去最后一项，将结果追加到表达式栈。
- **`DW_OP_plus_uconst, 93`**：将 93 加到栈顶值上。
- **`DW_OP_swap`**：交换栈顶两个条目。
- **`DW_OP_xderef`**：提供扩展解引用机制。栈顶条目被视为地址，第二个栈条目被视为地址空间标识符。弹出两个条目后，将一个实现定义的值压入栈。
- **`DW_OP_stack_value`**：在表达式中最多出现一次，且必须是最后一个操作码（若 `DW_OP_LLVM_fragment` 不存在）或倒数第二个操作码（若存在 `DW_OP_LLVM_fragment`）。弹出表达式栈的顶值，并以该值创建一个隐式值位置。
- **`DW_OP_breg`**（或 `DW_OP_bregx`）：表示指定寄存器的有符号偏移量处的内容。此操作码仅由 AsmPrinter pass 生成，用于描述需要对两个寄存器进行表达式运算的调用点参数值。
- **`DW_OP_push_object_address`**：将对象的地址压栈，可随后作为计算中的描述符。此操作码可用于计算具有数组描述符的 Fortran 可分配数组的边界。
- **`DW_OP_over`**：将当前栈中第二个条目复制到栈顶。此操作码可用于计算具有运行时已知秩的 Fortran 假定秩数组的边界，当前维度号隐式为栈的第一个元素。

---

## 内部操作码

在 DWARF 等价物不适用或不存在 DWARF 等价物的情况下，LLVM 定义了在 DWARF 中没有直接对应物的内部专用操作码。

> **注意：** 某些操作码不直接影响最终的 DWARF 表达式，而是编码逻辑上属于使用它们的调试记录的信息。

- **`DW_OP_LLVM_fragment, <offset>, <size>`**：在表达式中最多出现一次，且必须是最后一个操作码。它指定由使用该表达式的记录或内部函数所描述的变量片段的比特偏移量和比特大小。注意，与 `DW_OP_bit_piece` 相反，偏移量描述的是所描述源变量内的位置。在 DWARF 生成时，同一变量的所有片段会被收集在一起，使用 DWARF 的 `DW_OP_piece` 和 `DW_OP_bit_piece` 操作码来描述由对应片段组成的复合体（这不影响包含它的表达式的语义）。

- **`DW_OP_LLVM_convert, 16, DW_ATE_signed`**：指定比特大小和编码（此处分别为 16 和 `DW_ATE_signed`），表达式栈顶将被转换为该类型。映射为一个引用由所提供值构造的基类型的 `DW_OP_convert` 操作。

- **`DW_OP_LLVM_tag_offset, tag_offset`**：指定一个内存标签应可选地应用于指针。内存标签以实现定义的方式从给定的标签偏移量派生（这不影响包含它的表达式的语义）。

- **`DW_OP_LLVM_entry_value, N`**：将一个子表达式作为在当前调用帧入口处求值来求值。

  子表达式取代了组成它的操作，即所有这些操作仅在帧入口上下文中求值。

  子表达式从 `DIExpression` 中紧接在 `DW_OP_LLVM_entry_value, N` 之前的操作开始。如果不存在这样的操作（即表达式以 `DW_OP_LLVM_entry_value, N` 开头），则使用隐式操作（压入包含标记/伪指令的第一个调试参数）。值 `N` 必须始终至少为 1，因为第一个操作不能省略且计入 N 中。

  子表达式的其余部分由 `DIExpression` 中 `DW_OP_LLVM_entry_value, N` 之后的 (N-1) 个操作组成。

  由于框架限制：
  - `N` 不得大于 1，即 `N` 必须等于 1，子表达式仅包含紧接在 `DW_OP_LLVM_entry_value, N` 之前的操作。
  - `DW_OP_LLVM_entry_value, N` 必须是 `DIExpression` 的第一个操作，或者如果表达式以 `DW_OP_LLVM_arg, 0` 开头，则是第二个操作。
  - 第一个操作必须引用寄存器值。

  综合这些限制，`DW_OP_LLVM_entry_value` 目前只能用于推入当前栈帧入口处单个寄存器的值。

  例如，`!DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_entry_value, 1, DW_OP_LLVM_arg, 1, DW_OP_plus, DW_OP_stack_value)` 指定一个表达式，其中 `DIExpression` 第一个参数的入口值加上第二个参数的非入口值，结果用作隐式值位置的值。

  针对 DWARF 目标时，`DBG_VALUE(reg, ..., DIExpression(DW_OP_LLVM_entry_value, 1, ...))` 被降低为 `DW_OP_entry_value [reg], ...`，将 `reg` 在帧入口时的值压入 DWARF 表达式栈。

  由于 `DW_OP_LLVM_entry_value` 目前限于寄存器，它通常用于 MIR，但在针对 `swiftasync` 参数时，也允许在 LLVM IR 中使用。该操作由以下 pass 引入：

  - **LiveDebugValues pass**：将其应用于在整个函数中未被修改的函数参数。支持仅限于简单的寄存器位置描述，或间接位置（例如，通过调用者中临时副本的指针按值传递给被调用者的参数）。
  - **AsmPrinter pass**：当调用点参数值（`DW_AT_call_site_parameter_value`）表示为参数的入口值时。
  - **CoroSplit pass**：可能将变量从 alloca 移动到协程帧中。如果协程帧是 `swiftasync` 参数，则变量用 `DW_OP_LLVM_entry_value` 操作描述。

- **`DW_OP_LLVM_implicit_pointer`**：指定解引用后的值。可用于表示已优化掉的指针变量，但其所指向的值是已知的。此操作符是必需的，因为它在表示和规范（操作数的数量和类型）上与 DWARF 操作符 `DW_OP_implicit_pointer` 不同，且后者不能用于多级指针。

  使用 `DW_OP_LLVM_implicit_pointer` 的示例：

  ```llvm
  ; IR 对应 "*ptr = 4;"
    #dbg_value(i32 4, !17, !DIExpression(DW_OP_LLVM_implicit_pointer), !20)
  !17 = !DILocalVariable(name: "ptr", scope: !12, file: !3, line: 5,
                         type: !18)
  !18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
  !19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
  !20 = !DILocation(line: 10, scope: !12)

  ; IR 对应 "**ptr = 4;"
    #dbg_value(i32 4, !17,
      !DIExpression(DW_OP_LLVM_implicit_pointer, DW_OP_LLVM_implicit_pointer),
      !21)
  !17 = !DILocalVariable(name: "ptr", scope: !12, file: !3, line: 5,
                         type: !18)
  !18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
  !19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
  !20 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
  !21 = !DILocation(line: 10, scope: !12)
  ```

- **`DW_OP_LLVM_arg, N`**：用于引用多个值的调试内部函数，例如计算两个寄存器之和的函数。它总是与一个有序值列表配合使用，`DW_OP_LLVM_arg, N` 引用该列表中的第 N 个元素。例如，与列表 `(%reg1, %reg2)` 一起使用的 `!DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_minus, DW_OP_stack_value)` 将求值为具有 `%reg1 - reg2` 值的隐式值位置。该值列表应由包含的内部函数/指令提供。

- **`DW_OP_LLVM_extract_bits_sext, 16, 8`**：指定要从表达式栈顶值提取并进行符号扩展的比特的偏移量和大小（此处分别为 16 和 8）。如果栈顶是内存位置，则从该内存位置指向的值中提取这些比特。映射为一个 `DW_OP_shl` 后跟 `DW_OP_shra`。

- **`DW_OP_LLVM_extract_bits_zext`**：行为类似于 `DW_OP_LLVM_extract_bits_sext`，但使用零扩展而不是符号扩展。映射为一个 `DW_OP_shl` 后跟 `DW_OP_shr`。

---

## 对象生命周期与作用域

在许多语言中，函数中的局部变量的生命周期或作用域可以限制在函数的一个子集内。例如，在 C 语言家族中，变量只在定义它们的源代码块内是活跃的（可读写）。在函数式语言中，值只在定义后才可读。虽然这是一个非常明显的概念，但在 LLVM 中建模是非平凡的，因为 LLVM 没有这种意义上的作用域概念，且不希望与语言的作用域规则绑定。

为了处理这个问题，LLVM 调试格式使用附加到 LLVM 指令的元数据来编码行号和作用域信息。以下面的 C 片段为例：

```c
1.  void foo() {
2.    int X = 21;
3.    int Y = 22;
4.    {
5.      int Z = 23;
6.      Z = X;
7.    }
8.    X = Y;
9.  }
```

编译为 LLVM 后，此函数将表示如下：

```llvm
; Function Attrs: nounwind ssp uwtable
define void @foo() #0 !dbg !4 {
entry:
  %X = alloca i32, align 4
  %Y = alloca i32, align 4
  %Z = alloca i32, align 4
    #dbg_declare(ptr %X, !11, !DIExpression(), !13)
  store i32 21, i32* %X, align 4, !dbg !13
    #dbg_declare(ptr %Y, !14, !DIExpression(), !15)
  store i32 22, i32* %Y, align 4, !dbg !15
    #dbg_declare(ptr %Z, !16, !DIExpression(), !18)
  store i32 23, i32* %Z, align 4, !dbg !18
  %0 = load i32, i32* %X, align 4, !dbg !20
  store i32 %0, i32* %Z, align 4, !dbg !21
  %1 = load i32, i32* %Y, align 4, !dbg !22
  store i32 %1, i32* %X, align 4, !dbg !23
  ret void, !dbg !24
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8, !9}
!llvm.ident = !{!10}

!0 = !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.7.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, ...)
!1 = !DIFile(filename: "/dev/stdin", directory: "/Users/dexonsmith/data/llvm/debug-info")
!4 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !5, ...)
!11 = !DILocalVariable(name: "X", scope: !4, file: !1, line: 2, type: !12)
!12 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!13 = !DILocation(line: 2, column: 9, scope: !4)
!14 = !DILocalVariable(name: "Y", scope: !4, file: !1, line: 3, type: !12)
!15 = !DILocation(line: 3, column: 9, scope: !4)
!16 = !DILocalVariable(name: "Z", scope: !18, file: !1, line: 5, type: !12)
!17 = distinct !DILexicalBlock(scope: !4, file: !1, line: 4, column: 5)
!18 = !DILocation(line: 5, column: 11, scope: !17)
```

此示例说明了关于 LLVM 调试信息的几个重要细节。特别是，它展示了 `#dbg_declare` 记录和位置信息（附加到指令上）如何共同作用，允许调试器分析语句、变量定义与实现函数所用代码之间的关系。

```llvm
#dbg_declare(ptr %X, !11, !DIExpression(), !13)
; [debug line = 2:9] [debug variable = X]
```

第一条记录 `#dbg_declare` 编码变量 X 的调试信息。记录末尾的位置 `!13` 提供了变量 X 的作用域信息。

```llvm
!13 = !DILocation(line: 2, column: 9, scope: !4)
!4 = distinct !DISubprogram(name: "foo", ...)
```

这里 `!13` 是提供位置信息的元数据。在此示例中，作用域由 `!4`（一个子程序描述符）编码。这样，记录的位置信息参数表明变量 X 在函数 `foo` 的函数级作用域中的第 2 行第 9 列声明。

再来看另一个例子：

```llvm
#dbg_declare(ptr %Z, !16, !DIExpression(), !18)
; [debug line = 5:11] [debug variable = Z]
```

第三条记录 `#dbg_declare` 编码变量 Z 的调试信息。记录末尾的元数据 `!18` 提供了变量 Z 的作用域信息。

```llvm
!17 = distinct !DILexicalBlock(scope: !4, file: !1, line: 4, column: 5)
!18 = !DILocation(line: 5, column: 11, scope: !17)
```

这里 `!18` 表明 Z 在词法作用域 `!17` 内的第 5 行第 11 列声明。词法作用域本身位于上述子程序 `!4` 内部。

附加到每条指令的作用域信息提供了一种直接的方法来查找被某个作用域覆盖的指令。

---

## 优化代码中的对象生命周期

在上面的示例中，每次变量赋值都唯一对应于对栈上变量位置的内存 store。然而，在高度优化的代码中，LLVM 将大多数变量提升为 SSA 值，这些值最终可以放置在物理寄存器或内存位置中。为了在编译过程中跟踪 SSA 值，当对象被提升为 SSA 值时，每次赋值都会创建一条 `#dbg_value` 记录，记录变量的新位置。与 `#dbg_declare` 记录相比：

- `#dbg_value` 终止任何前置记录对公共变量公共比特的影响。

  > **注意：** 当前实现通常会在任何效果需要终止时完全终止每条记录的效果，而不是像定义允许的那样继续传递前置记录对非重叠比特的效果。这被允许，就像在编译的任何时刻丢弃调试信息都被允许一样。一个例外是[调试信息赋值追踪（Debug Info Assignment Tracking）](https://llvm.org/docs/AssignmentTracking.html)，其中某些基于内存的位置在某些情况下会被部分传递。

- `#dbg_value` 在 IR 中的位置定义了变量值在指令流中的变化点。
- 操作数可以是常量，表示变量被赋予了常量值。

当优化 pass 修改或移动指令和基本块时，必须注意更新 `#dbg_value` 记录——开发者可能会在调试程序时观察到这些变化反映在变量值中。对于优化程序的任何执行，调试器向开发者呈现的变量值集合不应显示在未优化程序执行中（给定相同输入）从未存在过的状态。这样做有误导开发者的风险——报告不存在的状态，损害他们对优化程序的理解，并破坏他们对调试器的信任。

有时完美保留变量位置是不可能的，通常是当冗余计算被优化掉时。在这种情况下，应使用操作数为 `poison` 的 `#dbg_value`，以终止较早的变量位置并让调试器向开发者呈现"已优化掉"。从开发者那里隐藏这些潜在陈旧的变量值减少了可用调试信息的数量，但提高了剩余信息的可靠性。

为了说明一些潜在问题，考虑以下示例：

```llvm
define i32 @foo(i32 %bar, i1 %cond) {
entry:
    #dbg_value(i32 0, !1, !DIExpression(), !4)
  br i1 %cond, label %truebr, label %falsebr
truebr:
  %tval = add i32 %bar, 1
    #dbg_value(i32 %tval, !1, !DIExpression(), !4)
  %g1 = call i32 @gazonk()
  br label %exit
falsebr:
  %fval = add i32 %bar, 2
    #dbg_value(i32 %fval, !1, !DIExpression(), !4)
  %g2 = call i32 @gazonk()
  br label %exit
exit:
  %merge = phi [ %tval, %truebr ], [ %fval, %falsebr ]
  %g = phi [ %g1, %truebr ], [ %g2, %falsebr ]
    #dbg_value(i32 %merge, !1, !DIExpression(), !4)
    #dbg_value(i32 %g, !3, !DIExpression(), !4)
  %plusten = add i32 %merge, 10
  %toret = add i32 %plusten, %g
    #dbg_value(i32 %toret, !1, !DIExpression(), !4)
  ret i32 %toret
}
```

包含 `!1` 和 `!3` 两个源级变量。该函数可能被优化为以下代码：

```llvm
define i32 @foo(i32 %bar, i1 %cond) {
entry:
  %g = call i32 @gazonk()
  %addoper = select i1 %cond, i32 11, i32 12
  %plusten = add i32 %bar, %addoper
  %toret = add i32 %plusten, %g
  ret i32 %toret
}
```

为了避免 `!1` 具有常量值零的同时 `!3` 具有 `@gazonk()` 的返回值（这在未优化程序中从未发生过的赋值对），必须在 `!3` 的 `#dbg_value` 之前插入一个 `poison` 的 `#dbg_value` 来终止 `!1` 具有常量值赋值的范围：

```llvm
define i32 @foo(i32 %bar, i1 %cond) {
entry:
    #dbg_value(i32 0, !1, !DIExpression(), !2)
  %g = call i32 @gazonk()
    #dbg_value(i32 poison, !1, !DIExpression(), !2)
    #dbg_value(i32 %g, !3, !DIExpression(), !2)
  %addoper = select i1 %cond, i32 11, i32 12
  %plusten = add i32 %bar, %addoper
  %toret = add i32 %plusten, %g
    #dbg_value(i32 %toret, !1, !DIExpression(), !2)
  ret i32 %toret
}
```

有几种其他 `#dbg_value` 配置意味着它终止主导位置定义而不添加新位置，完整列表为：

- 任何位置操作数是 `poison`（或 `undef`）。
- 任何位置操作数是空元数据元组（`!{}`，不能出现在 `!DIArgList` 中）。
- 没有位置操作数（空 `DIArgList`）且 `DIExpression` 为空。

这类终止变量位置的 `#dbg_value` 称为"kill `#dbg_value`"或"kill location"，出于历史原因，现有代码中可能使用术语"undef `#dbg_value`"。应尽可能使用 `DbgVariableIntrinsic` 的 `isKillLocation` 和 `setKillLocation` 方法来检查或设置一个 `#dbg_value` 是否为 kill location，而不是直接检查位置操作数。

一般来说，如果任何 `#dbg_value` 的操作数被优化掉且无法恢复，则需要一个 kill `#dbg_value` 来终止较早的变量位置。当调试器可以观察到赋值的重新排序时，可能需要额外的 kill `#dbg_value`。

---

## 变量位置元数据在 CodeGen 中的变换

LLVM 在中端和后端 pass 中保留调试信息，最终生成源级信息和指令范围之间的映射。对于行号信息，这相对简单，因为将指令映射到行号是一种简单的关联。然而对于变量位置，情况更为复杂。由于每条 `#dbg_value` 记录表示一次对源变量的源级赋值，调试记录实际上在 LLVM IR 中嵌入了一个小型命令式程序。到 CodeGen 结束时，这变成了从每个变量到其在指令范围内的机器位置的映射。从 IR 到目标文件发射，影响变量位置保真度的主要变换是：

1. 指令选择（Instruction Selection）
2. 寄存器分配（Register allocation）
3. 基本块布局（Block layout）

此外，指令调度会显著改变程序的顺序，并在多个不同的 pass 中发生。

某些变量位置在 CodeGen 中不发生变换。由 `#dbg_declare` 指定的栈位置在整个函数持续时间内有效且不变，并记录在简单的 `MachineFunction` 表中。函数序言和尾声中的位置变化也被忽略：帧的建立和销毁可能需要若干指令，在输出二进制文件中描述它们需要不成比例的大量调试信息，调试器无论如何都应该步过（step over）它们。

### 指令选择和 MIR 中的变量位置

指令选择从 IR 函数创建 MIR 函数，就像它将中间指令转换为机器指令一样，中间变量位置也必须变成机器变量位置。在 IR 中，变量位置总是由 `Value` 标识，但在 MIR 中可能有不同类型的变量位置。此外，某些 IR 位置会变为不可用——例如，如果多条 IR 指令的操作被合并为一条机器指令（如乘法累加），则中间 `Value` 会丢失。为了在指令选择过程中跟踪变量位置，它们首先被分为不依赖于代码生成的位置（常量、栈位置、已分配的虚拟寄存器）和依赖于代码生成的位置。对于后者，调试元数据附加到 `SelectionDAG` 中的 `SDNode` 上。指令选择完成且 MIR 函数创建后，如果与调试元数据关联的 `SDNode` 被分配了虚拟寄存器，则该虚拟寄存器被用作变量位置。如果 `SDNode` 被折叠进机器指令或以其他方式转换为非寄存器，则变量位置变为不可用。

不可用的位置被视为已优化掉：在 IR 中，位置将由调试记录赋值为 `undef`，在 MIR 中使用等价的位置。

在将 MIR 位置分配给每个变量后，会插入与每条 `#dbg_value` 记录对应的机器伪指令。有两种形式：

**第一种形式，DBG_VALUE：**

```
DBG_VALUE %1, $noreg, !123, !DIExpression()
```

操作数说明：
- 第一个操作数可以将变量位置记录为寄存器、帧索引、立即数，或者如果原始调试记录引用内存则为基地址寄存器。`$noreg` 表示变量位置未定义，等价于 `undef` 的 `#dbg_value` 操作数。
- 第二个操作数的类型指示 `DBG_VALUE` 是直接引用还是间接引用变量位置。`$noreg` 寄存器表示前者，立即数操作数（0）表示后者。
- 第三个操作数是原始调试记录的 `Variable` 字段。
- 第四个操作数是原始调试记录的 `Expression` 字段。

**第二种形式，DBG_VALUE_LIST：**

```
DBG_VALUE_LIST !123, !DIExpression(DW_OP_LLVM_arg, 0, DW_OP_LLVM_arg, 1, DW_OP_plus), %1, %2
```

操作数说明：
- 第一个操作数是原始调试记录的 `Variable` 字段。
- 第二个操作数是原始调试记录的 `Expression` 字段。
- 从第三个操作数起的任意数量操作数记录一系列变量位置操作数，可取与上述 `DBG_VALUE` 指令第一个操作数相同的任何值。这些变量位置操作数插入到最终 DWARF 表达式中 `DIExpression` 中 `DW_OP_LLVM_arg` 操作符指示的位置。

`DBG_VALUE` 的插入位置应对应于 IR 块中匹配的 `#dbg_value` 记录的位置。与优化一样，LLVM 旨在保留变量赋值在源程序中发生的顺序。然而，`SelectionDAG` 会进行一些指令调度，可能会重新排序赋值（见下文）。函数参数位置若不在函数开头，则会被移至函数开头，以确保在函数入口处立即可用。

### 指令调度

多个 pass 可以重新调度指令，尤其是指令选择和 RA 前后的机器调度器。指令调度可能显著改变程序的性质——在（极不可能的）最坏情况下，指令序列可能完全颠倒。在这种情况下，LLVM 遵循优化适用的原则：调试器不显示任何状态比显示误导性状态更好。因此，无论何时指令在执行顺序上提前，任何对应的 `DBG_VALUE` 都保持在其原始位置；如果指令被延迟，则在延迟期间变量被赋予未定义位置。

以下面的伪 MIR 为例：

```
%1:gr32 = MOV32rm %0, 1, $noreg, 4, $noreg, debug-location !5
DBG_VALUE %1, $noreg, !1, !2
%4:gr32 = ADD32rr %3, %2, implicit-def dead $eflags
DBG_VALUE %4, $noreg, !3, !4
%7:gr32 = SUB32rr %6, %5, implicit-def dead $eflags
DBG_VALUE %7, $noreg, !5, !6
```

如果 `SUB32rr` 被向前移动：

```
%7:gr32 = SUB32rr %6, %5, implicit-def dead $eflags
%1:gr32 = MOV32rm %0, 1, $noreg, 4, $noreg, debug-location !5
DBG_VALUE %1, $noreg, !1, !2
%4:gr32 = ADD32rr %3, %2, implicit-def dead $eflags
DBG_VALUE %4, $noreg, !3, !4
DBG_VALUE %7, $noreg, !5, !6
```

LLVM 会保持上图所示的 MIR。调试器将看到一个较少的变量值组合（`!3` 和 `!5` 同时改变值），而不是重排赋值并引入新的程序状态，这优于对原始程序的错误表示。

相比之下，如果 `MOV32rm` 被向后移动，LLVM 会产生：

```
DBG_VALUE $noreg, $noreg, !1, !2
%4:gr32 = ADD32rr %3, %2, implicit-def dead $eflags
DBG_VALUE %4, $noreg, !3, !4
%7:gr32 = SUB32rr %6, %5, implicit-def dead $eflags
DBG_VALUE %7, $noreg, !5, !6
%1:gr32 = MOV32rm %0, 1, $noreg, 4, $noreg, debug-location !5
DBG_VALUE %1, $noreg, !1, !2
```

这里，为了避免呈现 `!1` 的第一次赋值消失的状态，块顶部的 `DBG_VALUE` 将变量赋予未定义位置，直到其值在块末尾可用时添加一个额外的 `DBG_VALUE`。这准确地反映了在原始程序的对应部分中该值不可用的事实。

### 寄存器分配期间的变量位置

为了避免调试指令干扰寄存器分配器，**LiveDebugVariables** pass 从 MIR 函数中提取变量位置并删除对应的 `DBG_VALUE` 指令，同时在块内执行一些局部复制传播。寄存器分配后，**VirtRegRewriter** pass 将 `DBG_VALUE` 指令重新插入其原始位置，将虚拟寄存器引用转换为其物理机器位置。为避免编码错误的变量位置，在此 pass 中，任何非活跃虚拟寄存器的 `DBG_VALUE` 都会被替换为未定义位置。`LiveDebugVariables` 可能因虚拟寄存器重写而插入冗余的 `DBG_VALUE`，这些将由 `RemoveRedundantDebugValues` pass 随后移除。

### LiveDebugValues 对变量位置的扩展

在所有优化运行完毕且临近发射之前，**LiveDebugValues** pass 运行以实现两个目标：

1. 通过复制和寄存器溢出传播变量位置。
2. 对于每个基本块，记录该块中每个有效的变量位置。

此 pass 运行后，`DBG_VALUE` 指令的含义发生变化：它不再对应于变量可能改变值的源级赋值，而是断言变量在块中的位置，且在块外失效。通过复制和溢出传播变量位置是直接的；确定每个基本块中的变量位置则需要考虑控制流。

考虑以下 IR，它呈现了若干困难：

```llvm
define dso_local i32 @foo(i1 %cond, i32 %input) !dbg !12 {
entry:
  br i1 %cond, label %truebr, label %falsebr

bb1:
  %value = phi i32 [ %value1, %truebr ], [ %value2, %falsebr ]
  br label %exit, !dbg !26

truebr:
    #dbg_value(i32 %input, !30, !DIExpression(), !24)
    #dbg_value(i32 1, !23, !DIExpression(), !24)
  %value1 = add i32 %input, 1
  br label %bb1

falsebr:
    #dbg_value(i32 %input, !30, !DIExpression(), !24)
    #dbg_value(i32 2, !23, !DIExpression(), !24)
  %value2 = add i32 %input, 2
  br label %bb1

exit:
  ret i32 %value, !dbg !30
}
```

困难包括：
- 控制流大致与基本块顺序相反。
- 变量 `!23` 的值在 `%bb1` 中合并，但没有 PHI 节点。

`#dbg_value` 记录本质上形成嵌入在 IR 中的命令式程序，每条记录定义一个变量位置。这可以像 `mem2reg` 一样转换为 SSA 形式，使用 use-def 链识别控制流合并并为 IR 值插入 phi 节点。然而，由于调试变量位置是为每条机器指令定义的，实际上每条 IR 指令都使用每个变量位置，这将导致生成大量调试记录。

`LiveDebugValues` 通过数据流分析传播块间位置来确定变量位置：当控制流合并时，如果变量在所有前驱中具有相同位置，则该位置传播到后继。如果前驱位置不一致，则位置变为未定义。

`LiveDebugValues` 运行后，每个块应通过块内的 `DBG_VALUE` 指令描述所有有效的变量位置。支持类（如 `DbgEntityHistoryCalculator`）只需极少工作即可建立每条指令到所有有效变量位置的映射，而无需考虑控制流。

---

## C/C++ 前端特定调试信息

C 和 C++ 前端以在信息内容上实际上与 DWARF 相同的格式表示程序信息。这使代码生成器能够通过生成标准 DWARF 信息来轻松支持原生调试器，并包含足够的信息让非 DWARF 目标按需翻译。

本节描述用于表示 C 和 C++ 程序的形式。其他语言可以参照这种模式（它本身针对以 DWARF 的方式表示程序进行了调优），或者如果不适合 DWARF 模型，可以选择提供完全不同的形式。规范性参考是 `include/llvm/IR/DebugInfoMetadata.h` 中定义的 `DINode` 类以及 `lib/IR/DIBuilder.cpp` 中辅助函数的实现。

### C/C++ 源文件信息

`llvm::Instruction` 提供了对附加到指令的元数据的便捷访问。可以使用 `Instruction::getDebugLoc()` 和 `DILocation::getLine()` 提取 LLVM IR 中编码的行号信息：

```cpp
if (DILocation *Loc = I->getDebugLoc()) { // I 是一条 LLVM 指令
  unsigned Line = Loc->getLine();
  StringRef File = Loc->getFilename();
  StringRef Dir = Loc->getDirectory();
  bool ImplicitCode = Loc->isImplicitCode();
}
```

当 `ImplicitCode` 标志为 `true` 时，表示该指令由前端添加，但不对应用户编写的源代码。例如：

```cpp
if (MyBoolean) {
  MyObject MO;
  ...
}
```

在作用域结束时会调用 `MyObject` 的析构函数，但用户并未显式写出。这一信息在进行代码覆盖率分析时用于避免在括号上计数。

### C/C++ 全局变量信息

对于如下声明的整数全局变量：

```c
_Alignas(8) int MyGlobal = 100;
```

C/C++ 前端会生成以下描述符：

```llvm
@MyGlobal = global i32 100, align 8, !dbg !0

!llvm.dbg.cu = !{!1}
!llvm.module.flags = !{!6, !7}
!llvm.ident = !{!8}

!0 = distinct !DIGlobalVariable(name: "MyGlobal", scope: !1, file: !2, line: 1, type: !5, isLocal: false, isDefinition: true, align: 64)
!1 = distinct !DICompileUnit(language: DW_LANG_C99, file: !2,
                             producer: "clang version 4.0.0",
                             isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug,
                             enums: !3, globals: !4)
!2 = !DIFile(filename: "/dev/stdin", directory: "/Users/dexonsmith/data/llvm/debug-info")
!3 = !{}
!4 = !{!0}
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{!"clang version 4.0.0"}
```

`DIGlobalVariable` 描述中的 `align` 值指定了变量对齐方式，当它由 C11 的 `_Alignas()`、C++11 的 `alignas()` 关键字或编译器属性 `__attribute__((aligned()))` 强制指定时使用。其他情况下（此字段缺失时），对齐被认为是默认值。这在为 `DW_AT_alignment` 值生成 DWARF 输出时使用。

### C/C++ 函数信息

对于如下声明的函数：

```c
int main(int argc, char *argv[]) {
  return 0;
}
```

C/C++ 前端会生成以下描述符：

```llvm
!4 = !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !5,
                   isLocal: false, isDefinition: true, scopeLine: 1,
                   flags: DIFlagPrototyped, isOptimized: false,
                   retainedNodes: !2)

define i32 @main(i32 %argc, i8** %argv) !dbg !4 {
...
}
```

---

## C++ 特定调试信息

### C++ 特殊成员函数信息

DWARF v5 引入了用于增强 C++ 程序调试信息的属性。LLVM 可以生成（或省略）这些适当的 DWARF 属性。在 C++ 中，特殊成员函数（构造函数、析构函数、拷贝/移动构造函数、赋值运算符）可以用 C++11 关键字 `deleted` 声明。这在 LLVM 中使用 `spFlags` 值 `DISPFlagDeleted` 表示。

对于带有已删除拷贝构造函数的类声明：

```cpp
class foo {
 public:
   foo(const foo&) = delete;
};
```

C++ 前端会生成：

```llvm
!17 = !DISubprogram(name: "foo", scope: !11, file: !1, line: 5, type: !18,
                    scopeLine: 5, flags: DIFlagPublic | DIFlagPrototyped,
                    spFlags: DISPFlagDeleted)
```

并产生额外的 DWARF 属性：

```
DW_TAG_subprogram [7] *
  DW_AT_name [DW_FORM_strx1]    (indexed (00000006) string = "foo")
  DW_AT_decl_line [DW_FORM_data1]       (5)
  ...
  DW_AT_deleted [DW_FORM_flag_present]  (true)
```

---

## Fortran 特定调试信息

### Fortran 函数信息

有若干 DWARF 属性支持 Fortran 程序的客户端调试。LLVM 可以为 `ELEMENTAL`、`PURE`、`IMPURE`、`RECURSIVE` 和 `NON_RECURSIVE` 等前缀规格生成（或省略）相应的 DWARF 属性，使用的 `spFlags` 值为 `DISPFlagElemental`、`DISPFlagPure` 和 `DISPFlagRecursive`。

对于 Fortran 基本函数：

```fortran
elemental function elem_func(a)
```

Fortran 前端会生成：

```llvm
!11 = distinct !DISubprogram(name: "subroutine2", scope: !1, file: !1,
        line: 5, type: !8, scopeLine: 6,
        spFlags: DISPFlagDefinition | DISPFlagElemental, unit: !0,
        retainedNodes: !2)
```

并产生额外的 DWARF 属性：

```
DW_TAG_subprogram [3]
   DW_AT_low_pc [DW_FORM_addr]     (0x0000000000000010 ".text")
   DW_AT_high_pc [DW_FORM_data4]   (0x00000001)
   ...
   DW_AT_elemental [DW_FORM_flag_present]  (true)
```

有若干 DWARF 标签用于表示 Fortran 特有的构造，例如 `DW_TAG_string_type` 用于表示 Fortran 的 `character(n)`，在 LLVM 中表示为 `DIStringType`：

```fortran
character(len=*), intent(in) :: string
```

Fortran 前端会生成：

```llvm
!DILocalVariable(name: "string", arg: 1, scope: !10, file: !3, line: 4, type: !15)
!DIStringType(name: "character(*)!2", stringLength: !16, stringLengthExpression: !DIExpression(), size: 32)
```

Fortran 延迟长度字符还可以在字符串长度之外包含字符原始存储的信息，该信息编码在 `stringLocationExpression` 字段中，用于在 `DW_TAG_string_type` 调试信息中发射 `DW_AT_data_location` 属性：

```llvm
!DIStringType(name: "character(*)!2", stringLengthExpression: !DIExpression(),
              stringLocationExpression: !DIExpression(DW_OP_push_object_address, DW_OP_deref),
              size: 32)
```

在 DWARF 标签中体现为：

```
DW_TAG_string_type
             DW_AT_name      ("character(*)!2")
             DW_AT_string_length     (0x00000064)
0x00000064:    DW_TAG_variable
               DW_AT_location      (DW_OP_fbreg +16)
               DW_AT_type  (0x00000083 "integer*8")
               DW_AT_data_location (DW_OP_push_object_address, DW_OP_deref)
               DW_AT_artificial    (true)
```

Fortran 前端可能需要生成一个蹦床函数（trampoline function）来调用在不同编译单元中定义的函数。在这种情况下，前端可以发射以下描述符，`targetFuncName` 字段为蹦床调用的函数名：

```llvm
!DISubprogram(name: "sub1_.t0p", linkageName: "sub1_.t0p", scope: !4, file: !4,
              type: !5, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition,
              unit: !7, retainedNodes: !24, targetFuncName: "sub1_")
```

产生的 DWARF 标签为：

```
DW_TAG_subprogram
  ...
  DW_AT_linkage_name  ("sub1_.t0p")
  DW_AT_name  ("sub1_.t0p")
  DW_AT_trampoline    ("sub1_")
```

---

## Objective-C 属性调试信息扩展

### 简介

Objective-C 提供了一种更简单的方式，通过声明属性（declared properties）来声明和定义访问器方法。调试器允许开发者检查 Objective-C 接口及其实例变量和类变量，但调试器不了解 Objective-C 接口中定义的属性。调试器使用编译器以 DWARF 格式生成的信息，但 DWARF 格式不支持对 Objective-C 属性的编码。本提案描述了对 DWARF 的扩展以编码 Objective-C 属性，供调试器使用。

### 提案

Objective-C 属性独立于类成员而存在。属性可以仅由"setter"和"getter"选择器定义，并在每次访问时重新计算；也可以只是对某个已声明 ivar 的直接访问；还可以由编译器为其"自动合成"一个 ivar，这种情况下可以在用户代码中用标准 C 解引用语法和属性"点"语法直接引用该属性，但在 `@interface` 声明中没有对应此 ivar 的条目。

为了便于调试，将在类的 `DW_TAG_structure_type` 定义中添加一个新的 DWARF TAG 来保存给定属性的描述，以及一组提供该描述的 DWARF 属性。属性标签还将包含属性的名称和声明类型。

如果存在相关的 ivar，在该 ivar 的 `DW_TAG_member` DIE 中还会有一个 DWARF 属性，指回该属性的属性 TAG。在编译器直接合成 ivar 的情况下，编译器应为该 ivar 生成一个 `DW_TAG_member`（设置 `DW_AT_artificial` 为 1），其名称为在代码中直接访问此 ivar 所用的名称，且属性属性指回它所支持的属性。

以下示例用于说明：

```objc
@interface I1 {
  int n2;
}

@property int p1;
@property int p2;
@end

@implementation I1
@synthesize p1;
@synthesize p2 = n2;
@end
```

产生以下 DWARF（伪 dwarfdump 输出）：

```
0x00000100:  TAG_structure_type [7] *
               AT_APPLE_runtime_class( 0x10 )
               AT_name( "I1" )
               AT_decl_file( "Objc_Property.m" )
               AT_decl_line( 3 )

0x00000110    TAG_APPLE_property
                AT_name ( "p1" )
                AT_type ( {0x00000150} ( int ) )

0x00000120:   TAG_APPLE_property
                AT_name ( "p2" )
                AT_type ( {0x00000150} ( int ) )

0x00000130:   TAG_member [8]
                AT_name( "_p1" )
                AT_APPLE_property ( {0x00000110} "p1" )
                AT_type( {0x00000150} ( int ) )
                AT_artificial ( 0x1 )

0x00000140:    TAG_member [8]
                 AT_name( "n2" )
                 AT_APPLE_property ( {0x00000120} "p2" )
                 AT_type( {0x00000150} ( int ) )
```

注意：当前惯例是，自动合成属性的 ivar 名称是在属性名前加下划线，但我们实际上不需要了解这个约定，因为 ivar 名称是直接给出的。

开发者可以用属性特性（attributes）装饰属性，这些特性使用 `DW_AT_APPLE_property_attribute` 编码。setter 和 getter 方法名称使用 `DW_AT_APPLE_property_setter` 和 `DW_AT_APPLE_property_getter` 属性附加到属性上。

### 新 DWARF 标签

| TAG | 值 |
|-----|-----|
| `DW_TAG_APPLE_property` | `0x4200` |

### 新 DWARF 属性

| 属性 | 值 | 类 |
|------|-----|-----|
| `DW_AT_APPLE_property` | `0x3fed` | Reference |
| `DW_AT_APPLE_property_getter` | `0x3fe9` | String |
| `DW_AT_APPLE_property_setter` | `0x3fea` | String |
| `DW_AT_APPLE_property_attribute` | `0x3feb` | Constant |

### 新 DWARF 常量

| 名称 | 值 |
|------|-----|
| `DW_APPLE_PROPERTY_readonly` | `0x01` |
| `DW_APPLE_PROPERTY_getter` | `0x02` |
| `DW_APPLE_PROPERTY_assign` | `0x04` |
| `DW_APPLE_PROPERTY_readwrite` | `0x08` |
| `DW_APPLE_PROPERTY_retain` | `0x10` |
| `DW_APPLE_PROPERTY_copy` | `0x20` |
| `DW_APPLE_PROPERTY_nonatomic` | `0x40` |
| `DW_APPLE_PROPERTY_setter` | `0x80` |
| `DW_APPLE_PROPERTY_atomic` | `0x100` |
| `DW_APPLE_PROPERTY_weak` | `0x200` |
| `DW_APPLE_PROPERTY_strong` | `0x400` |
| `DW_APPLE_PROPERTY_unsafe_unretained` | `0x800` |
| `DW_APPLE_PROPERTY_nullability` | `0x1000` |
| `DW_APPLE_PROPERTY_null_resettable` | `0x2000` |
| `DW_APPLE_PROPERTY_class` | `0x4000` |

---

## 名称加速表（Name Accelerator Tables）

### 简介

`.debug_pubnames` 和 `.debug_pubtypes` 格式并不是调试器所需要的。节名中的"pub"表明表中的条目仅为公开可见的名称。这意味着静态或隐藏函数不会出现在 `.debug_pubnames` 中，静态变量或私有类变量不在 `.debug_pubtypes` 中。不同编译器向这些表中添加的内容各不相同，因此我们无法依赖 gcc、icc 或 clang 之间的内容一致性。

用户给出的典型查询往往与这些表的内容不匹配。例如，DWARF 规范指出，对于 C++ 结构、类或联合的函数成员或静态数据成员的名称，`.debug_pubnames` 节中的名称不是引用调试信息条目的 `DW_AT_name` 属性给出的简单名称，而是数据或函数成员的完全限定名。所以这些表中对于复杂 C++ 条目的唯一名称是完全限定名，而调试器用户倾向于输入 `"c"`、`"b::c"` 或 `"a::b::c"` 而不是 `"a::b::c(int,const Foo&) const"` 这样的搜索字符串。因此，名称表中的条目必须被解码以适当地拆分，还必须向表中手动添加其他名称才能使其作为调试器使用的名称查找表有效。

所有调试器目前都忽略 `.debug_pubnames` 表，因为其内容不一致且仅含公开名称使其成为目标文件中的浪费空间。这些表写到磁盘时没有任何排序，导致每个调试器都需要自行解析和排序。这些表还在表本身中包含字符串值的内联副本，使表在磁盘上比必要的大得多，对于大型 C++ 程序尤为如此。

能否通过向此表添加所有需要的名称来修复这些节？不能，因为那不是表定义要包含的内容，我们无法区分旧的差表和新的好表。最好的方案是创建自己的重命名节，包含所有需要的数据。

这些表对于像 LLDB 这样的调试器也是不够的。LLDB 使用 clang 进行表达式解析，LLDB 充当 PCH。LLDB 经常被要求查找类型 "foo"、命名空间 "bar" 或列出命名空间 "baz" 中的条目。命名空间不包含在 pubnames 或 pubtypes 表中。由于 clang 在解析表达式时会提出大量问题，我们需要非常快速地查找名称。拥有针对快速查找优化的新加速表将极大地改善这种调试体验。

我们希望生成可以从磁盘映射到内存的名称查找表，并直接使用，几乎不需要或不需要前期解析。名称加速表被设计来解决这些问题，需要：

1. 具有可以从磁盘映射到内存并直接使用的格式
2. 查找非常快速
3. 可扩展的表格式，允许多个生产者创建这些表
4. 开箱即用地包含典型查找所需的所有名称
5. 表内容的严格规则

表大小很重要，加速表格式应允许从公共字符串表复用字符串，以避免名称字符串重复。我们还希望确保表在映射到内存后可以直接使用，只需最少的头部解析。

### 哈希表

#### 标准哈希表

典型哈希表有一个头部、若干桶，每个桶指向桶内容：

```
.------------.
|  HEADER    |
|------------|
|  BUCKETS   |
|------------|
|  DATA      |
`------------'
```

BUCKETS 是每个哈希到 DATA 的偏移量数组：

```
.------------.
| 0x00001000 | BUCKETS[0]
| 0x00002000 | BUCKETS[1]
| 0x00002200 | BUCKETS[2]
| 0x000034f0 | BUCKETS[3]
|            | ...
| 0xXXXXXXXX | BUCKETS[n_buckets]
'------------'
```

对于上面的 `bucket[3]`，我们有一个指向 `0x000034f0` 的偏移量，指向该桶的条目链，每个桶必须包含 next 指针、完整 32 位哈希值、字符串本身和当前字符串值的数据。

```
            .------------.
0x000034f0: | 0x00003500 | next pointer
            | 0x12345678 | 32-bit hash
            | "erase"    | string value
            | data[n]    | HashData for this bucket
            |------------|
0x00003500: | 0x00003550 | next pointer
            | 0x29273623 | 32-bit hash
            | "dump"     | string value
            | data[n]    | HashData for this bucket
            |------------|
0x00003550: | 0x00000000 | next pointer
            | 0x82638293 | 32-bit hash
            | "main"     | string value
            | data[n]    | HashData for this bucket
            `------------'
```

标准哈希表对调试器的问题在于需要针对**负查找**情况（查找的符号不存在）进行优化。搜索时需要读取 next 指针再读取哈希值进行比较，每次都在内存中跳过许多字节并访问新页面。

#### 名称哈希表

为解决上述问题，名称哈希表采用不同的结构：头部、桶、所有唯一 32 位哈希值的数组，以及每个哈希值的数据偏移量数组，然后是所有哈希值的数据：

```
.-------------.
|  HEADER     |
|-------------|
|  BUCKETS    |
|-------------|
|  HASHES     |
|-------------|
|  OFFSETS    |
|-------------|
|  DATA       |
`-------------'
```

BUCKETS 中的值是 HASHES 数组的索引。通过将所有完整 32 位哈希值在内存中连续存放，我们可以在触及尽可能少的内存的情况下高效检查匹配。大多数情况下，检查 32 位哈希值就是查找所到达的最远处。

表的布局：

```
.-------------------------.
|  HEADER.magic           | uint32_t
|  HEADER.version         | uint16_t
|  HEADER.hash_function   | uint16_t
|  HEADER.bucket_count    | uint32_t
|  HEADER.hashes_count    | uint32_t
|  HEADER.header_data_len | uint32_t
|  HEADER_DATA            | HeaderData
|-------------------------|
|  BUCKETS                | uint32_t[n_buckets]
|-------------------------|
|  HASHES                 | uint32_t[n_hashes]
|-------------------------|
|  OFFSETS                | uint32_t[n_hashes]
|-------------------------|
|  ALL HASH DATA          |
`-------------------------'
```

这种名称哈希表比标准哈希表更高效地组织数据：查找 "printf" 时，计算其 32 位哈希值，找到对应桶的索引，在 HASHES 数组中连续比较 32 位哈希值，失败时只需访问 BUCKETS[3] 的内存和若干连续的 32 位哈希值，保持最少的处理器数据缓存行访问。

这些查找表使用的字符串哈希是 Daniel J. Bernstein 哈希，也用于 ELF 的 `GNU_HASH` 节，对程序中各类名称具有极少哈希冲突。空桶用无效哈希索引 `UINT32_MAX` 表示。

### 详细信息

#### 头部布局

```c
struct Header {
  uint32_t   magic;           // 'HASH' 魔数，用于端序检测
  uint16_t   version;         // 版本号
  uint16_t   hash_function;   // 使用的哈希函数枚举
  uint32_t   bucket_count;    // 哈希表中桶的数量
  uint32_t   hashes_count;    // 唯一哈希值和哈希数据偏移量的总数
  uint32_t   header_data_len; // 到达哈希索引（桶）的字节跳过量（HeaderData 字段长度）
  HeaderData header_data;     // 实现特定的头部数据
};
```

哈希函数枚举：

```c
enum HashFunctionType {
  eHashFunctionDJB = 0u, // Daniel J Bernstein 哈希函数
};
```

#### 固定查找

```c
struct FixedTable {
  uint32_t buckets[Header.bucket_count];  // 哈希索引数组，指向 hashes[] 数组
  uint32_t hashes [Header.hashes_count];  // 整个表中所有名称的唯一 32 位哈希
  uint32_t offsets[Header.hashes_count];  // 对应 hashes[] 每项的数据偏移量
};
```

#### 原子类型（Atom Types）

`HeaderData` 定义了每个 `HashData` 块的内容：

```c
enum AtomType {
  eAtomTypeNULL       = 0u,  // 终止原子，指定原子列表结束
  eAtomTypeDIEOffset  = 1u,  // DIE 偏移量，检查 form 以了解编码
  eAtomTypeCUOffset   = 2u,  // 包含所讨论条目的编译单元头的 DIE 偏移量
  eAtomTypeTag        = 3u,  // DW_TAG_xxx 值，应编码为 DW_FORM_data1 或 DW_FORM_data2
  eAtomTypeNameFlags  = 4u,  // 来自 NameFlags 枚举的标志
  eAtomTypeTypeFlags  = 5u,  // 来自 TypeFlags 枚举的标志
};

struct Atom {
  uint16_t type;  // AtomType 枚举值
  uint16_t form;  // DWARF DW_FORM_XXX 定义
};

struct HeaderData {
  uint32_t die_offset_base;
  uint32_t atom_count;
  Atoms    atoms[atom_count];
};
```

对于 `.apple_names`（所有函数和全局变量）、`.apple_types`（所有定义类型的名称）和 `.apple_namespaces`（所有命名空间），当前将原子数组设置为：

```
HeaderData.atom_count = 1;
HeaderData.atoms[0].type = eAtomTypeDIEOffset;
HeaderData.atoms[0].form = DW_FORM_data4;
```

DWARF 表的 `KeyType` 是到 `.debug_str` 表的 32 位字符串表偏移量，帮助在所有 DWARF 节之间复用字符串并减小哈希表大小。

哈希数据块结构：

```
uint32_t str_offset      // 若为零则桶内容结束
uint32_t hash_data_count
HashData[hash_data_count]
```

99.9% 的哈希数据块包含单个条目（无 32 位哈希冲突）。真实 C++ 二进制测试显示每 100,000 个名称条目约有 1 个 32 位哈希冲突。

### 表内容规范

**`.apple_names`** 节应包含每个 `DW_TAG_label`、`DW_TAG_inlined_subroutine` 或具有地址属性（`DW_AT_low_pc`、`DW_AT_high_pc`、`DW_AT_ranges` 或 `DW_AT_entry_pc`）的 `DW_TAG_subprogram` 的 DWARF DIE 条目，以及在位置中有 `DW_OP_addr` 的 `DW_TAG_variable` DIE（全局和静态变量）。所有函数应同时发射完整名称和基名，C/C++ 中完整名称是修饰名（若可用），通常在 `DW_AT_MIPS_linkage_name` 属性中，`DW_AT_name` 包含函数基名。

**`.apple_types`** 节应包含标签为以下之一的每个 DWARF DIE 的条目：`DW_TAG_array_type`、`DW_TAG_class_type`、`DW_TAG_enumeration_type`、`DW_TAG_pointer_type`、`DW_TAG_reference_type`、`DW_TAG_string_type`、`DW_TAG_structure_type`、`DW_TAG_subroutine_type`、`DW_TAG_typedef`、`DW_TAG_union_type`、`DW_TAG_ptr_to_member_type`、`DW_TAG_set_type`、`DW_TAG_subrange_type`、`DW_TAG_base_type`、`DW_TAG_const_type`、`DW_TAG_immutable_type`、`DW_TAG_file_type`、`DW_TAG_namelist`、`DW_TAG_packed_type`、`DW_TAG_volatile_type`、`DW_TAG_restrict_type`、`DW_TAG_atomic_type`、`DW_TAG_interface_type`、`DW_TAG_unspecified_type`、`DW_TAG_shared_type`。仅包含具有 `DW_AT_name` 属性且不是前向声明的条目（即 `DW_AT_declaration` 属性值为非零）。

**`.apple_namespaces`** 节应包含所有 `DW_TAG_namespace` DIE。遇到没有名称的命名空间时为匿名命名空间，名称应输出为 `"(anonymous namespace)"`，与标准 C++ 库中 `abi::cxa_demangle()` 的输出匹配。

### 语言扩展和文件格式变更

**Objective-C 扩展（`.apple_objc`）**：应包含 Objective-C 类的所有 `DW_TAG_subprogram` DIE。如果 Objective-C 类有类别（category），则对类名（不含类别）和类名（含类别）各添加一个条目。在 `.apple_names` 节中，Objective-C 函数的完整名称是带括号的整个函数名，基名是仅选择器部分。

**Mach-O 变更**：对于 mach-o 文件，节名如下：
- `.apple_names` → `__apple_names`
- `.apple_types` → `__apple_types`
- `.apple_namespaces` → `__apple_namespac`（16 字符限制）
- `.apple_objc` → `__apple_objc`

---

## CodeView 调试信息格式

LLVM 支持发射 CodeView（Microsoft 调试信息格式），本节描述该支持的设计和实现。

### 格式背景

CodeView 格式明显围绕 C++ 调试设计，在 C++ 中，大多数调试信息倾向于是类型信息。因此 CodeView 的首要设计约束是将类型信息与其他"符号"信息分离，使类型信息可以跨翻译单元高效合并。类型信息和符号信息通常都以记录序列的形式存储，每条记录以 16 位记录大小和 16 位记录类型开头。

类型信息通常存储在目标文件的 `.debug$T` 节中，所有其他调试信息（如行信息、字符串表、符号信息和内联信息）存储在一个或多个 `.debug$S` 节中。每个目标文件只能有一个 `.debug$T` 节，因为所有其他调试信息都引用它。如果在编译时使用了 PDB（由 `/Zi` MSVC 选项启用），`.debug$T` 节将只包含一个指向 PDB 的 `LF_TYPESERVER2` 记录。

类型记录通过其索引引用，索引是流中该记录之前的记录数加上 `0x1000`。许多常见基本类型（如基本整数类型及其无限定指针）使用小于 `0x1000` 的类型索引表示，内置于 CodeView 消费者中，不需要类型记录。

每个类型记录只能包含小于其自身类型索引的类型索引，确保类型流引用图是非循环的。虽然源级类型图可能通过指针类型包含循环（如链表结构体），但通过始终引用用户定义记录类型的前向声明记录，这些循环从类型流中被移除。

### 使用 CodeView

以下是改善 LLVM CodeView 支持的开发者常用任务指南，主要围绕使用 `llvm-readobj` 中嵌入的 CodeView 转储器：

**测试 MSVC 的输出：**

```bash
$ cl -c -Z7 foo.cpp  # 使用 /Z7 将类型保留在目标文件中
$ llvm-readobj --codeview foo.obj
```

**从 Clang 获取 LLVM IR 调试信息：**

```bash
$ clang -g -gcodeview --target=x86_64-windows-msvc foo.cpp -S -emit-llvm
```

**从 LLVM IR 元数据生成并转储 CodeView：**

```bash
$ llc foo.ll -filetype=obj -o foo.obj
$ llvm-readobj --codeview foo.obj > foo.txt
```

在 lit 测试用例中使用此模式，并用 FileCheck 检查 `llvm-readobj` 的输出。

改善 LLVM CodeView 支持的过程是：找到有趣的类型记录，构造使 MSVC 发射这些记录的 C++ 测试用例，转储记录，理解它们，然后在 LLVM 后端中生成等价的记录。

---

*© Copyright 2003-2026, LLVM Project. 最后更新于 2026-04-22。*
