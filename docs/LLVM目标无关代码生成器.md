# LLVM 目标无关代码生成器

> 原文来源：[LLVM Target-Independent Code Generator](https://llvm.org/docs/CodeGenerator.html)
> 翻译日期：2026-04-21

---

## 目录

1. [简介](#1-简介)
   - [代码生成器所需组件](#11-代码生成器所需组件)
   - [代码生成器的高层设计](#12-代码生成器的高层设计)
   - [使用 TableGen 进行目标描述](#13-使用-tablegen-进行目标描述)
2. [目标描述类](#2-目标描述类)
   - [TargetMachine 类](#21-targetmachine-类)
   - [DataLayout 类](#22-datalayout-类)
   - [TargetLowering 类](#23-targetlowering-类)
   - [TargetRegisterInfo 类](#24-targetregisterinfo-类)
   - [TargetInstrInfo 类](#25-targetinstrinfo-类)
   - [TargetFrameLowering 类](#26-targetframelowering-类)
   - [TargetSubtarget 类](#27-targetsubtarget-类)
   - [TargetJITInfo 类](#28-targetjitinfo-类)
3. [机器码描述类](#3-机器码描述类)
   - [MachineInstr 类](#31-machineinstr-类)
   - [MachineBasicBlock 类](#32-machinebasicblock-类)
   - [MachineFunction 类](#33-machinefunction-类)
   - [MachineInstr 指令包（Bundles）](#34-machineinstr-指令包bundles)
4. [MC 层](#4-mc-层)
   - [MCStreamer API](#41-mcstreamer-api)
   - [MCContext 类](#42-mccontext-类)
   - [MCSymbol 类](#43-mcsymbol-类)
   - [MCSection 类](#44-mcsection-类)
   - [MCInst 类](#45-mcinst-类)
   - [目标文件格式](#46-目标文件格式)
5. [目标无关代码生成算法](#5-目标无关代码生成算法)
   - [指令选择](#51-指令选择)
   - [基于 SSA 的机器码优化](#52-基于-ssa-的机器码优化)
   - [活跃区间](#53-活跃区间)
   - [寄存器分配](#54-寄存器分配)
   - [序言/尾声代码插入](#55-序言尾声代码插入)
   - [紧凑展开信息（Compact Unwind）](#56-紧凑展开信息compact-unwind)
   - [后期机器码优化](#57-后期机器码优化)
   - [代码发射](#58-代码发射)
   - [VLIW 打包器](#59-vliw-打包器)
6. [实现原生汇编器](#6-实现原生汇编器)
   - [指令解析](#61-指令解析)
   - [指令别名处理](#62-指令别名处理)
   - [指令匹配](#63-指令匹配)
7. [目标相关实现注意事项](#7-目标相关实现注意事项)
   - [尾调用优化](#71-尾调用优化)
   - [兄弟调用优化](#72-兄弟调用优化)
   - [X86 后端](#73-x86-后端)
   - [PowerPC 后端](#74-powerpc-后端)
   - [NVPTX 后端](#75-nvptx-后端)
   - [eBPF 后端](#76-ebpf-后端)
   - [AMDGPU 后端](#77-amdgpu-后端)
   - [轻量级故障隔离（LFI）子架构](#78-轻量级故障隔离lfi子架构)

---

## 1. 简介

LLVM 目标无关代码生成器由六个核心组件构成，负责将 LLVM 中间表示（IR）翻译为指定目标架构的机器码：

1. **抽象目标描述接口** — 描述目标架构特性的抽象类层次
2. **代码表示类** — 在代码生成过程中表示机器码的类
3. **MC 层** — 用于处理目标文件的底层机器码抽象
4. **目标无关算法** — 可跨目标复用的代码生成算法（如寄存器分配）
5. **具体目标实现** — 针对特定架构的目标相关代码
6. **JIT 组件** — 即时编译支持

### 1.1 代码生成器所需组件

`TargetMachine` 和 `DataLayout` 是将一个后端集成到 LLVM 系统中**唯一必须**定义的两个接口。这一设计允许极大的灵活性——后端可以完全绕过标准组件，构建全定制的代码生成流水线。

> 在 LLVM 3.1 中被移除的 C 后端就是这种极端案例的代表：它仅实现了这两个接口，却实现了完全非传统的代码生成方式。

### 1.2 代码生成器的高层设计

代码生成过程遵循以下七个顺序阶段：

| 阶段 | 说明 |
|------|------|
| **1. 指令选择** | 将 LLVM IR 转换为以虚拟/物理寄存器表示的目标指令 DAG（SSA 形式）|
| **2. 调度与生成** | 对指令排序并将其发射为 `MachineInstr` |
| **3. 基于 SSA 的机器码优化** | 可选优化，如模调度（modulo-scheduling）|
| **4. 寄存器分配** | 将虚拟寄存器映射为物理寄存器，并引入溢出代码 |
| **5. 序言/尾声代码插入** | 添加函数入口/出口代码，消除抽象栈引用 |
| **6. 后期机器码优化** | 窥孔优化等最终优化 |
| **7. 代码发射** | 输出最终汇编文件或机器码目标文件 |

该架构的设计优先考虑"高效编译（对 JIT 环境至关重要）和激进优化"，允许在各阶段使用不同复杂度的组件。

### 1.3 使用 TableGen 进行目标描述

目标描述需要大量架构细节，且这些细节在相似指令之间往往重复。LLVM 采用 TableGen 来减轻这一负担，允许使用领域特定的抽象。

官方文档指出：

> "它使 LLVM 的移植更加容易，因为它减少了必须编写的 C++ 代码量，以及需要理解的代码生成器表面积。"

---

## 2. 目标描述类

目标描述类位于 `include/llvm/Target`，以目标机器无关的方式提供目标的抽象描述，独立于任何特定的客户端。这些类负责捕获目标的**抽象属性**（如其支持的指令集和寄存器文件），而不涉及具体的代码生成算法。

除 `DataLayout` 类外，所有目标描述类都被设计为由具体目标实现子类化，并通过 `TargetMachine` 类的访问器方法来获取具体实现。

### 2.1 `TargetMachine` 类

`TargetMachine` 类提供一组虚方法，通过 `get*Info` 系列方法（如 `getInstrInfo`、`getRegisterInfo`、`getFrameInfo` 等）来访问各种目标描述类的目标相关实现。

该类被设计为由具体目标实现（如 `X86TargetMachine`）所继承和特化，子类需要实现各虚方法。唯一必须实现的目标描述类是 `DataLayout`，但若需使用代码生成器的各组件，则应同时实现其余接口。

### 2.2 `DataLayout` 类

`DataLayout` 类是唯一**必须**实现的目标描述类，也是唯一**不可扩展**的类（不能从它派生新类）。

`DataLayout` 描述以下信息：

- 目标架构如何在内存中布局结构体
- 各种数据类型的对齐要求
- 目标架构中指针的大小
- 目标架构是小端序（little-endian）还是大端序（big-endian）

### 2.3 `TargetLowering` 类

`TargetLowering` 类主要用于基于 SelectionDAG 的指令选择器，描述如何将 LLVM 代码降低（lower）为 SelectionDAG 操作。其主要职责包括：

- 为各种 `ValueType` 指定初始寄存器类
- 指明目标机器原生支持哪些操作
- 描述 `setcc` 操作的返回类型
- 指定移位操作数的类型
- 描述各种高层特性（如将常数除法转换为乘法序列是否合算）

### 2.4 `TargetRegisterInfo` 类

`TargetRegisterInfo` 类用于描述目标架构的寄存器文件及寄存器之间的交互关系。

**寄存器的表示方式：**

- 寄存器在代码生成器中用无符号整数表示
- 物理寄存器（实际存在于目标描述中的寄存器）是较小的唯一整数
- 虚拟寄存器通常是较大的整数
- 寄存器 `#0` 保留为标志值

处理器中的每个寄存器都有一个关联的 `TargetRegisterDesc` 条目，提供寄存器的文本名称（用于汇编输出和调试转储）以及一组别名（用于指示一个寄存器是否与另一个重叠）。

`TargetRegisterInfo` 还暴露一组处理器特定的**寄存器类**（`TargetRegisterClass` 的实例）。每个寄存器类包含具有相同属性的寄存器集合（例如，所有 32 位整数寄存器）。指令选择器创建的每个 SSA 虚拟寄存器都关联一个寄存器类；寄存器分配器运行时，用该集合中的某个物理寄存器替换虚拟寄存器。

这些类的目标相关实现由寄存器文件的 TableGen 描述自动生成。

### 2.5 `TargetInstrInfo` 类

`TargetInstrInfo` 类用于描述目标架构支持的机器指令。描述内容包括：

- 操作码的助记符
- 操作数的数量
- 隐式使用和定义的寄存器列表
- 指令是否具有某些目标无关属性（访问内存、可交换等）
- 目标特定标志

### 2.6 `TargetFrameLowering` 类

`TargetFrameLowering` 类用于提供目标架构栈帧布局信息，包含：

- 栈的增长方向
- 每个函数入口处已知的栈对齐
- 局部区域的偏移量（即函数入口时栈指针到第一个可存储函数数据的位置的偏移）

### 2.7 `TargetSubtarget` 类

`TargetSubtarget` 类用于提供被目标的特定芯片组的信息。子目标（sub-target）向代码生成器告知：

- 哪些指令被支持
- 指令延迟（instruction latencies）
- 指令执行行程（instruction execution itinerary）：使用哪些处理单元、以何种顺序、持续多长时间

### 2.8 `TargetJITInfo` 类

`TargetJITInfo` 类暴露一个抽象接口，供即时（JIT）代码生成器执行目标相关活动，例如发射存根（stub）。如果 `TargetMachine` 支持 JIT 代码生成，则应通过 `getJITInfo` 方法提供该对象。

---

## 3. 机器码描述类

在高层上，LLVM 代码通过三个关键类转换为机器相关表示：

```
MachineFunction
  └── MachineBasicBlock（列表）
        └── MachineInstr（列表）
```

这些类定义于 `include/llvm/CodeGen`，并保持完全目标无关性，以最抽象的形式表示指令。

### 3.1 `MachineInstr` 类

`MachineInstr` 类以极为抽象的方式表示目标机器指令，仅追踪**操作码编号**和**一组操作数**。操作码是一个简单的无符号整数，其语义对每个后端而言是特定的，派生自目标的 `*InstrInfo.td` 文件。

机器指令操作数可以是以下几种类型：寄存器引用、常量整数或基本块引用。每个操作数应标记为定义（def）或使用（use），但**只有寄存器允许作为定义**。

#### 操作数排序惯例

按照惯例，LLVM 代码生成器对指令操作数的排序方式是：**所有寄存器定义在前，寄存器使用在后**。

例如，SPARC 的加法指令 `add %i1, %i2, %i3` 在内部存储为 `%i3, %i1, %i2`（目标寄存器在前）。这种排序有两个好处：
1. 调试打印器可以将指令显示为 `%r3 = add %i1, %i2`，更直观
2. 简化了"唯一定义为第一个操作数"的指令的构建

#### 使用 `MachineInstrBuilder` 函数

`include/llvm/CodeGen/MachineInstrBuilder.h` 中的 `BuildMI` 函数簇用于构建任意机器指令：

```cpp
const TargetInstrInfo &TII = ...
MachineBasicBlock &MBB = ...
DebugLoc DL;

// 构建一条带目标寄存器的指令
MachineInstr *MI = BuildMI(MBB, DL, TII.get(X86::MOV32ri), DestReg).addImm(42);

// 在特定位置插入指令
MachineBasicBlock::iterator MBBI = ...
BuildMI(MBB, MBBI, DL, TII.get(X86::MOV32ri), DestReg).addImm(42);

// 无目标寄存器的比较指令
MI = BuildMI(MBB, DL, TII.get(X86::CMP32ri8)).addReg(Reg).addImm(42);

// 无操作数指令
MI = BuildMI(MBB, DL, TII.get(X86::SAHF));

// 跳转指令
BuildMI(MBB, DL, TII.get(X86::JNE)).addMBB(&MBB);
```

对于第一个操作数以外的定义操作数，需要显式标记：

```cpp
MI.addReg(Reg, RegState::Define);
```

#### 固定（预分配）寄存器

代码生成器必须处理固定寄存器——即由于指令限制或调用惯例，值必须驻留于其中的物理寄存器。例如，X86 除法指令要求使用特定寄存器。指令选择器会根据需要发射将虚拟寄存器复制到物理寄存器（或从物理寄存器复制）的代码。

**示例：**

LLVM IR：
```llvm
define i32 @test(i32 %X, i32 %Y) {
  %Z = sdiv i32 %X, %Y
  ret i32 %Z
}
```

X86 指令选择器可能产生（寄存器分配前）：
```
%EAX = mov %reg1024
%reg1027 = sar %reg1024, 31
%EDX = mov %reg1027
idiv %reg1025
%reg1026 = mov %EAX

%EAX = mov %reg1026
ret
```

寄存器分配后（合并冗余复制）：
```
mov %EAX, %EDX
sar %EDX, 31
idiv %ECX
ret
```

物理寄存器应尽量保持短生命周期。在寄存器分配前，**所有物理寄存器在基本块的入口和出口处均被假定为死亡**。需要跨基本块持久化的值必须存放在虚拟寄存器中。

#### 调用破坏寄存器（Call-Clobbered Registers）

`call` 等机器指令会破坏（clobber）大量物理寄存器。为了避免为每个被破坏的寄存器添加 `<def,dead>` 操作数，可以使用 `MO_RegisterMask` 操作数代替。寄存器掩码保存一个位掩码，标记被保留的寄存器，其余寄存器均被视为被破坏。

#### SSA 形式的机器码

`MachineInstr` 最初以 SSA 形式被选择，并在寄存器分配发生之前一直保持 SSA 形式。由于 LLVM IR 本身已是 SSA 形式，这一过程相对简单：PHI 节点变为机器 PHI 节点，虚拟寄存器保持单次定义的性质。寄存器分配完成后，代码中不再存在虚拟寄存器，机器码也**不再是 SSA 形式**。

### 3.2 `MachineBasicBlock` 类

`MachineBasicBlock` 类包含一组机器指令（`MachineInstr` 实例）的列表。它大致对应于输入到指令选择器的 LLVM 基本块，但两者之间是一对多的映射关系——一个 LLVM 基本块可能映射到多个机器基本块。该类提供 `getBasicBlock()` 方法，返回其对应的 LLVM 基本块。

### 3.3 `MachineFunction` 类

`MachineFunction` 类包含一组机器基本块（`MachineBasicBlock` 实例）的列表，与输入到指令选择器的 LLVM 函数一一对应。

除机器基本块外，它还包含：

| 子组件 | 说明 |
|--------|------|
| `MachineConstantPool` | 常量池 |
| `MachineFrameInfo` | 栈帧信息 |
| `MachineFunctionInfo` | 目标特定函数信息 |
| `MachineRegisterInfo` | 寄存器信息 |

详细内容参见 `include/llvm/CodeGen/MachineFunction.h`。

### 3.4 `MachineInstr` 指令包（Bundles）

LLVM 代码生成器可以将指令序列建模为 `MachineInstr` 包（bundle），用于表示：

- **VLIW 指令组/包**：包含任意数量并行执行指令
- **不可分割的顺序指令序列**：如 ARM Thumb2 的 IT 块（潜在存在数据依赖）

**概念模型：** 一个包（bundle）是一条顶层 MI，其中嵌套了多条 MI。所有 MI（包括顶层和嵌套的）都以顺序列表存储；被打包的 MI 用 `InsideBundle` 标志标记。顶层的特殊 `BUNDLE` 操作码 MI 表示包的开始。

**使用规范：**

- Pass 应将 MI 包视为单个整体来操作
- `MachineBasicBlock` 的迭代器会跳过被打包的 MI，以强制"包即单元"语义
- 若需遍历所有 MI（包括嵌套的），使用 `instr_iterator`
- 顶层 `BUNDLE` 指令必须拥有正确的寄存器 `MachineOperand` 集合，代表所有被打包 MI 的累积输入和输出

**VLIW 架构的建议：** 打包/捆绑操作通常应在寄存器分配超级 Pass（super-pass）中完成，在虚拟寄存器被重写为物理寄存器之后进行，以避免在 `BUNDLE` 指令上重复虚拟寄存器列表。

---

## 4. MC 层

MC 层用于在**原始机器码级别**表示和处理代码，不包含"常量池"、"跳转表"、"全局变量"等高层信息。

在这个抽象层次上，LLVM 管理目标文件中的标签、机器指令和 section。MC 层有两个主要用途：
1. 代码生成器的最终阶段用它来输出 `.s` 或 `.o` 文件
2. `llvm-mc` 工具利用它实现独立的机器码汇编器和反汇编器

### 4.1 `MCStreamer` API

`MCStreamer` 是汇编器 API——一个以不同方式实现的抽象接口（生成 `.s` 文件、ELF `.o` 文件等），其 API 与汇编文件指令直接对应。

该类为每条指令提供一个方法，包括 `EmitLabel`、`EmitSymbolAttribute`、`switchSection`、`emitValue`（对应 `.byte`、`.word`）等，以及用于向流输出 `MCInst` 对象的 `EmitInstruction` 方法。

**主要客户端：**

- `llvm-mc` 独立汇编器：解析输入行并调用 `MCStreamer` 方法
- 代码生成器的代码发射阶段：将 LLVM IR 和 `Machine*` 构造降低到 MC 层

**两种主要实现：**

| 实现类 | 说明 |
|--------|------|
| `MCAsmStreamer` | 写 `.s` 文件，直接打印指令 |
| `MCObjectStreamer` | 实现完整汇编器逻辑，生成目标文件 |

**目标特定指令支持：** `MCStreamer` 持有一个 `MCTargetStreamer` 实例。每个目标定义一个继承自它的类，包含每条指令对应的方法。目标汇编流打印指令，目标对象流实现汇编器逻辑。

目标必须通过 `TargetRegistry::RegisterAsmStreamer` 和 `TargetRegistry::RegisterMCObjectStreamer` 注册回调，以分配相应的目标流。

### 4.2 `MCContext` 类

`MCContext` 类是 MC 层各种唯一化数据结构（包括符号、section 等）的**所有者**。它是创建符号和 section 的交互点。注意：`MCContext` **不可**被子类化。

### 4.3 `MCSymbol` 类

`MCSymbol` 类表示汇编文件中的**符号**（即标签）。存在两种有趣的符号类型：

1. **汇编器临时符号**：由汇编器使用和处理，在生成目标文件时丢弃。通常具有特定前缀（例如，MachO 中以 "L" 开头的标签是汇编器临时标签）
2. **普通符号**：保留在最终输出中

`MCContext` 创建并唯一化 `MCSymbol`，允许通过指针相等性比较来确定符号是否相同。但**指针不相等并不保证标签最终位于不同地址**：

```asm
foo:
bar:
  .byte 4
```

`foo` 和 `bar` 两个符号可以共享相同的地址。

### 4.4 `MCSection` 类

`MCSection` 类表示**目标文件特定的 section**。它被目标文件实现子类化（`MCSectionMachO`、`MCSectionCOFF`、`MCSectionELF`），由 `MCContext` 创建和唯一化。

`MCStreamer` 维护当前 section 的概念，通过 `SwitchToSection` 方法修改，对应汇编文件中的 `.section` 指令。

### 4.5 `MCInst` 类

`MCInst` 类是指令的**目标无关表示**，比 `MachineInstr` 简单得多，仅持有一个目标特定的操作码和一个 `MCOperand` 向量。

`MCOperand` 是三种情况的判别联合体（discriminated union）：

1. 简单立即数
2. 目标寄存器 ID
3. 符号表达式（如 "`Lfoo-Lbar+42`"），表示为 `MCExpr`

`MCInst` 是 MC 层表示机器指令的**通用货币**，被指令编码器、打印器、汇编解析器和反汇编器广泛使用。

### 4.6 目标文件格式

MC 层的目标文件写入器支持多种目标文件格式。由于各格式具有目标特定性，每个目标只支持部分格式子集。大多数目标支持 ELF；厂商特定格式通常只在相关目标上支持（MachO 在 Darwin 目标上；XCOFF 在 AIX 目标上）。

部分目标具有自定义格式（DirectX、SPIR-V、WebAssembly）。

**支持格式一览：**

| 格式 | 支持的目标架构 |
|------|---------------|
| COFF | AArch64, ARM, X86 |
| DXContainer | DirectX |
| ELF | AArch64, AMDGPU, ARM, AVR, BPF, CSKY, Hexagon, Lanai, LoongArch, M68k, MSP430, MIPS, PowerPC, RISCV, SPARC, SystemZ, VE, X86 |
| GOFF | SystemZ |
| MachO | AArch64, ARM, X86 |
| SPIR-V | SPIRV |
| WASM | WebAssembly |
| XCOFF | PowerPC |

---

## 5. 目标无关代码生成算法

### 5.1 指令选择

指令选择是将提交给代码生成器的 LLVM 代码转换为目标特定机器指令的过程。LLVM 采用基于 SelectionDAG 的方法，其中部分代码由目标描述文件（`*.td`）自动生成，但某些方面仍需自定义 C++ 代码。

#### SelectionDAG 简介

SelectionDAG 提供了一种适合自动指令选择技术的抽象，将代码表示为有向无环图（DAG），其中节点是包含操作码和操作数的 `SDNode` 实例。

**关键特性：**

- **节点结构**：每个节点定义一个操作，产生一个或多个值，表示为 `SDValue` 对（节点 + 结果索引）
- **值类型**：关联的 `MVT`（机器值类型）指示值的分类
- **控制流**：链边（类型 `MVT::Other`）对有副作用的操作（如 load/store）进行排序
- **链惯例**：令牌链输入占操作数 #0；链结果出现在最后
- **合法/非法 DAG**：合法 DAG 仅使用支持的操作和类型；非法性触发合法化阶段

SelectionDAG 有专用的 Entry 节点和 Root 节点，Entry 标记起点，Root 代表最后一个有副作用的操作。

#### SelectionDAG 指令选择流程

完整流程包含八个顺序步骤：

| 步骤 | 说明 |
|------|------|
| 1. 构建初始 DAG | 从 LLVM 代码简单转换为（非法的）SelectionDAG |
| 2. 优化 SelectionDAG | 简化和元指令识别 |
| 3. 合法化类型 | 消除不支持的值类型 |
| 4. 优化 SelectionDAG | 清理类型合法化引入的冗余 |
| 5. 合法化操作 | 移除不支持的操作 |
| 6. 优化 SelectionDAG | 消除操作合法化引入的低效 |
| 7. 选择指令 | 将 DAG 操作匹配到目标指令 |
| 8. 调度与生成 | 分配线性指令顺序并发射 `MachineFunction` |

**调试工具：**

- `-debug-only=isel`：详细输出
- `-view-dag-combine1-dags`：查看第一次 DAG 组合
- `-view-legalize-dags`：查看合法化后的 DAG
- `-view-isel-dags`：查看指令选择前的 DAG
- `-view-sched-dags`：查看调度后的 DAG

#### 初始 SelectionDAG 构建

`SelectionDAGBuilder` 类将 LLVM 输入朴素地展开为 SelectionDAG 形式，暴露底层目标细节。大多数展开是硬编码的（LLVM `add` 变为 `SDNode add`；`getelementptr` 展开为算术操作）。目标特定的钩子通过 `TargetLowering` 接口处理函数调用、返回值和可变参数。

#### SelectionDAG 类型合法化阶段（LegalizeTypes）

该阶段将 DAG 转换为仅使用原生支持类型。两种主要转换机制：

- **提升（Promotion）**：将小类型转换为更大类型
- **展开（Expansion）**：将大整数拆分为更小的对

对于向量：拆分直到出现合法类型，或通过添加元素来扩展。若拆分到达单元素标量且没有合法的向量类型，则进行标量化（scalarization）。

目标通过在 `TargetLowering` 构造函数中调用 `addRegisterClass()` 来声明支持的类型。

#### SelectionDAG 操作合法化阶段（Legalize）

该阶段将 DAG 转换为仅使用支持的操作。三种合法化策略：

| 策略 | 说明 |
|------|------|
| **展开（Expansion）** | 用替代操作序列开放编码 |
| **提升（Promotion）** | 使用支持该操作的更大类型 |
| **自定义（Custom）** | 目标特定实现钩子 |

通过在 `TargetLowering` 构造函数中调用 `setOperationAction()` 来声明不支持的操作及所选策略。

对于向量目标，`shufflevector` IR 指令处理应包括：向量选择（blend）、插入/提取子向量操作、以及广播（splat/broadcast）模式。

#### SelectionDAG 优化阶段：DAG 组合器

DAG 组合器在选择过程中多次运行，执行简化和模式识别。早期运行清理初始代码；后续运行消除合法化引入的低效。组合器特别擅长优化插入的符号扩展和零扩展。

#### SelectionDAG 选择阶段

模式匹配将合法的 SelectionDAG 转换为目标指令 DAG。TableGen 从 `.td` 文件中的包含语义信息的指令 pattern 自动生成匹配代码。

**PowerPC 乘加融合示例：**

```tablegen
def FMADDS : AForm_1<59, 29,
    (ops F4RC:$FRT, F4RC:$FRA, F4RC:$FRC, F4RC:$FRB),
    "fmadds $FRT, $FRA, $FRC, $FRB",
    [(set F4RC:$FRT, (fadd (fmul F4RC:$FRA, F4RC:$FRC), F4RC:$FRB))]>;
```

TableGen 模式匹配提供的能力：

- 编译期模式验证
- 任意操作数约束（如"13 位有符号扩展立即数"）
- 代数恒等式（识别交换律）
- 完整类型推导，无需显式声明
- 用于代码重用的模式片段（pattern fragments）
- 通过 `Pat` 类实现任意模式到指令的映射
- 通过 `ComplexPattern` 规范处理复杂操作数

**现有局限：** 多值节点定义、复杂寻址模式、标志推断、合法化器支持生成以及自定义合法化节点处理方面仍存在不足。

#### SelectionDAG 调度与生成阶段

选择完成后，调度器根据机器约束（最小化寄存器压力或覆盖指令延迟）分配指令顺序。排序完成后，DAG 转换为 `MachineInstr` 列表，SelectionDAG 随即销毁。

该阶段在逻辑上与指令选择分离，但在实现中紧密集成，仍在 SelectionDAG 上操作。

#### SelectionDAG 的未来方向

两个主要开发目标：

1. 可选的逐函数选择（function-at-a-time selection）
2. 从 `.td` 文件完全自动生成整个选择器

### 5.2 基于 SSA 的机器码优化

（待完善）

该阶段对寄存器分配前的 SSA 形式机器码执行可选优化，例如模调度（modulo-scheduling）。

### 5.3 活跃区间

活跃区间表示变量保持"活跃"（正在被使用）的范围。寄存器分配器使用这些信息来判断是否存在两个需要同一物理寄存器同时活跃的虚拟寄存器冲突，进而决定是否需要将值溢出到内存。

#### 活跃变量分析

该基础分析阶段识别两个关键集合：

- **立即死亡的寄存器**：在某条指令后立即死亡的寄存器（已计算但从未使用）
- **被杀死的寄存器**：被使用后即被杀死的寄存器（之后不再使用）

分析对虚拟寄存器利用 SSA 形式高效工作，对物理寄存器仅在单个基本块内追踪。由于物理寄存器在块边界处被假定为死亡（分配前），每个块的局部分析已经足够。

**预着色寄存器（pre-colored registers）的特殊处理：**

- 系统区分隐式定义和显式定义的寄存器
- 函数参数寄存器被标记为 live-in 值，带有虚拟定义指令
- 返回值被标记为 live-out

**PHI 节点的特殊处理：** 由于深度优先 CFG 遍历不能保证操作数在使用前已被定义，分析立即处理定义，但将使用处理推迟到后继块。当遇到来自当前块的 PHI 节点操作数时，变量在前驱链中被标记为活跃，直到达到定义指令。

#### 活跃区间分析

在活跃变量分析之后，系统对基本块和机器指令进行顺序编号。活跃区间本身在指令编号 [1, N] 上计算，表示为 [i, j) 形式的区间（其中 1 ≥ i ≥ j > N），表示变量保持活跃的范围。

### 5.4 寄存器分配

寄存器分配问题是将使用无限虚拟寄存器的程序映射到使用有限物理寄存器程序的过程。当物理寄存器不足时，多余的虚拟寄存器溢出到内存。

#### LLVM 中寄存器的表示

- 物理寄存器使用整数表示（通常为 1–1023），其架构特定含义定义在生成文件（如 `X86GenRegisterInfo.inc`）中
- 寄存器 0 保留为标志值

许多架构包含**别名寄存器**，共享同一物理位置。例如，X86 的 `EAX`、`AX`、`AL` 共享相同的 8 位。`MCRegAliasIterator` 类用于枚举别名物理寄存器。

物理寄存器按功能组织为**寄存器类**（Register Class），每个类中的寄存器可互换。每个虚拟寄存器映射到一个兼容的寄存器类；兼容性检查验证目标寄存器是否在虚拟寄存器的寄存器类中。

虚拟寄存器接收唯一的整数标识，通过 `MachineRegisterInfo::createVirtualRegister()` 创建。

#### 虚拟寄存器到物理寄存器的映射

两种映射策略：

**直接映射：** 灵活但需仔细实现。程序员指定 load/store 插入位置：
- `MachineOperand::setReg()` 用于分配
- `TargetInstrInfo::storeRegToStackSlot()` 用于存储
- `TargetInstrInfo::loadRegFromStackSlot()` 用于加载

**间接映射：** 通过 `VirtRegMap` 类简化流程：
- `VirtRegMap::assignVirt2Phys()` 将虚拟寄存器映射到物理寄存器
- `assignVirt2StackSlot()` 将虚拟寄存器映射到内存位置
- 注意：即使内存映射的虚拟寄存器也需要物理寄存器，用于 store 前/reload 后的临时位置

映射完成后，溢出器（spiller）对象插入 load 和 store 指令，尝试复用指令并避免冗余。

#### 处理双地址指令

大多数 LLVM 指令是三地址形式（一个定义，两个使用），但某些架构要求双地址形式（目标寄存器同时是源寄存器之一）。`TwoAddressInstructionPass` 在寄存器分配前将三地址指令转换为双地址等价形式。

```
// 三地址形式（转换前）
%a = ADD %b %c

// 双地址形式（转换后）
%a = MOVE %b
%a = ADD %a %c
```

#### SSA 解构阶段

SSA 形式简化了程序分析，但传统指令集不支持 PHI 节点。SSA 解构阶段（实现于 `lib/CodeGen/PHIElimination.cpp`）通过将 PHI 指令替换为保留语义的复制指令来消除 PHI 节点。寄存器分配器通过 `PHIEliminationID` 标识符调用此阶段。

#### 指令折叠

指令折叠在寄存器分配期间移除不必要的复制。例如：

```
// 折叠前
%EBX = LOAD %mem_address
%EAX = COPY %EBX

// 折叠后
%EAX = LOAD %mem_address
```

`TargetRegisterInfo::foldMemoryOperand()` 方法执行折叠。注意：折叠后的指令可能与原指令差异较大，需谨慎处理。

#### 内置寄存器分配器

LLVM 提供四种主要分配器：

| 分配器 | 说明 |
|--------|------|
| **Fast** | 调试构建的默认分配器，在基本块级别分配寄存器，尝试保留值并复用寄存器 |
| **Basic** | 增量式方法，使用启发式顺序分配活跃区间，适合性能基准测试 |
| **Greedy** | 生产构建的默认分配器，结合全局活跃区间分割以最小化溢出代码开销 |
| **PBQP** | 分区布尔二次规划求解器，构建并求解寄存器分配问题，将解映射回分配结果 |

通过命令行选项 `-regalloc=<linearscan|fast|pbqp>` 选择分配器。

### 5.5 序言/尾声代码插入

该阶段在寄存器分配确定栈空间需求后执行。编译器在函数入口处插入序言代码，在返回前插入尾声代码。主要职责包括：

- 建立栈帧
- 为局部变量和溢出槽分配空间
- 实现帧指针消除等优化
- 管理栈打包策略

### 5.6 紧凑展开信息（Compact Unwind）

紧凑展开信息作为 DWARF 帧信息的替代方案，提供高效的异常展开元数据。与 DWARF 每个函数需要 20–30 字节不同，紧凑展开只需 **4 字节**。

该格式将一个 32 位值编码，指定哪些寄存器需要恢复以及展开方法。链接器在创建最终镜像时生成 `__TEXT,__unwind_info` section，支持运行时快速访问展开信息。

**X86 紧凑展开模式：**

| 模式 | 说明 |
|------|------|
| **帧指针模式** | 使用 EBP/RBP（紧跟返回地址后压栈）。非易失性寄存器保存于 EBP-4 到 EBP-1020（64 位为 RBP-8 到 RBP-1020）。偏移编码在位 16-23；保存的寄存器在位 0-14（五个 3 位条目）|
| **无帧小栈模式** | 用于具有固定小栈大小的函数。栈大小除以 4（32 位）或 8（64 位）编码在位 16-23。寄存器计数在位 9-12；寄存器和顺序在位 0-9。最大 1024 字节（32 位）或 2048 字节（64 位）|
| **无帧大栈模式** | 当栈大小超过编码限制时，需要在序言中使用 `subl $nnnnnn, %esp`。到大小值的偏移编码在位 9-12 |

### 5.7 后期机器码优化

该阶段在寄存器分配完成后对最终机器码进行操作。适用的优化包括：

- 溢出代码调度
- 窥孔优化
- 目标特定增强

### 5.8 代码发射

代码发射将代码生成器抽象（`MachineFunction`、`MachineInstr`）降低到 MC 层构造（`MCInst`、`MCStreamer`）。这一最终步骤产生汇编文件或目标文件。

**关键类：**

| 类 | 说明 |
|----|------|
| `AsmPrinter` | 目标特定子类，处理从机器函数到 MC 层构造的通用降低 |
| 指令打印器 | 将 `MCInst` 对象渲染为文本，大部分由 `.td` 文件自动生成 |
| `MCInstLower` | 通常实现于 `<target>MCInstLower.cpp`，将 `MachineInstr` 转换为 `MCInst`，展开伪操作并处理重定位 |

后端必须提供三个组件：
1. `AsmPrinter` 子类（目标特定降低）
2. 指令打印器（操作数渲染）
3. `MCInstLower` 实现（`MachineInstr` 到 `MCInst` 的转换）

可选地，实现 `MCCodeEmitter` 子类以直接发射目标文件。

#### 发射函数栈大小信息

当 `TargetLoweringObjectFile::StackSizesSection` 已定义且 `TargetOptions::EmitStackSizeSection` 被启用（通过 `-stack-size-section`）时，编译器发射一个元数据 section，包含函数符号值与栈大小（无符号 LEB128 格式）的配对。

> 栈大小值仅包含在函数序言中分配的空间。具有动态栈分配的函数不包含在内。

#### 发射函数调用图信息

当 `TargetOptions::EmitCallGraphSection` 被设置（通过 `--call-graph-section`）时，编译器发射调用图元数据。布局详情参见专用的 ".llvm.callgraph Section Layout" 规范文档。

### 5.9 VLIW 打包器

VLIW 打包器为超长指令字（VLIW）架构提供与机器无关的指令分组（打包）功能，编译器负责将指令映射到功能单元。

#### 指令到功能单元的映射

VLIW 指令通常映射到多个功能单元。打包期间，编译器判断指令能否组合在一起。为管理这一复杂性，打包器在构建时通过确定性有限自动机（DFA）构造生成查找表。

#### 打包表的生成与使用

打包器分析目标行程（itinerary），创建表示数据包状态的 DFA，其元素包括：

- **输入**：正在添加到数据包中的指令
- **状态**：功能单元消耗级别
- **转换**：指令添加后的合法状态变化

缺少转换表示该指令不存在合法的功能单元映射。

**生成的 API 函数：**

| 函数 | 说明 |
|------|------|
| `DFAPacketizer::clearResources()` | 重置数据包状态 |
| `DFAPacketizer::reserveResources(MachineInstr *MI)` | 将指令添加到数据包 |
| `DFAPacketizer::canReserveResources(MachineInstr *MI)` | 验证指令添加的可行性 |

要启用表生成，在目标目录的 Makefile 中将 `_Target_GenDFAPacketizer.inc` 添加为目标。

---

## 6. 实现原生汇编器

LLVM 支持通过从 `.td` 文件自动生成来构建原生汇编器，减少手动数据录入，同时在编译器和汇编器之间共享规范。

### 6.1 指令解析

（待完善）

### 6.2 指令别名处理

别名处理阶段将同一指令的不同词法表示转换为统一形式，发生在 `MatchInstructionImpl` 函数内部，从最简单到最复杂依次处理。

#### 助记符别名（Mnemonic Aliases）

该初始别名阶段对支持备用名称的指令类进行直接的、无条件的名称映射。

X86 示例：

```tablegen
def : MnemonicAlias<"cbw",   "cbtw">;
def : MnemonicAlias<"smovq", "movsq">;
```

助记符别名可通过 `Requires` 子句依赖全局模式：

```tablegen
def : MnemonicAlias<"pushf", "pushfq">, Requires<[In64BitMode]>;
def : MnemonicAlias<"pushf", "pushfl">, Requires<[In32BitMode]>;
```

#### 指令别名（Instruction Aliases）

这是最全面的别名处理阶段，在匹配过程中起作用。它为匹配器提供新的匹配形式，以及对应要生成的特定指令。

每个指令别名包含两个组成部分：要匹配的模式和要生成的指令。示例：

```tablegen
// 重新排序操作数
def : InstAlias<"movsx $src, $dst", (MOVSX16rr8W GR16:$dst, GR8:$src)>;

// 将清零操作展开为 XOR
def : InstAlias<"clrb $reg", (XOR8rr GR8:$reg, GR8:$reg)>;

// 提供隐式立即数
def : InstAlias<"aad", (AAD8i8 10)>;
```

指令别名支持：重新排序操作数、多次使用输入、使用立即数、引用固定物理寄存器，以及通过 `Requires` 子句实现子目标特定性。

### 6.3 指令匹配

（待完善）

---

## 7. 目标相关实现注意事项

### 7.1 尾调用优化

尾调用优化允许被调用者重用调用者的栈，目前在 x86/x86-64、PowerPC、AArch64 和 WebAssembly 上支持。

**通用要求：**

- 调用者和被调用者使用调用惯例 `fastcc`、`cc 10`（GHC 调用惯例）、`cc 11`（HiPE 调用惯例）、`tailcc` 或 `swifttailcc`
- 调用必须在尾位置，且之后紧跟 `ret`
- 启用 `-tailcallopt` 选项，或调用惯例为 `tailcc`

**各平台约束：**

| 平台 | 约束 |
|------|------|
| x86/x86-64 | 无可变参数列表；x86-64 GOT/PIC 代码仅支持模块内调用 |
| PowerPC | 无可变参数列表或 byval 参数；ppc32/64 GOT/PIC 仅支持模块内调用 |
| WebAssembly | 无可变参数列表；启用 `tail-call` 目标属性；调用者和被调用者返回类型必须匹配 |
| AArch64 | 无可变参数列表 |

> **注意：** `-tailcallopt` 标志会使未优化的 fastcc 调用后跟栈重调整，可能降低性能。

### 7.2 兄弟调用优化（Sibling Call Optimization）

兄弟调用优化是尾调用优化的一种受限形式，在 x86/x86-64 上无需 `-tailcallopt` 即可自动执行。

**要求：**

- 调用者和被调用者使用相同的调用惯例（`c` 或 `fastcc`）
- 调用位于尾位置，且之后紧跟 `ret`
- 返回类型匹配，或被调用者结果未被使用
- 若被调用者参数位于栈上，则这些参数必须可从调用者的传入栈中获取

### 7.3 X86 后端

#### 支持的目标三元组

X86 后端支持 i386、i486、i586、i686 和 x86-64 等架构变体，以及各种操作系统组合（Linux、BSD、Windows、Darwin、Solaris）。

#### 支持的调用惯例

- C 调用惯例（cdecl）
- 快速调用惯例（fastcc）
- 不同操作系统的标准惯例

#### X86 寻址模式在 MachineInstr 中的表示

X86 指令可以使用复杂的寻址模式，组合寄存器、位移、比例索引和基址值。后端使用多个操作数在 `MachineInstr` 中编码完整的寻址模式。

#### X86 支持的地址空间

X86 后端支持代码、数据等段的标准内存地址空间，遵循 x86 架构规范。

#### 指令命名惯例

X86 指令名称遵循约定，指示操作数大小（`b`=字节, `w`=字, `l`=长字, `q`=四字）和操作数类型（`r`=寄存器, `m`=内存, `i`=立即数）。

### 7.4 PowerPC 后端

#### LLVM PowerPC ABI

PowerPC 后端实现了管理函数调用、寄存器使用和栈布局的应用程序二进制接口（ABI）。

#### 帧布局

PowerPC 使用特定的帧布局，包含参数区域、局部变量区域和保存寄存器区域，遵循 ABI 规范。

#### 序言/尾声

- **序言**：初始化栈帧并保存寄存器
- **尾声**：在返回前恢复寄存器并释放帧

#### 动态分配

PowerPC 后端通过 `alloca` 操作支持动态内存分配，在运行时管理可变大小的栈空间。

### 7.5 NVPTX 后端

NVPTX 后端通过 PTX 中间表示针对 NVIDIA GPU，生成可在支持 CUDA 的设备上执行的代码。它处理 GPU 特定的并行性、内存层次结构和指令集。

### 7.6 eBPF 后端

扩展的 Berkeley 包过滤器（eBPF）后端用于 Linux 内核虚拟机。

#### 指令编码（算术与跳转）

算术和跳转操作使用 64 位指令格式编码，包含操作码、寄存器和立即数字段。

#### 指令编码（Load/Store）

Load 和 Store 指令指定内存访问操作，包含寻址模式和字节/半字/字/双字的大小规格。

#### 数据包访问（BPF_ABS、BPF_IND）

专用指令支持绝对和间接数据包数据访问，允许程序检查指定偏移处的网络数据包内容。

#### eBPF Maps

Maps 提供键值存储，可从 eBPF 程序访问，支持内核空间与用户空间之间的数据共享。

#### 函数调用

eBPF 支持函数调用，调用惯例定义了参数传递和返回值处理方式。

#### 程序入口

eBPF 程序从指定入口点开始执行，初始化上下文包含数据包数据和其他运行时信息。

### 7.7 AMDGPU 后端

AMDGPU 后端针对 AMD 图形处理器和计算加速器，处理 GPU 特定功能，包括 wave 级并行性、局部内存以及图形硬件固有的指令级并行性。

### 7.8 轻量级故障隔离（LFI）子架构

LFI 子架构通过沙箱化机制提供内存安全，将内存访问限制在指定区域内，防止未授权访问或权限提升。

---

## 参考资料

- [LLVM Target-Independent Code Generator 官方文档](https://llvm.org/docs/CodeGenerator.html)
- [Writing an LLVM Backend](https://llvm.org/docs/WritingAnLLVMBackend.html)
- [LLVM TableGen 文档](https://llvm.org/docs/TableGen/index.html)
- [TutorialLLVMBackendCpu0](http://jonathan2251.github.io/lbd/)
