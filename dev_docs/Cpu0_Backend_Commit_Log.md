# Cpu0 Backend — Commit Log

## 文档目的

这份文档不是逐段复述教程，而是按**每一步对应的提交**来记录我们在
Cpu0 backend 开发过程中到底把系统推进到了什么状态、
每一步为什么要在那个时间点做，以及这些步骤之间是怎样串起来的。

如果只看原始小节文档，很容易把它们理解成一串分散的小任务。站在实际开发过程里，
这些已经落地的提交更像是一条连续的 bring-up 路线：

1. 先给 Cpu0 建立 LLVM 能识别的”身份”
2. 再用 TableGen 把寄存器、指令和 ABI 骨架描述出来
3. 再把目标接入 LLVM 的注册、构建和库分层体系
4. 验证这套最小骨架真的能被完整编译出来
5. 建立完整后端 C++ 类层次，让 codegen pipeline 能跑起来
6. 修复级联崩溃，让 llc 端到端输出合法 Cpu0 汇编

---

## 阶段总览

| Section | Commit | 我们实际完成的里程碑 |
|---|---|---|
| 2.4.1 | `84cf21f9619bc03d36579dfa9ef56b467c90648d` | 让 LLVM 在 Triple / ELF / relocation / data layout 层面认识 Cpu0 |
| 2.4.2 | `4ea9f615de473ac435a3443101079142507b3f8f` | 建立 Cpu0 的 `.td` 骨架，定义寄存器、指令格式、初始指令集、调度与调用约定 |
| 2.4.3 | `cfec707741a14e0552f8baf6b201914734e99567` | 把 Cpu0 backend 注册进 LLVM 的 TargetInfo / MCTargetDesc / CodeGen 体系 |
| 2.4.4 | `cfec707741a14e0552f8baf6b201914734e99567` | 验证 TableGen 和三个 Cpu0 组件库可以被真正构建出来 |
| 3.1   | `65e0954bd083`                             | 建立完整 C++ 后端类层次，把 Cpu0 从空壳推进成能跑通最小 codegen 流程的 backend skeleton |
| 3.2   | `b3335206d25e`                            | 修复三类级联崩溃：`Cpu0ISD::Ret` 无法选择、`FrameIndex` 无法降低、`Cpu0FI` null dereference |
| 3.3–3.5 | `f6a0a213667d`                          | 实现 prologue/epilogue 栈帧分配、callee-saved 寄存器溢出恢复、subtarget 访问工具方法；章节门控推进至 CH3_5 |
| 4.1   | `c6862d06a91c`                             | 实现完整算术/逻辑/移位/旋转/乘除指令集，添加 HI/LO 寄存器对，接入 DAG combine 驱动的除法降低，实现 copyPhysReg 和 selectMULT |

这份文件的组织方式是：

- 先列每一步对应的 commit
- 再说明该 commit 实际解决了什么问题
- 最后串起来看目前整个 Cpu0 backend skeleton 的状态

2.4.3 和 2.4.4 在代码上来自同一个提交，但从开发流程上是两个不同的问题：

- 2.4.3 关注“线路有没有接上”
- 2.4.4 关注“接上以后能不能真正编过去”

---

## 目录

| # | 章节 | Commit | 核心内容 |
|---|---|---|---|
| Stage 1 | [2.4.1 — 让 LLVM 认识 Cpu0](#stage-1--241让-llvm-认识-cpu0) | `84cf21f9` | Triple / ELF machine id / relocation / data layout 注册 |
| Stage 2 | [2.4.2 — 声明性后端骨架](#stage-2--242把声明性后端骨架搭起来) | `4ea9f615` | TableGen 骨架：寄存器、指令格式、ISA、调度、调用约定 |
| Stage 3 | [2.4.3 — 注册与构建体系](#stage-3--243把-cpu0-接进-llvm-的注册和分层构建体系) | `cfec7077` | TargetInfo / MCTargetDesc / CodeGen 三层接入 LLVM registry |
| Stage 4 | [2.4.4 — 首次构建验证](#stage-4--244第一次验证这套骨架真的能编译起来) | `cfec7077` | 验证 TableGen `.inc` 生成、三库构建、目标注册符号存在 |
| Stage 5 | [3.1 — 完整 C++ 类层次](#stage-5--31targetmachine-结构建立后端-c-类层次) | `65e0954b` | Subtarget / RegisterInfo / InstrInfo / FrameLowering / ISelLowering / MCAsmInfo 骨架 |
| Stage 6 | [3.2 — AsmPrinter 与函数返回](#stage-6--32asmprinter-与函数返回修复三类级联崩溃) | `b3335206` | 修复 Ret 无法选择、FrameIndex 无法降低、Cpu0FI null dereference |
| Stage 7 | [3.3–3.5 — 栈帧与寄存器溢出](#stage-7--335栈帧实现寄存器溢出恢复与-subtarget-工具方法) | `f6a0a213` | emitPrologue/Epilogue、storeRegToStackSlot/loadRegFromStackSlot、isLittleEndian() |
| Stage 8 | [4.1 — 算术指令集](#stage-8--41算术指令集完整-isa-乘除法与-dag-combine) | `c6862d06` | HI/LO 寄存器、算术/逻辑/移位/旋转/乘除指令、DAG combine 除法降低、copyPhysReg、selectMULT |

---

## Stage 1 — 2.4.1：先让 LLVM 认识 Cpu0

从开发者视角看，Section 2.4.1 做的不是“开始实现 Cpu0 后端”，而是先把
**Cpu0 这个目标架构注册进 LLVM 的公共基础设施**。

换句话说，这一步的目标是让 LLVM 先回答下面这些最基础的问题：

1. 当用户写 `cpu0`、`cpu0el`、`cpu0eb` 时，LLVM 知道这是哪个架构吗？
2. 当 LLVM 以后产出一个 Cpu0 ELF 文件时，文件头里该写什么 `e_machine`？
3. 当目标文件里出现 Cpu0 relocation 时，LLVM 知道这些 relocation 的名字吗？
4. 当 IR 或对象文件说“我是 Cpu0”时，LLVM 知道这个目标的 data layout 是什么吗？
5. 当 LLVM 重新读回一个 Cpu0 ELF 文件时，它能反推出正确的 `Triple` 吗？

如果这些问题没有先解决，后面即使我们写了 TableGen、寄存器、指令选择、
汇编器或者 MC 层，整个工具链仍然会在很前面的基础设施层面卡住。

所以这一步本质上是一次 **infrastructure bring-up**：
先把身份、格式、布局、重定位、对象文件识别这些“最底层的通路”接起来，
让 Cpu0 至少能被 LLVM 当成一个合法 target 看待。

---

## 对应提交

- Commit: `84cf21f9619bc03d36579dfa9ef56b467c90648d`
- Subject: `Update 2.4.1`
- AuthorDate: `2026-04-17 03:45:33 -0400`

这次提交一共改了 8 个文件：

- 7 个功能文件
- 1 个教程文档

其中真正影响 LLVM 行为的是下面这 7 个功能文件。

---

## 从系统角度看，这一步补齐了哪几层

这一节不是在单点加功能，而是在同一时间把 4 层能力补齐：

1. Triple 层：让 LLVM 认识 `cpu0` / `cpu0el`
2. ABI / DataLayout 层：让 LLVM 知道 Cpu0 的类型大小和对齐规则
3. ELF 常量 / relocation 层：让 LLVM 认识 Cpu0 的 machine id 和 relocation 编号
4. ELF 读取层：让 LLVM 能把一个现有 Cpu0 ELF 文件识别回 `Triple::cpu0`

这四层必须一起补。如果只补其中一部分，系统就会出现“能写不能读”、
“能识别名字但不知道 ABI”、或者“能识别 ELF machine 但 relocation 名字全是
Unknown”这类半通状态。

---

## 7 个功能文件分别做了什么，为什么必须做

### 1. `llvm/include/llvm/TargetParser/Triple.h`

这里给 `Triple::ArchType` 增加了：

- `cpu0`
- `cpu0el`

这是整个接入过程的起点。因为 LLVM 里很多地方并不是直接比较字符串，而是先把
目标架构收敛成 `ArchType` 枚举，再围绕这个枚举做分派。

为什么必须改：

- 如果这里没有 `cpu0` / `cpu0el`，后面的 `Triple.cpp` 根本没有合法枚举值可处理
- 其他模块也不能把 Cpu0 当作一个正式 target 来区分

如果不做：

- `"cpu0-unknown-linux"` 这样的 triple 最终只能落到 `UnknownArch`
- 后面所有依赖 `ArchType` 的逻辑都无法稳定工作

---

### 2. `llvm/lib/TargetParser/Triple.cpp`

这里是给 `Triple.h` 里新增的枚举补上“行为定义”。

这次实际补的是 **12 处** 逻辑点：

| Function | 实际作用 |
|---|---|
| `getArchTypeName()` | 把枚举转回 `"cpu0"` / `"cpu0el"` |
| `getArchTypePrefix()` | 给两个变体统一前缀 `"cpu0"` |
| `getArchTypeForLLVMName()` | 解析 LLVM 风格名字：`cpu0` / `cpu0eb` / `cpu0el` |
| `parseArch()` | 解析 target triple 里的架构字符串 |
| `getDefaultFormat()` | 指定默认对象文件格式为 ELF |
| `getArchPointerBitWidth()` | 指定 Cpu0 是 32 位指针架构 |
| `get32BitArchVariant()` | 说明它本身就是 32 位变体 |
| `get64BitArchVariant()` | 说明它没有 64 位变体 |
| `getBigEndianArchVariant()` | `cpu0el -> cpu0` |
| `getLittleEndianArchVariant()` | `cpu0 -> cpu0el` |
| `isLittleEndian()` | 把 `cpu0el` 标记为小端 |
| `getDefaultExceptionHandling()` | 默认返回 `DwarfCFI` |

为什么必须改：

- `Triple.h` 只定义“有这个架构”，`Triple.cpp` 才定义“这个架构在系统里怎么表现”
- LLVM 需要知道 Cpu0 是 32 位、默认用 ELF、怎么处理大小端、怎么从字符串解析

如果不做：

- triple 解析可能返回 `UnknownArch`
- 大小端转换函数可能走到 `llvm_unreachable`
- 指针宽度查询、默认对象格式查询都会不正确
- 不同模块对 Cpu0 的理解会不一致

开发者备注：

- 原文档写的是 `eleven switch-statement sites`，但按代码实际变更数应为 **12 处**
- 原文档把 `getDefaultExceptionHandling()` 写成了 `No EH`，但代码实际返回的是
  `DwarfCFI`

这两个点是代码核对后得到的实际结果。

---

### 3. `llvm/lib/TargetParser/TargetDataLayout.cpp`

这里为 `cpu0` 和 `cpu0el` 补上了 data layout：

- 大端版本以 `E-` 开头
- 小端版本以 `e-` 开头
- 其余布局规则基本按这套教程里 MIPS 风格的 32 位 ABI 走

为什么必须改：

- LLVM 不只是“知道这是 Cpu0”就够了，它还必须知道这个目标上的类型大小、对齐、
  向量布局、栈对齐等 ABI 规则
- data layout 是 IR、优化器、后续 codegen 共同依赖的基础信息

如果不做：

- LLVM 即使能认出 `cpu0` triple，也不知道这个 target 的 ABI 布局
- struct layout、load/store 对齐、类型大小推导都可能出错

---

### 4. `llvm/include/llvm/BinaryFormat/ELF.h`

这里做了两件事：

1. 增加 `EM_CPU0 = 999`
2. 通过 `#include "ELFRelocs/Cpu0.def"` 把 Cpu0 relocation 常量导入 ELF 枚举体系

为什么必须改：

- `EM_CPU0` 是以后 Cpu0 ELF 文件写进 `e_machine` 字段的机器号
- relocation 编号也必须在 LLVM 的 ELF 公共定义里有统一入口

如果不做：

- Cpu0 目标文件没有正式的 ELF machine identity
- relocation 值只能散落在局部实现里，无法成为 LLVM 公共常量的一部分

开发者备注：

- `999` 是教程里使用的非标准值，只是为了示范接入流程
- 真正的上游架构一般会使用正式分配的 `e_machine` 编号

---

### 5. `llvm/include/llvm/BinaryFormat/ELFRelocs/Cpu0.def`

这是新建文件，里面统一定义了 24 个 `R_CPU0_*` relocation。

为什么必须单独建 `.def` 文件：

- LLVM 很多架构的 relocation 都采用这种 `.def` 复用模式
- 同一份列表可以被 `ELF.h`、`ELF.cpp` 以及后续 MC / linker 代码重复 include
- 这样 relocation 编号和名字只有一份事实来源

如果不做：

- relocation 列表就得在多个文件里手写重复版本
- 后续扩展时更容易出现编号和名字不同步的问题

---

### 6. `llvm/lib/Object/ELF.cpp`

这里给 `getELFRelocationTypeName()` 增加了 `EM_CPU0` 分支，通过 `Cpu0.def`
把 relocation 编号映射回名字。

为什么必须改：

- 定义了 relocation 常量，不等于工具就知道怎么打印它们
- `llvm-readobj`、`llvm-objdump` 之类对象文件工具最终会走这里拿名字

如果不做：

- Cpu0 ELF 里即使真的带 relocation，工具打印时也只会显示 `Unknown`
- 后续调试 assembler / MC / linker 时会非常不方便

---

### 7. `llvm/include/llvm/Object/ELFObjectFile.h`

这里补上了两类反向识别逻辑：

1. `getFileFormatName()`
   返回 `elf32-cpu0` 或 `elf32-cpu0el`
2. `getArch()`
   根据 `EM_CPU0 + ELFCLASS32 + endianness` 反推出 `Triple::cpu0` 或 `Triple::cpu0el`

为什么必须改：

- 写出 Cpu0 ELF 只是单向流程
- LLVM 还必须能把一个已有的 Cpu0 ELF 文件重新识别回来

如果不做：

- 未来即使某个阶段能产出 Cpu0 ELF，LLVM 自己重新打开它时也未必认得出这是 Cpu0
- 对象文件工具展示的格式名也会不对

---

## 为什么这一步要现在做，而不是后面再补

从工程顺序上，这一步必须尽量早做，因为它属于“地基”：

- 没有 triple 注册，`-march=cpu0`、target triple、目标架构分派都不成立
- 没有 data layout，IR 层就没有正确 ABI
- 没有 ELF machine id，后续目标文件没有合法身份
- 没有 relocation 名字映射，对象文件调试体验会很差
- 没有 ELF 反向识别，工具链 round-trip 是断的

这意味着后面无论你做：

- TableGen 描述
- 寄存器信息
- 指令定义
- Instruction Selection
- MC 编码
- 汇编器/反汇编器

都默认建立在“LLVM 已经把 Cpu0 视为一个合法 target”的前提上。

Section 2.4.1 做的就是把这个前提先建立起来。

---

## 这一步完成后，LLVM 获得了什么能力

完成这一节后，LLVM 至少具备了这些能力：

- 能把 `cpu0` / `cpu0el` / `cpu0eb` 解析成合法架构
- 能区分 Cpu0 的大小端变体
- 能为 Cpu0 选择默认 ELF 对象格式
- 能给 Cpu0 计算 data layout
- 能识别 `EM_CPU0`
- 能识别并打印 24 个 `R_CPU0_*` relocation 名字
- 能把一个 Cpu0 ELF 文件识别成 `elf32-cpu0` 或 `elf32-cpu0el`

这些能力本身还不足以“生成 Cpu0 机器码”，但足以让后续 backend bring-up
建立在正确的基础设施上。

---

## 这一步还没有做什么

为了避免误解，也需要明确这一步**还没有**完成下面这些事情：

- 还没有定义 Cpu0 的寄存器文件
- 还没有定义指令集和 TableGen 描述
- 还没有实现 SelectionDAG / GlobalISel 相关逻辑
- 还没有实现 MCInst lowering、编码、汇编输出
- 还没有实现 asm parser / disassembler
- 还没有真正让 `llc` 产出可执行的 Cpu0 机器指令

所以这一步更准确的定位不是“后端功能实现”，而是“后端接入准备”。

---

## 用一句话总结

站在开发者视角，Section 2.4.1 做的是：

**先把 Cpu0 在 LLVM 里的“身份信息、对象文件身份、重定位命名、ABI 布局、对象文件回读路径”全部注册好，为后续真正的后端实现铺平路。**

---

## Stage 2 — 2.4.2：把声明性后端骨架搭起来

### 对应提交

- Commit: `4ea9f615de473ac435a3443101079142507b3f8f`
- Subject: `Update 2.4.2`
- AuthorDate: `2026-04-17 04:38:19 -0400`

这次提交一共改了 9 个文件：

- 8 个功能文件
- 1 个教程文档

这一步最重要的变化是：Cpu0 不再只是“LLVM 认识的一个名字”，而是第一次拥有了
**可以被 TableGen 消化的 backend 描述骨架**。

### 这一步我们做了什么

我们新建了 8 个 `.td` 文件，把 Cpu0 后端最小可用的声明性模型搭起来：

| 文件组 | 作用 |
|---|---|
| `Cpu0RegisterInfo.td` | 定义物理寄存器、寄存器类和编码 |
| `Cpu0RegisterInfoGPROutForOther.td` | 定义不含 `SW` 的输出寄存器类，避免条件码寄存器被错误分配 |
| `Cpu0Schedule.td` | 定义最小调度模型和 itinerary |
| `Cpu0InstrFormats.td` | 定义 A/L/J 三种编码格式 |
| `Cpu0InstrInfo.td` | 定义初始指令集、operand、pattern 和 chapter gating |
| `Cpu0CallingConv.td` | 定义 O32 callee-saved 集合和 calling convention 辅助规则 |
| `Cpu0.td` | 定义 target、processor、subtarget feature |
| `Cpu0Other.td` | 作为 TableGen 的统一入口文件 |

从 bring-up 的角度看，这一步真正补齐的是 6 件事：

1. 把寄存器文件显式定义出来
2. 把指令编码格式显式定义出来
3. 把最小初始指令集显式定义出来
4. 把调度模型和调用约定显式定义出来
5. 把 processor / feature / chapter gating 体系搭起来
6. 把 TableGen 的根文件和生成入口固定下来

### 为什么这一步必须紧跟在 2.4.1 后面

2.4.1 只是解决“LLVM 认识 Cpu0 是谁”，但还没有解决“LLVM 认为 Cpu0 长什么样”。

对一个 LLVM backend 来说，寄存器、指令、调度、调用约定这些元数据不能主要靠
手写 C++ 来描述，它们必须先进入 TableGen 世界。因为后面大量的自动生成文件都要
建立在这些 `.td` 描述上，比如：

- `Cpu0GenRegisterInfo.inc`
- `Cpu0GenInstrInfo.inc`
- `Cpu0GenSubtargetInfo.inc`
- `Cpu0GenCallingConv.inc`

如果跳过这一步直接去写 `TargetMachine`、`AsmPrinter` 或者 `ISel`，很快就会发现：

- 没有正式寄存器类可引用
- 没有 instruction enum / descriptor 可引用
- 没有 processor / feature bit 可引用
- 没有 callee-saved register set 可引用

换句话说，2.4.2 是把“目标身份”推进成“目标结构”。

### 这一步对后续开发的实际价值

做完 2.4.2 后，我们第一次具备了以下开发前提：

- 可以让 TableGen 为 Cpu0 生成基础 `.inc` 文件
- 可以在后续 C++ 代码里引用正式的寄存器和指令描述
- 可以把教程后续章节按 `FeatureChapter*` 逐步放开，而不需要每章重构一遍 target
- 可以把 `SW` 当作特殊寄存器处理，避免它误入普通输出寄存器集合

这一步其实就是在回答一句话：

**“如果 LLVM 现在要为 Cpu0 生成自己的内部描述表，它知道该从哪里开始吗？”**

2.4.2 完成后，这个问题第一次可以回答“知道”。

### 这一步还没有做什么

虽然 `.td` 骨架已经建立，但它仍然不是一个能工作的后端。此时我们还没有：

- 把 Cpu0 target 注册到 `llc` 可发现的目标列表里
- 定义 `TargetMachine`
- 定义 `TargetInfo` / `MCTargetDesc` 的 C++ 初始化入口
- 让 `LLVMCpu0Info` / `LLVMCpu0Desc` / `LLVMCpu0CodeGen` 真的参与构建

所以 2.4.2 完成后，Cpu0 拥有的是“声明性后端模型”，还不是“可链接的 LLVM target”。

### 用一句话总结

站在开发者视角，Section 2.4.2 做的是：

**把 Cpu0 从“只有身份的目标”推进成“有寄存器、指令格式、初始 ISA、调用约定和 TableGen 入口的声明性后端骨架”。**

---

## Stage 3 — 2.4.3：把 Cpu0 接进 LLVM 的注册和分层构建体系

### 对应提交

- Commit: `cfec707741a14e0552f8baf6b201914734e99567`
- Subject: `Update 2.4.3 and 2.4.4`
- AuthorDate: `2026-04-17 04:41:15 -0400`

这一提交同时覆盖 2.4.3 和 2.4.4。对 2.4.3 而言，重点是把 Cpu0 backend
从“有 `.td` 和零散源文件”推进成“LLVM 运行时能发现、构建系统能编译”的正式目标。

### 这一步我们做了什么

这一节主要补的是 C++ 层和构建层的接线工作：

| 文件组 | 作用 |
|---|---|
| `llvm/CMakeLists.txt` | 把 `Cpu0` 加入 `LLVM_ALL_EXPERIMENTAL_TARGETS` |
| `llvm/lib/Target/Cpu0/CMakeLists.txt` | 定义 TableGen 生成规则和 `LLVMCpu0CodeGen` |
| `TargetInfo/*` | 提供 `getTheCpu0Target()` / `getTheCpu0elTarget()` 和 `LLVMInitializeCpu0TargetInfo()` |
| `MCTargetDesc/*` | 提供 `LLVMInitializeCpu0TargetMC()` stub 和生成枚举入口 |
| `Cpu0.h` / `Cpu0TargetMachine.*` | 定义并注册 big-endian / little-endian 两个 `TargetMachine` |
| `Cpu0.td` | 增加 `RemapAllTargetPseudoPointerOperands<CPURegs>` 适配现代 LLVM |

从开发者角度看，这一步真正完成的是：

1. 把 Cpu0 接入 LLVM 的 experimental target 清单
2. 把 Cpu0 backend 切分成 `Info` / `Desc` / `CodeGen` 三层
3. 给 Cpu0 提供运行时 target singleton 和初始化入口
4. 给 Cpu0 提供 `TargetMachine` 骨架
5. 让 CMake 知道如何从 `Cpu0Other.td` 生成 `.inc`
6. 解决现代 LLVM 对 `ptr_rc` remap 的要求，避免 TableGen 直接失败

### 为什么这一步要在 2.4.2 之后立刻做

2.4.2 解决的是“描述有没有”，2.4.3 解决的是“LLVM 能不能把这些描述当成一个目标来使用”。

如果没有 2.4.3，即使 `.td` 文件已经很完整，仍然会卡在这些地方：

- `llc -march=cpu0` 找不到目标
- 构建系统不会生成 Cpu0 相关库
- `LLVMInitializeCpu0TargetInfo()` / `LLVMInitializeCpu0TargetMC()` /
  `LLVMInitializeCpu0Target()` 没有入口
- `TargetMachine` 没有注册，工具也无法实例化目标机器

所以 2.4.3 的本质，是把 Cpu0 从“源码目录里的描述文件集合”推进成“LLVM target
registry 里真的存在的目标”。

### 这一步里最关键的几个设计点

- `TargetInfo` 层先落地：
  先把 target singleton 和 `RegisterTarget<Triple::cpu0>` 接起来，LLVM 工具才能发现 Cpu0。

- `MCTargetDesc` 先用 stub：
  这一章还没有真正实现 asm info、inst printer、code emitter，但初始化入口必须先占位，
  这样后面章节只需要往这个框架里填内容。

- `TargetMachine` 先给骨架：
  这时 `Cpu0Subtarget` 还没真正成型，但 big-endian / little-endian 的 `TargetMachine`
  类型和注册动作必须先存在，否则 `llc -march=cpu0` 无法走到后面。

- `RemapAllTargetPseudoPointerOperands<CPURegs>` 必须补：
  这是现代 LLVM 相比旧教程实现多出来的一处兼容点。不补这行，TableGen 会因为
  `ptr_rc` 无法映射而报错，整个 target 根本进不了构建。

### 这一步完成后，我们拿到了什么

完成 2.4.3 后，Cpu0 backend 第一次拥有了“被 LLVM 视为一个真正 target”的外形：

- LLVM 配置阶段可以选择构建 Cpu0
- `llc --version` 有机会列出 `cpu0` / `cpu0el`
- `TargetInfo` / `MCTargetDesc` / `CodeGen` 三层结构已经齐了
- `TargetMachine` 的注册路径已经接上
- TableGen 生成产物和 C++ 源码第一次处在同一套构建图里

但要注意，此时更准确的说法是：

**目标已经“注册成功”，但还没有“实现完整”。**

### 用一句话总结

站在开发者视角，Section 2.4.3 做的是：

**把 Cpu0 从“已经有 `.td` 骨架的源码集合”推进成“LLVM 构建系统和 target registry 真正能识别、能初始化、能实例化的 backend skeleton”。**

---

## Stage 4 — 2.4.4：第一次验证这套骨架真的能编译起来

### 对应提交

- Commit: `cfec707741a14e0552f8baf6b201914734e99567`
- Subject: `Update 2.4.3 and 2.4.4`
- AuthorDate: `2026-04-17 04:41:15 -0400`

2.4.4 没有再引入一批新的核心源文件，它的重点是验证：

**2.4.1 到 2.4.3 这几步拼起来以后，Cpu0 backend 是不是真的能被 LLVM 编译系统完整接受。**

### 这一步我们实际验证了什么

2.4.4 关注两类产物：

| 产物 | 说明 |
|---|---|
| `Cpu0GenCallingConv.inc` | 由 `-gen-callingconv` 生成 |
| `Cpu0GenInstrInfo.inc` | 由 `-gen-instr-info` 生成 |
| `Cpu0GenRegisterInfo.inc` | 由 `-gen-register-info` 生成 |
| `Cpu0GenSubtargetInfo.inc` | 由 `-gen-subtarget` 生成 |
| `libLLVMCpu0Info.a` | TargetInfo 层静态库 |
| `libLLVMCpu0Desc.a` | MCTargetDesc 层静态库 |
| `libLLVMCpu0CodeGen.a` | CodeGen 层静态库 |

从开发流程看，2.4.4 不是“再写功能”，而是把整个最小 backend 骨架第一次真正跑通：

1. TableGen 能否从 `Cpu0Other.td` 顺利生成四个 `.inc`
2. 这些 `.inc` 能否被 `TargetInfo` / `Desc` / `CodeGen` 代码正确 include
3. 三个 Cpu0 静态库能否在 LLVM 的标准分层下被构建出来
4. 目标注册符号是否真的出现在最终产物里

### 为什么 2.4.4 必须单独作为一个里程碑

在 backend bring-up 里，“代码写完了”和“系统真的接通了”是两回事。

2.4.4 的价值就在于，它把状态从：

- “看起来文件都齐了”

推进到：

- “LLVM 构建系统已经承认这是一套可编译的 backend skeleton”

这一节特别容易暴露那些纸面上不明显、但一编就炸的集成问题，比如：

- target 名字没接进 CMake 总表
- `LLVM_TARGET_DEFINITIONS` 指向了错误的根 `.td`
- `ptr_rc` 没有 remap
- 某个旧 LLVM API 在当前版本已经失效
- 三层库之间的依赖关系不完整

### 从工程意义看，2.4.4 代表什么

完成 2.4.4 后，我们终于拿到一个很重要的性质：

**Cpu0 不再只是“正在开发中的后端源码”，而是“已经可以进入 LLVM 正常构建流程的后端骨架”。**

这意味着后面继续补：

- `Subtarget`
- `FrameLowering`
- `InstrInfo`
- `ISelLowering`
- `AsmPrinter`
- `MCCodeEmitter`

时，不需要再反复怀疑“是不是连构建框架都没接好”。这会极大降低后续章节的排障范围。

### 用一句话总结

站在开发者视角，Section 2.4.4 做的是：

**把前面几节搭出来的 Cpu0 backend 骨架第一次放进真实构建链路里验证，确认 TableGen、库分层和目标注册已经能一起工作。**

---

## 截至 2.4.4，我们已经具备了什么

走到 2.4.4 结束时，Cpu0 backend 已经具备这些基础能力：

- LLVM 能识别 `cpu0` / `cpu0el` / `cpu0eb`
- LLVM 知道 Cpu0 的 ELF machine id、relocation 名字和 data layout
- Cpu0 已经拥有寄存器、指令格式、初始指令集、调度模型、调用约定的 `.td` 描述
- TableGen 已经能为 Cpu0 生成基础 `.inc` 文件
- Cpu0 已经接入 LLVM 的 experimental target 列表
- `TargetInfo` / `MCTargetDesc` / `CodeGen` 三层组件已经搭好
- `TargetMachine` 的注册路径已经建立
- 三个 Cpu0 静态库已经可以被构建

这说明我们已经跨过了“目标不存在”和“目标无法构建”这两个最早期门槛。

---

## 截至 2.4.4，我们还没有完成什么

但这离一个真正可用的 backend 还有明显距离。当前还缺少的核心部分至少包括：

- 真正可工作的 `Subtarget` 实现
- `PassConfig` 和 codegen pass pipeline 定制
- `MachineFunctionInfo`、`RegisterInfo`、`FrameLowering` 等运行期 C++ 实现
- 更完整的 `InstrInfo` / `ISelLowering` / DAG 选择逻辑
- asm info、inst printer、asm parser、disassembler
- `MCCodeEmitter` 和真正的机器码编码
- 真正的 object emission、relocation 应用和汇编/反汇编链路
- 更系统的 lit / unit test 覆盖

所以现在的 Cpu0 更准确地说是：

**一个已经被 LLVM 接纳、能被构建、但还没有完整 codegen 能力的 backend skeleton。**

---

## 总结这四步的开发含义

如果把 2.4.1 到 2.4.4 连起来看，这四步不是四个孤立动作，而是一条非常标准的
LLVM backend bring-up 路径：

1. 先注册目标身份
2. 再定义声明性模型
3. 再接入注册和构建体系
4. 最后验证这套骨架真的能构建

所以截至当前阶段，我们最重要的成果不是”已经能生成 Cpu0 机器码”，而是：

**我们已经把 Cpu0 从一个教程里的概念目标，推进成了 LLVM 源码树中一个真实存在、可被识别、可被构建、并且具备最小后端骨架的 experimental target。**

---

## Stage 5 — 3.1：TargetMachine 结构——建立后端 C++ 类层次

### 这一步的目的

2.4.3 把 Cpu0 接进了 LLVM 的注册和构建体系，但 `TargetMachine` 只是一个空壳，
`Subtarget` 不存在，`RegisterInfo`、`InstrInfo`、`FrameLowering` 也都没有真正实现。
这意味着虽然 `llc -march=cpu0` 已经能找到目标，但一旦进入真正的 codegen 流程就会崩溃。

这一步（对应教程 Section 3.1 — TargetMachine）的核心任务是建立 `Cpu0TargetMachine`
及其聚合的完整后端类层次：

**把 Cpu0 从”能被注册的空壳”推进成”有完整 TargetMachine 类结构、能跑通最小 codegen 流程的后端骨架”。**

具体要解决三个层面的问题：

1. **Subtarget 层**：让 `TargetMachine` 有一个真正的 `Subtarget`，把所有后端组件聚合起来
2. **MC 层**：补全 `MCAsmInfo` 和 `ABIInfo`，让工具不在初始化阶段崩溃
3. **CodeGen 层**：补全 `RegisterInfo`、`InstrInfo`、`ISelLowering`、`ISelDAGToDAG`、
   `FrameLowering`、`MachineFunction`、`TargetObjectFile` 的骨架实现，
   让 codegen pass pipeline 能完整走下去

---

### 这一步修改了哪些文件，为什么修改

#### 新建文件（20 个）

##### Cpu0Subtarget.cpp / Cpu0Subtarget.h

`Subtarget` 是 Cpu0 backend 的核心聚合器。它持有当前目标变体（大端 / 小端）的
所有组件引用，包括 `InstrInfo`、`FrameLowering`、`RegisterInfo`、`ISelLowering`
以及 `TSInfo`。

为什么必须建：

- 现代 LLVM 的 `TargetMachine` 通过 `Subtarget` 来获取所有运行期后端组件
- `Subtarget` 同时负责解析 CPU 特性字符串（如 feature bits）并驱动 `SubtargetInfo`
- 没有 `Subtarget`，`TargetMachine::getSubtargetImpl()` 会返回 null，
  任何 codegen pass 访问 subtarget 时立即崩溃

在这一步里，`Cpu0Subtarget` 采用”SE 模式”——持有 `Cpu0SEInstrInfo`、
`Cpu0SERegisterInfo`、`Cpu0SEFrameLowering`、`Cpu0SEISelLowering` 的具体实例，
并通过基类指针暴露给外部。这种 base / SE 分离模式来自 MIPS backend，
为将来扩展多变体（如 Cpu0S）预留了设计空间。

##### Cpu0RegisterInfo.cpp / Cpu0RegisterInfo.h
##### Cpu0SERegisterInfo.cpp / Cpu0SERegisterInfo.h

`RegisterInfo` 是寄存器分配和帧操作的核心。

`Cpu0RegisterInfo`（基类）实现：

- `getCalleeSavedRegs()`：返回 callee-saved 寄存器集合
- `getReservedRegs()`：标记不可分配的寄存器（SP、FP、LR、PC 等）
- `eliminateFrameIndex()`：把帧索引伪指令展开成真实的 SP/FP 偏移指令
- `getFrameRegister()`：返回帧寄存器

`Cpu0SERegisterInfo`（SE 子类）：目前只是薄包装，继承基类行为，未来可按变体定制。

为什么必须建：

- 没有 `RegisterInfo`，寄存器分配 pass 找不到可用寄存器集合
- `eliminateFrameIndex()` 不实现，含局部变量的函数编译到帧索引消除阶段必然崩溃
- `getReservedRegs()` 不实现，SP/FP 可能被错误分配给普通用途

##### Cpu0InstrInfo.cpp / Cpu0InstrInfo.h
##### Cpu0SEInstrInfo.cpp / Cpu0SEInstrInfo.h

`InstrInfo` 提供指令操作接口，主要包括：

- 加载/存储指令的查询
- 分支分析和替换
- 寄存器拷贝（`copyPhysReg`）

这一步里实现的是骨架版本，足以通过编译；具体指令操作等到有完整 ISA 后再补全。

为什么必须建：

- `Subtarget::getInstrInfo()` 必须返回合法指针
- `CodeGen` 里有大量 pass 通过 `TII` 操作指令，空指针会直接崩溃

##### Cpu0ISelLowering.cpp / Cpu0ISelLowering.h
##### Cpu0SEISelLowering.cpp / Cpu0SEISelLowering.h

`ISelLowering` 负责把 LLVM IR 里的高层操作（函数调用、返回、全局地址访问等）
降到 SelectionDAG 节点，供后续 DAG-to-DAG 选择器匹配。

这一步实现了：

- `LowerFormalArguments()`：把入参从寄存器 / 栈帧拿出来，接入 DAG
- `LowerReturn()`：生成 `ret` 指令的 DAG 序列，带 CC glue chain
- `Cpu0ISD` 自定义节点类型（`JmpLink`、`Ret`、`Hi`/`Lo`、`GPRel` 等）
- 调用约定通过 `Cpu0GenCallingConv.inc`（由 `Cpu0CallingConv.td` 生成）驱动

为什么必须建：

- 没有 `LowerFormalArguments`，函数参数无法进入 DAG，最简单的函数都无法编译
- 没有 `LowerReturn`，返回序列无法生成，函数末尾会产生非法 IR

##### Cpu0ISelDAGToDAG.cpp / Cpu0ISelDAGToDAG.h

`ISelDAGToDAG` 是 LLVM 的 DAG-to-DAG 选择器，负责把 SelectionDAG 节点
匹配成具体的 Cpu0 机器指令。

这一步实现：

- `Select()`：遍历 DAG 节点，先尝试 TableGen 生成的模式，再做手动匹配
- `SelectAddr()`：`ComplexPattern` 回调，用于地址模式识别
- `INITIALIZE_PASS` 和 `createCpu0ISelDag()` 工厂函数

为什么必须建：

- `PassConfig::addInstSelector()` 需要注册这个 pass
- 没有 selector，整个指令选择阶段是空的，IR 无法转换成机器指令
- `Cpu0GenDAGISel.inc`（由 TableGen 生成）依赖 `SelectAddr` 方法存在于类中

##### Cpu0FrameLowering.cpp / Cpu0FrameLowering.h
##### Cpu0SEFrameLowering.cpp / Cpu0SEFrameLowering.h

`FrameLowering` 负责函数 prologue / epilogue 的生成，即：

- 进入函数时分配栈帧（`emitPrologue`）
- 退出函数时释放栈帧（`emitEpilogue`）
- 查询栈对齐、帧布局规则

这一步实现骨架版本（prologue/epilogue 为空），足以通过编译。

为什么必须建：

- `Subtarget::getFrameLowering()` 必须返回合法指针
- 没有 `FrameLowering`，函数不能有局部变量（帧分配 pass 无法运行）

##### Cpu0MachineFunction.cpp / Cpu0MachineFunction.h

`MachineFunctionInfo` 是 per-function 的后端私有数据容器，用于在不同 pass 之间
传递函数级的后端状态，比如：

- 是否用到了 FP 寄存器
- 函数是否有可变参数区域
- 局部变量的帧槽分配

这一步提供最小骨架（空实现），足以让 `createMachineFunctionInfo()` 正常返回。

##### Cpu0TargetObjectFile.cpp / Cpu0TargetObjectFile.h

`TargetObjectFile` 封装目标文件的 section 创建逻辑，比如：

- 把全局变量分配到 `.data`、`.bss`、`.rodata`、`.sdata` 等正确的 section

这一步继承 `TargetLoweringObjectFileELF`，使用默认 ELF section 映射规则。

为什么必须建：

- `TargetMachine::getObjFileLowering()` 必须返回合法指针
- 没有它，汇编/对象文件输出阶段无法确定 section 归属

##### MCTargetDesc/Cpu0ABIInfo.cpp / Cpu0ABIInfo.h

`ABIInfo` 封装调用约定相关的 ABI 决策，比如：

- 哪些寄存器用于参数传递
- 整数 / 指针 / 浮点类型如何映射到寄存器

`Cpu0ABIInfo` 实现 O32 调用约定，通过 `Cpu0GenCallingConv.inc` 驱动参数分配。

`ISelLowering` 持有 `ABIInfo` 引用，在 `LowerFormalArguments` 和 `LowerReturn`
里使用它来访问参数寄存器列表。

##### MCTargetDesc/Cpu0MCAsmInfo.cpp / Cpu0MCAsmInfo.h

`MCAsmInfo` 描述汇编输出格式，包括：

- 注释字符、标签分隔符
- 数据指令前缀（`.byte`、`.word` 等）
- 调试信息格式

这一步继承 `MCAsmInfoELF`，使用标准 ELF 汇编风格。

为什么必须建：

- `LLVMInitializeCpu0TargetMC()` 里必须通过 `RegisterMCAsmInfo` 注册它
- 没有它，工具在初始化 MC 层时访问 `MCAsmInfo` 指针会触发 null dereference
- 这正是之前 `llc -march=cpu0 -mcpu=help` 触发 segfault 的根本原因

---

#### 修改文件（15 个）

| 文件 | 修改内容 |
|---|---|
| `Cpu0TargetMachine.cpp` | 接入 `Cpu0Subtarget`；实现 `getSubtargetImpl()`；补全 `Cpu0PassConfig::addInstSelector()` 注册 `ISelDAGToDAG` pass |
| `Cpu0TargetMachine.h` | 声明 `Subtarget` 成员；添加 `getSubtargetImpl()` |
| `Cpu0.h` | 添加 `createCpu0ISelDag()` 工厂函数前向声明；添加 `initializeCpu0DAGToDAGISelLegacyPass()` 前向声明 |
| `Cpu0CallingConv.td` | 增加 `CC_Cpu0_I32` 等调用约定规则（整数参数寄存器分配、栈传参规则） |
| `CMakeLists.txt` | 把所有新 `.cpp` 文件加入 `LLVMCpu0CodeGen` 构建目标 |
| `MCTargetDesc/CMakeLists.txt` | 把 `Cpu0ABIInfo.cpp`、`Cpu0MCAsmInfo.cpp` 加入 `LLVMCpu0Desc` |
| `MCTargetDesc/Cpu0MCTargetDesc.cpp` | 补全 TableGen `.inc` include（`MC_DESC`、`MC_REG_INFO`、`MC_SUBTARGET_INFO` 等 define guard 版本）；注册 `MCAsmInfo` |

---

### 如何验证这一步的修改

#### 验证 1：构建成功

```bash
# 在 build 目录下
cmake --build . --target LLVMCpu0CodeGen LLVMCpu0Desc LLVMCpu0Info -- -j$(nproc)
```

期望结果：三个静态库编译链接全部通过，无错误无警告（除已知的 TableGen 旧式警告）。

#### 验证 2：`llc --version` 能列出 Cpu0

```bash
./bin/llc --version 2>&1 | grep -i cpu0
```

期望结果：

```
Registered Targets:
  cpu0   - Cpu0 (Big Endian)
  cpu0el - Cpu0 (Little Endian)
```

#### 验证 3：`-mcpu=help` 不再崩溃

```bash
./bin/llc -march=cpu0 -mcpu=help 2>&1 | head -5
```

期望结果：打印可用 CPU 列表（`cpu032I`、`cpu032II` 等），不触发 segfault。
这一点直接验证了 `Cpu0MCAsmInfo` 被正确注册。

#### 验证 4：最小 IR 能通过 codegen 流程

```bash
cat > /tmp/test_ret.ll << 'EOF'
define i32 @test() {
  ret i32 0
}
EOF
./bin/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic \
    /tmp/test_ret.ll -o /tmp/test_ret.s 2>&1
cat /tmp/test_ret.s
```

期望结果：产出合法的 Cpu0 汇编输出，不崩溃。

---

### 这一步完成后，Cpu0 backend 的状态

| 能力 | 状态 |
|---|---|
| LLVM 能识别 `cpu0` / `cpu0el` triple | ✅ |
| `-mcpu=help` 不崩溃 | ✅ |
| `Subtarget` 聚合所有后端组件 | ✅ |
| `RegisterInfo` 含帧索引消除 | ✅（骨架） |
| `InstrInfo` / `ISelLowering` / `FrameLowering` | ✅（骨架） |
| `ISelDAGToDAG` pass 注册并接入 pipeline | ✅ |
| `LowerFormalArguments` / `LowerReturn` 基于 CC | ✅ |
| `MCAsmInfo` 注册，MC 层初始化完整 | ✅ |
| 能编译含 `ret` 的最小函数 | ✅ |
| 完整 ISA（算术、Load/Store、分支、调用） | ❌（下一章） |
| prologue/epilogue 生成 | ❌（框架已在，实现待填） |
| AsmPrinter / 对象文件输出 | ❌（下一章） |

### 用一句话总结

**这一步把 Cpu0 从”只有注册入口的空壳”推进成了”有完整 C++ 类层次结构、能跑通最小 codegen 流程的 backend skeleton”，核心贡献是补全 Subtarget 聚合器、ISelDAGToDAG pass 接入、调用约定驱动的参数/返回值降低，以及修复因缺少 MCAsmInfo 导致的 MC 层初始化崩溃。**

---

## Stage 6 — 3.2：AsmPrinter 与函数返回——修复三类级联崩溃

### 这一步的目的

3.1 把完整后端 C++ 类层次建立起来。运行
`llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc` 仍然失败，
并且是三类错误级联出现——每修一个才暴露下一个：

1. **”Cannot select: Cpu0ISD::Ret”**
   SelectionDAG 里没有把 `Cpu0ISD::Ret` 节点匹配到机器指令的 TableGen 模式。

2. **”Cannot select: i32 = FrameIndex\<0\>”**
   `SelectAddr` 没有把 `FrameIndexSDNode` 转成 `TargetFrameIndex`，
   DAG legalizer 找不到能处理裸帧索引节点的模式。

3. **SIGSEGV in `Cpu0AsmPrinter::emitFunctionBodyStart`**
   `Cpu0FI`（per-function 后端数据指针）为 null。LLVM 20 中
   `MachineFunction::getInfo<T>()` 做的是无检查的 `static_cast`，不会自动构造 `T`；
   目标必须自行重写 `TargetMachine::createMachineFunctionInfo`。

这一步（对应教程 Section 3.2 — AsmPrinter）的核心任务是依次修复上述三类崩溃，
让 `llc` 以退出码 0 输出包含 `ret` 指令的合法 Cpu0 汇编。

---

### 这一步修改了哪些文件，为什么修改

#### `Cpu0InstrInfo.td`

**问题**：`Cpu0Ret` SDNode 使用 `SDTNone` 作为类型描述（零操作数）。
`RET` 机器指令需要一个寄存器操作数（`$lr`）。
因为节点和指令的操作数签名不匹配，TableGen 无法生成匹配规则。

**修改内容**：

1. 新增 `SDT_Cpu0Ret` 类型描述，声明 1 个整数操作数：
   ```tablegen
   def SDT_Cpu0Ret : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
   ```
2. 把 `Cpu0Ret` 从 `SDTNone` 改为 `SDT_Cpu0Ret`：
   ```tablegen
   def Cpu0Ret : SDNode<”Cpu0ISD::Ret”, SDT_Cpu0Ret,
                        [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
   ```
3. 在 `RET` 指令定义之后加上 `Pat<>` 规则：
   ```tablegen
   def : Pat<(Cpu0Ret GPROut:$ra), (RET GPROut:$ra)>;
   ```

#### `Cpu0ISelLowering.cpp`

**问题**：`LowerReturn` 把返回值寄存器（如 `$v0`）直接推进 `RetOps`，
导致 `Cpu0ISD::Ret` 节点携带了两个寄存器操作数，与 `SDT_Cpu0Ret`（1 个操作数）不符。

**修改内容**：
在返回值循环里只做 `getCopyToReg`，不再把返回值寄存器推入 `RetOps`；
只将 `LR` 寄存器和 glue token 加入 `RetOps`：

```cpp
for (unsigned I = 0; I < RVLocs.size(); ++I) {
    const CCValAssign &VA = RVLocs[I];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[I], Glue);
    Glue = Chain.getValue(1);
    // 返回值寄存器不推入 RetOps
}
RetOps[0] = Chain;
RetOps.push_back(DAG.getRegister(Cpu0::LR, MVT::i32));
if (Glue.getNode())
    RetOps.push_back(Glue);
return DAG.getNode(Cpu0ISD::Ret, DL, MVT::Other, RetOps);
```

#### `Cpu0ISelDAGToDAG.cpp`

**问题**：`SelectAddr` 对所有输入都走 `Base = N` 这条通用路径，
包括 `FrameIndexSDNode`。`FrameIndex` 是 RA 前的占位符，
必须先转成 `TargetFrameIndex`，指令选择器才能匹配 load/store 模式。

**修改内容**：
新增对 `FrameIndexSDNode` 的识别分支：

```cpp
if (FrameIndexSDNode *FI = dyn_cast<FrameIndexSDNode>(N)) {
    Base   = CurDAG->getTargetFrameIndex(FI->getIndex(), ValTy);
    Offset = CurDAG->getTargetConstant(0, DL, ValTy);
    return true;
}
```

#### `Cpu0MachineFunction.h`

**问题**：`Cpu0FunctionInfo` 的构造函数签名是旧版形式，
与 LLVM 20 `MachineFunctionInfo::create<T>` 工厂要求的签名不符。

**修改内容**：
把构造函数更新为 LLVM 20 工厂 API 签名：

```cpp
explicit Cpu0FunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}
```

#### `Cpu0TargetMachine.h`

**问题**：`Cpu0TargetMachine` 没有重写 `createMachineFunctionInfo`，
所以 LLVM 永远不会为 Cpu0 函数分配 `Cpu0FunctionInfo`，
`Cpu0AsmPrinter` 中的 `Cpu0FI` 指针始终为 null。

**修改内容**：
声明 override：

```cpp
MachineFunctionInfo *
createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                          const TargetSubtargetInfo *STI) const override;
```

#### `Cpu0TargetMachine.cpp`

**修改内容**：
用 LLVM 的 placement-new 工厂模式实现该方法（LLVM 用 `BumpPtrAllocator` 分配，
避免每个函数单独 `new`）：

```cpp
MachineFunctionInfo *
Cpu0TargetMachine::createMachineFunctionInfo(BumpPtrAllocator &Allocator,
                                              const Function &F,
                                              const TargetSubtargetInfo *STI) const {
    return Cpu0FunctionInfo::create<Cpu0FunctionInfo>(Allocator, F, STI);
}
```

有了这个方法，LLVM 在构造每个 `MachineFunction` 时就会调用它，
确保 `Cpu0FI` 在 `emitFunctionBodyStart` 被调用前始终非 null。

#### `Cpu0AsmPrinter.cpp`

新增 `Cpu0AsmPrinter` 类的完整实现，包括：

- `runOnMachineFunction`：从 `MachineFunction` 获取 `Cpu0FI`
- `emitFunctionBodyStart` / `emitFunctionBodyEnd`：输出 `.frame`、`.mask`、
  `.set noreorder` / `.set macro` 等 Cpu0 汇编指令
- `emitInstruction`：通过 `MCInstLowering` 把 `MachineInstr` 转换并输出到流
- `emitStartOfAsmFile`：输出 `.section .mdebug.*` 和 `.previous`

由于 `createMachineFunctionInfo` 已经就位，`Cpu0FI` 保证非 null，
`emitFunctionBodyStart` 和 `emitFunctionBodyEnd` 中对 `Cpu0FI->getEmitNOAT()` 的访问不再崩溃。

---

### 如何验证这一步的修改

```bash
# 1. 重新构建（在 build/ 目录下）
cmake --build . --target LLVMCpu0CodeGen -- -j$(nproc)
cmake --build . --target llc -- -j$(nproc)

# 2. 编译 ch3.bc 到 Cpu0 汇编
./bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o /tmp/ch3.cpu0.s
echo “Exit: $?”   # 期望：0

# 3. 确认 ret 指令存在
grep 'ret' /tmp/ch3.cpu0.s
# 期望输出包含：ret $lr

# 4. 查看完整输出
cat /tmp/ch3.cpu0.s
# 期望大致输出：
#   main:
#       .frame  $fp,8,$lr
#       .mask   0x00000000,0
#       .set    noreorder
#       .set    nomacro
#   # %bb.0:
#       addiu   $2, $zero, 0
#       st      $2, 0($fp)
#       ret     $lr
#       .set    macro
#       .set    reorder
#       .end    main
```

---

### 这一步完成后，Cpu0 backend 的状态

| 能力 | 状态 |
|---|---|
| `Cpu0ISD::Ret` 能选择为 `RET $lr` | ✅ |
| `FrameIndex` 节点在 `SelectAddr` 中正确降低为 `TargetFrameIndex` | ✅ |
| `Cpu0FunctionInfo` 通过 `createMachineFunctionInfo` 分配（LLVM 20 API） | ✅ |
| `Cpu0AsmPrinter` 能输出 `.frame` / `.mask` / `.set` 等指令 | ✅ |
| `llc` 对最小 `int main() { return 0; }` 以退出码 0 输出汇编 | ✅ |
| 完整 ISA（算术、Load/Store、分支、调用） | ❌（下一章） |
| 非平凡栈帧的 prologue/epilogue | ❌（下一章） |
| 对象文件输出（`-filetype=obj`） | ❌（下一章） |

### 用一句话总结

**这一步把 Cpu0 从”后端骨架已在但 codegen 末尾全部崩溃”推进到”能端到端编译一个最小 C 函数并输出含 `ret $lr` 的合法 Cpu0 汇编”，核心是修复 TableGen 类型描述不匹配、FrameIndex 未转换、MachineFunctionInfo 工厂缺失这三个级联问题。**

---

## 截至 3.2，当前后端状态

### 已具备的能力

- LLVM 能识别 `cpu0` / `cpu0el` / `cpu0eb`，知道其 ELF machine id、data layout 和 relocation 命名
- Cpu0 已进入 LLVM experimental target 列表，三个静态库（Info / Desc / CodeGen）可被正常构建
- `Subtarget` 聚合了 RegisterInfo、InstrInfo、FrameLowering、ISelLowering 的完整骨架
- `ISelDAGToDAG` pass 已接入 codegen pipeline，`SelectAddr` 能正确处理 FrameIndex 节点
- `LowerFormalArguments` / `LowerReturn` 基于调用约定驱动，最小函数能完整走通 codegen
- `MCAsmInfo` 已注册，MC 层初始化不崩溃
- `createMachineFunctionInfo` 已实现（LLVM 20 API），`Cpu0FunctionInfo` 在每个函数中保证非 null
- `llc -march=cpu0` 对 `int main() { return 0; }` 以退出码 0 输出包含 `ret $lr` 的合法 Cpu0 汇编

### 还未完成的部分

| 功能 | 对应章节 |
|------|---------|
| 完整 ISA（算术、Load/Store、分支、函数调用） | Chapter 4 |
| 对象文件输出（`-filetype=obj`） | Chapter 5 |
| 全局变量支持 | Chapter 6 |
| 其他数据类型 | Chapter 7 |
| 控制流语句 | Chapter 8 |
| 函数调用完整实现 | Chapter 9 |
| ELF 完整支持 | Chapter 10 |
| 汇编器 / 反汇编器 | Chapter 11 |
| C++ 支持 | Chapter 12 |
| 非平凡栈帧的 prologue / epilogue | Chapter 3.5+ |

### 用一句话总结当前整体状态

**Cpu0 backend 已经从一个教程里的概念目标，走到了一个 LLVM 能识别、能构建、能用 llc 端到端输出最小合法汇编的 experimental backend skeleton；接下来的工作是逐章填充真实 ISA、完整 ABI、对象文件输出和汇编器支持。**

---

## Stage 7 — 3.3–3.5：栈帧实现、寄存器溢出恢复与 Subtarget 工具方法

### 对应提交

- Commit: `f6a0a213667d`
- Subject: `Update 3.3 3.4 3.5`
- AuthorDate: `2026-04-28 06:14:36 -0400`

这次提交一共改了 6 个文件，章节门控从 `CH3_2` 推进到 `CH3_5`。

### 这一步做了什么

这一步补全了三类此前仍为骨架的核心组件：

| 章节 | 核心改动 | 涉及文件 |
|---|---|---|
| 3.3/3.4 | 实现 `emitPrologue` / `emitEpilogue`，补全栈帧分配与释放 | `Cpu0SEFrameLowering.cpp` |
| 3.3/3.4 | 实现 `storeRegToStackSlot` / `loadRegFromStackSlot`，补全 callee-saved 溢出恢复 | `Cpu0SEInstrInfo.cpp/.h` |
| 3.5     | 新增 `getCpu0Subtarget()` helper（PassConfig 层）和 `isLittleEndian()` 访问器 | `Cpu0TargetMachine.cpp/.h` |

---

### `Cpu0SEFrameLowering.cpp` — emitPrologue / emitEpilogue

**问题**：3.1 时 `emitPrologue` / `emitEpilogue` 均为空实现，函数调用会跳过栈帧分配，局部变量的帧槽偏移计算会出错。

**修改内容**：

`emitPrologue` 通过循环发出 `ADDiu SP, SP, -chunk` 指令来分配栈帧：

```cpp
uint64_t Remaining = StackSize;
while (Remaining > 0) {
    uint64_t Chunk = Remaining > 32767 ? 32767 : Remaining;
    BuildMI(MBB, MBBI, DL, TII.get(Cpu0::ADDiu), Cpu0::SP)
        .addReg(Cpu0::SP)
        .addImm(-(int64_t)Chunk);
    Remaining -= Chunk;
}
```

`emitEpilogue` 对称地发出 `ADDiu SP, SP, +chunk`：

```cpp
while (Remaining > 0) {
    uint64_t Chunk = Remaining > 32767 ? 32767 : Remaining;
    BuildMI(MBB, MBBI, DL, TII.get(Cpu0::ADDiu), Cpu0::SP)
        .addReg(Cpu0::SP)
        .addImm((int64_t)Chunk);
    Remaining -= Chunk;
}
```

**为什么用 32767 字节分块**：

Cpu0 的 `ADDiu` 指令使用 16 位有符号立即数（范围 −32768 到 +32767）。当栈帧大于 32767 字节时，单条 `ADDiu` 无法表达完整偏移，必须分多次累加。真正的生产后端通常用 `LUi + ORi + ADDU` 实现任意偏移，但 Cpu0 目前 ISA 中没有 `LUi`，因此改用多段 `ADDiu` 逼近。

---

### `Cpu0SEInstrInfo.cpp/.h` — storeRegToStackSlot / loadRegFromStackSlot

**问题**：callee-saved 寄存器溢出/恢复需要目标后端提供这两个方法；3.1 时它们未声明也未实现，寄存器分配器无法在调用保存寄存器时发出正确的 spill/fill 指令。

**修改内容**：

`storeRegToStackSlot` 使用 `Cpu0::ST` 指令将寄存器写入帧槽：

```cpp
BuildMI(MBB, I, DL, get(Cpu0::ST))
    .addReg(SrcReg, getKillRegState(IsKill))
    .addFrameIndex(FI)
    .addImm(0)
    .addMemOperand(MMO);
```

`loadRegFromStackSlot` 使用 `Cpu0::LD` 指令从帧槽读回寄存器：

```cpp
BuildMI(MBB, I, DL, get(Cpu0::LD), DestReg)
    .addFrameIndex(FI)
    .addImm(0)
    .addMemOperand(MMO);
```

两者都通过 `MachineMemOperand` 携带访存元信息（大小、对齐），确保后端调度器和别名分析能正确处理这些帧操作。LLVM 20 的方法签名在这两处各增加了一个 `Register VReg` 参数（用于 GlobalISel 互操作），声明时须与基类签名严格匹配。

---

### `Cpu0TargetMachine.cpp/.h` — getCpu0Subtarget() 与 isLittleEndian()

**修改内容**：

在 `Cpu0PassConfig` 类中新增 `getCpu0Subtarget()` 方法，把 `TargetMachine → Cpu0TargetMachine → getSubtargetImpl()` 的链式调用封装为单一入口：

```cpp
const Cpu0Subtarget &getCpu0Subtarget() const {
    return *getCpu0TargetMachine().getSubtargetImpl();
}
```

在 `Cpu0TargetMachine` 中新增 `isLittleEndian()` 访问器，委托给 `DefaultSubtarget.isLittle()`：

```cpp
bool isLittleEndian() const { return DefaultSubtarget.isLittle(); }
```

后续章节的 pass 和 lowering 代码需要频繁查询 subtarget 特性。有了这两个工具方法，调用点不需要重复写类型转换链，也与后续章节教程里的调用模式对齐。

---

### 章节门控推进

`Cpu0SetChapter.h` 从 `CH3_2` 改为 `CH3_5`，开启更多指令和功能的条件编译块，让后端能处理更复杂的测试用例。

---

### 这一步完成后，Cpu0 backend 的状态

| 能力 | 状态 |
|---|---|
| `emitPrologue`：ADDiu 分块分配栈帧 | ✅ |
| `emitEpilogue`：ADDiu 分块释放栈帧 | ✅ |
| `storeRegToStackSlot`：callee-saved 寄存器溢出 | ✅ |
| `loadRegFromStackSlot`：callee-saved 寄存器恢复 | ✅ |
| `getCpu0Subtarget()` / `isLittleEndian()` 工具方法 | ✅ |
| 章节门控推进到 CH3_5 | ✅ |
| 大帧（>32767 字节）分多段分配 | ✅（ADDiu 循环） |
| 大帧：最优单指令实现（LUi+ORi+ADDU） | ❌（ISA 不含 LUi） |
| 完整 ISA（算术、分支、函数调用） | ❌（Chapter 4+） |
| 对象文件输出（`-filetype=obj`） | ❌（Chapter 5） |

### 用一句话总结

**这一步把 Cpu0 从"有完整类层次但 prologue/epilogue 为空、无法溢出寄存器"推进成"能正确分配和释放栈帧、能在寄存器不足时溢出/恢复 callee-saved 寄存器"的功能性后端骨架，同时补全了 subtarget 访问工具方法，为后续章节的 pass 开发铺路。**

---

## Stage 8 — 4.1：算术指令集——完整 ISA、乘除法与 DAG Combine

### 对应提交

- Commit: `c6862d06a91c8f85fd910a0fc1691973eb632238`
- Subject: `Update 4.1`
- AuthorDate: `2026-04-29 07:52:23 -0400`

这次提交一共改了 12 个文件：

- 11 个功能文件
- 1 个测试文档（`test/ch4_1_test.md`）

### 这一步做了什么

这一步把 Cpu0 backend 从"只能编译 `return 0`"推进成"能编译完整 C 算术表达式（含乘法、除法、取余）"的真正功能性后端。

| 章节 | 核心改动 | 涉及文件 |
|---|---|---|
| 寄存器 | 新增 HI / LO 寄存器和 HILO 寄存器类 | `Cpu0RegisterInfo.td` |
| 调度 | 新增 IIHiLo / IImul / IIIdiv itinerary，接入 IMULDIV 功能单元 | `Cpu0Schedule.td` |
| 指令定义 | 新增 ~35 条指令，含算术、逻辑、移位、旋转、乘除、HI/LO 搬移、C0 搬移 | `Cpu0InstrInfo.td` |
| 立即数合成 | 新增 HI16/LO16 XForm、immLow16Zero/immZExt16/immZExt5 PatLeaf 和 32 位常量 Pat 规则 | `Cpu0InstrInfo.td` |
| Subtarget | 新增 `EnableOverflowOpt` 命令行选项，控制 ADD/SUB vs ADDu/SUBu | `Cpu0Subtarget.cpp/.h` |
| 除法降低 | `performDivRemCombine` 将 SDIVREM/UDIVREM 替换为 Cpu0ISD::DivRem + getCopyFromReg | `Cpu0SEISelLowering.cpp/.h` |
| 乘法选择 | `selectMULT` 手动展开 ISD::MULHS/MULHU → MULT/MULTu + MFHI | `Cpu0ISelDAGToDAG.cpp/.h` |
| 物理寄存器拷贝 | `copyPhysReg` 处理 GPR↔HI/LO 的各方向拷贝 | `Cpu0SEInstrInfo.cpp/.h` |

---

### 关键设计决策

#### HI/LO 寄存器命名

实现使用 `"hi"`/`"lo"` 作为汇编名称（encoding 0/1），而非教程的 `"ac0"` 双重命名。
这样在汇编输出中更清晰，与 MIPS 后端的习惯也一致。

#### selectMULT 设计：void 而非 pair 返回

教程版本返回 `std::pair<SDNode*, SDNode*>` 并在 `Select()` 调用方做 `ReplaceNode`。
实际实现将 `ReplaceNode` 移入 `selectMULT` 内部，返回 void，调用点更简洁。
参数顺序为 `(HasHi, HasLo)`（教程为 `HasLo, HasHi`）。

#### PerformDAGCombine 放在 SE 子类

教程将 `PerformDAGCombine` 声明在 `Cpu0TargetLowering`（基类）。
实际实现放在 `Cpu0SETargetLowering`（SE 子类），因为 `setTargetDAGCombine` 注册也在
SE 子类构造函数里，两者内聚在同一个文件。

#### copyPhysReg：LLVM 20 扩展签名

LLVM 20 在原有 `(MBB, I, DL, DestReg, SrcReg, KillSrc)` 基础上新增了
`RenamableDest` 和 `RenamableSrc` 两个布尔参数（用于 GlobalISel 互操作）。
声明时须使用 `Register`（非 `MCRegister`）并带默认值 `false`。

#### setTargetDAGCombine：ArrayRef API

LLVM 20 的 `setTargetDAGCombine` 接受 `ArrayRef<unsigned>`，可用 initializer list
一次注册多个操作码：`setTargetDAGCombine({ISD::SDIVREM, ISD::UDIVREM})`。
教程里的两次单独调用在新 API 下仍然合法但冗余。

#### AND/OR/XOR/NOR 以及 SLT/CMP 提前加入

这批指令在教程中属于后续章节，但与 Ch4_1 算术指令在同一 TD 块中定义更自然，
且它们的 isel pattern 不依赖任何尚未实现的基础设施。
`NOR` 通过 `Pat<(not (or ...))>` 匹配，`SLT` 系列受 `HasSlt` feature 门控，
`CMP` 受 `HasCmp` 门控（cpu032I 子目标）。

---

### 这一步修改的 11 个功能文件

| 文件 | 改动摘要 |
|---|---|
| `Cpu0RegisterInfo.td` | 新增 HI / LO 寄存器定义（encoding 0/1，名称 "hi"/"lo"）；新增 HILO 寄存器类 |
| `Cpu0Schedule.td` | 新增 IIHiLo（1 cycle）、IImul（17 cycles）、IIIdiv（38 cycles）三个 itinerary，更新 Cpu0GenericItineraries |
| `Cpu0InstrInfo.td` | 新增 SDT_Cpu0DivRem / Cpu0DivRem / Cpu0DivRemU SDNode；新增 immLow16Zero / immZExt16 / uimm16 / uimm5 / immZExt5 / HI16 / LO16；新增 ArithLogicR / ShiftRotateI / ShiftRotateR / Mult32 / Div32 / MoveFromLOHI / MoveToLOHI / MoveToC0 / MoveFromC0 / C0Move 指令类；新增 ~35 条指令 def；新增 32 位立即数合成 Pat 规则 |
| `Cpu0Subtarget.cpp` | 新增 `EnableOverflowOpt` cl::opt；在 `initializeSubtargetDependencies` 中赋值给 `EnableOverflow` |
| `Cpu0Subtarget.h` | 新增 `hasChapter3_5()` / `hasChapter4_1()` 访问器（均读 HasChapterDummy） |
| `Cpu0SEISelLowering.cpp` | 新增 `performDivRemCombine` 静态函数；新增 `PerformDAGCombine` 方法；constructor 新增 MULHS/MULHU/SDIV/SREM/UDIV/UREM 操作码动作设置和 DAG combine 注册 |
| `Cpu0SEISelLowering.h` | 声明 `PerformDAGCombine` override |
| `Cpu0SEInstrInfo.cpp` | 实现 `copyPhysReg`（GPR↔GPR via ADDu；GPR↔HI/LO via MFHI/MFLO/MTHI/MTLO） |
| `Cpu0SEInstrInfo.h` | 声明 `copyPhysReg` override（LLVM 20 签名：Register + RenamableDest/Src） |
| `Cpu0ISelDAGToDAG.cpp` | 实现 `selectMULT`（void 返回，内部调用 ReplaceNode）；在 `Select()` 中处理 ISD::MULHS / ISD::MULHU |
| `Cpu0ISelDAGToDAG.h` | 声明 `selectMULT` |

---

### 这一步完成后，Cpu0 backend 的状态

| 能力 | 状态 |
|---|---|
| HI / LO 寄存器和 HILO 寄存器类 | ✅ |
| 算术指令（ADDu, SUBu, ADD, SUB, MUL, ADDiu, ORi, LUi） | ✅ |
| 逻辑指令（AND, OR, XOR, NOR） | ✅ |
| 移位 / 旋转指令（SHL, SHR, SRA, ROL, ROR + 寄存器版本） | ✅ |
| 64 位乘法（MULT/MULTu → HI:LO，MFHI/MFLO 提取） | ✅ |
| 有符号 / 无符号除法（SDIV/UDIV → HI:LO，DAG combine 接入） | ✅ |
| GPR ↔ HI/LO 物理寄存器拷贝（copyPhysReg） | ✅ |
| 32 位立即数合成（LUi + ORi 两指令序列） | ✅ |
| EnableOverflow 命令行选项（ADD/SUB vs ADDu/SUBu） | ✅ |
| SLT / SLTu / SLTi / SLTiu（cpu032II，HasSlt 门控） | ✅ |
| CMP（cpu032I，HasCmp 门控） | ✅ |
| 对象文件输出（`-filetype=obj`） | ❌（Chapter 5） |
| 全局变量支持 | ❌（Chapter 6） |
| 控制流语句（if / while / for） | ❌（Chapter 8） |
| 函数调用完整实现 | ❌（Chapter 9） |

### 用一句话总结

**这一步把 Cpu0 从"只能编译最小返回函数"推进成"能编译完整 C 算术表达式（含乘、除、取余、移位）的功能性后端"，核心是建立 HI/LO 寄存器模型、实现 DAG combine 驱动的除法降低、手动展开 MULHS/MULHU 到 MULT+MFHI，以及补全 copyPhysReg 支持 GPR↔HI/LO 拷贝。**

---

## 截至 4.1，当前后端状态

### 已具备的能力

- LLVM 能识别 `cpu0` / `cpu0el` / `cpu0eb`，知道其 ELF machine id、data layout 和 relocation 命名
- Cpu0 已进入 LLVM experimental target 列表，三个静态库（Info / Desc / CodeGen）可被正常构建
- `Subtarget` 聚合了 RegisterInfo、InstrInfo、FrameLowering、ISelLowering 的完整骨架
- `ISelDAGToDAG` pass 已接入 codegen pipeline，`SelectAddr` 能正确处理 FrameIndex 节点
- `LowerFormalArguments` / `LowerReturn` 基于调用约定驱动，最小函数能完整走通 codegen
- `MCAsmInfo` 已注册，MC 层初始化不崩溃
- `createMachineFunctionInfo` 已实现（LLVM 20 API），`Cpu0FunctionInfo` 在每个函数中保证非 null
- `emitPrologue` / `emitEpilogue` 已实现，能通过分块 ADDiu 正确分配和释放栈帧
- `storeRegToStackSlot` / `loadRegFromStackSlot` 已实现，寄存器分配器能溢出/恢复 callee-saved 寄存器
- 完整算术指令集（算术、逻辑、移位/旋转、乘法、除法）已实现，`llc -march=cpu0` 能编译 C 算术表达式
- 32 位立即数合成（LUi + ORi）已实现，任意常量可在两条指令内加载

### 还未完成的部分

| 功能 | 对应章节 |
|------|---------|
| 对象文件输出（`-filetype=obj`） | Chapter 5 |
| 全局变量支持 | Chapter 6 |
| 其他数据类型 | Chapter 7 |
| 控制流语句 | Chapter 8 |
| 函数调用完整实现 | Chapter 9 |
| ELF 完整支持 | Chapter 10 |
| 汇编器 / 反汇编器 | Chapter 11 |
| C++ 支持 | Chapter 12 |
| 大帧最优实现（LUi+ORi+ADDU 替代分块 ADDiu） | 待定 |

### 用一句话总结当前整体状态

**Cpu0 backend 已经具备完整的算术指令集和栈帧管理能力，能用 `llc -march=cpu0` 编译含乘除法的 C 算术函数；接下来的工作是补全对象文件输出、全局变量支持和控制流语句。**

---

## Stage 9 — 4.2：逻辑指令与比较指令——setcc 完整实现

### 对应提交

- Commit: `b51a4c6855a825a0161c72fe0c9dca81e172aa28`
- Subject: `Update 4.2`
- AuthorDate: `2026-04-30 00:54:37 -0400`

这次提交一共改了 6 个文件：

- 4 个功能文件
- 1 个开发文档（`dev_docs/Cpu0_Section4_2.md`）
- 1 个测试文档（`test/ch4_2_test.md`）

### 这一步做了什么

这一步把 Cpu0 backend 从"能编译算术表达式"推进成"能编译包含逻辑运算符和比较运算符（`&`、`|`、`^`、`~`、`!`、`==`、`!=`、`<`、`<=`、`>`、`>=`）的 C 表达式"。

| 章节 | 核心改动 | 涉及文件 |
|---|---|---|
| 4.2 | 新增 `CmpInstr`/`LogicNOR`/`SetCC_R`/`SetCC_I` 指令类；AND/OR/XOR/NOR 移至 Ch4_2 门控；新增 ANDi/XORi；SLT/CMP 操作码修正；完整 setcc 多类模式 | `Cpu0InstrInfo.td` |
| 4.2 | `SIGN_EXTEND_INREG` 扩展为 SHL+SRA 对 | `Cpu0SEISelLowering.cpp` |
| 4.2 | 新增 `hasChapter4_2()` 访问器 | `Cpu0Subtarget.h` |
| 4.2 | 新增 `isCpu0CPU()` 辅助函数；`getSubtargetImpl()` 对 per-function CPU 属性做合法性过滤 | `Cpu0TargetMachine.cpp` |

---

### `Cpu0InstrInfo.td` — 新增指令类和 setcc 模式

#### 新增 `CmpInstr` 类（cpu032I，HasCmp）

```tablegen
class CmpInstr<bits<8> op, string instr_asm,
               InstrItinClass itin, RegisterClass RC, RegisterClass RD,
               bit isComm = 0> :
  FA<op, (outs RD:$ra), (ins RC:$rb, RC:$rc), ...>
```

与 `ArithLogicR` 的关键区别：输出寄存器类是 `SR`（状态寄存器类），而非 `GPROut`。比较结果写入 SW（状态字）寄存器，而非通用寄存器。类体内没有 DAG 模式，匹配由 setcc 的 `Pat<>` 规则完成。

两条指令定义在 `[Ch4_2, HasCmp]` 门控下：

| 助记符 | 操作码 | 说明 |
|--------|--------|------|
| `CMP`  | 0x2A   | 有符号比较，写入 SW/SR |
| `CMPu` | 0x2B   | 无符号比较，写入 SW/SR |

> **操作码修正**：Ch4_1 原型中 CMP 错误使用了 0x2c → 修正为 **0x2A**；CMPu（0x2B）在 Ch4_1 中完全缺失，本次新增。

#### 新增 `LogicNOR` 类

```tablegen
class LogicNOR<bits<8> op, string instr_asm, RegisterClass RC> :
  FA<op, ..., [(set RC:$ra, (not (or RC:$rb, RC:$rc)))], IIAlu>
```

把 `not (or ...)` DAG 模式直接内联在类体中，TableGen 自动为该 DAG 形状生成匹配规则，无需外部 `Pat<>` 规则。

#### AND/OR/XOR/NOR 从 Ch4_1 移至 Ch4_2

这批寄存器-寄存器逻辑指令在 Ch4_1 原型中提前定义；本次规范对齐到教程章节结构，统一移入 `[Ch4_2]` 门控。

#### 新增立即数逻辑指令 ANDi / XORi

| 助记符 | 操作码 | IR 操作 | 立即数类型 |
|--------|--------|---------|-----------|
| `ANDi` | 0x0c   | `and`   | `immZExt16`（uimm16） |
| `XORi` | 0x0e   | `xor`   | `immZExt16`（uimm16） |

均复用已有的 `ArithLogicI` 模板。`ORi`（0x0d）在 Ch3_5 中已存在，本次不变。

#### SLT 系列操作码修正

Ch4_1 原型中操作码错误：

| 助记符  | Ch4_1 错误操作码 | Ch4_2 修正操作码 |
|---------|-----------------|-----------------|
| `SLTi`  | 0x2a            | **0x26**        |
| `SLTiu` | 0x2b            | **0x27**        |

`SLT`（0x28）和 `SLTu`（0x29）操作码不变，全部移入 `[Ch4_2, HasSlt]` 门控。

#### `not` 模式从 Ch4_1 移至 Ch4_2

```tablegen
let Predicates = [Ch4_2] in
def : Pat<(not CPURegs:$in), (NOR CPURegs:$in, ZERO)>;
```

处理 C 一元 `~` 运算符（按位取反）。移入 Ch4_2 以匹配章节结构。

#### setcc 多类模式

所有模式定义在 `let Predicates = [Ch4_2] in { ... }` 块内。

##### CMP 变体（cpu032I，HasCmp）

CMP 执行后 SW 位布局：
- Bit 0：负标志（a < b，有符号）
- Bit 1：零标志（a == b）

| 多类 | 覆盖的 setcc 条件 |
|------|-----------------|
| `SeteqPatsCmp` | `seteq`、`setne` |
| `SetltPatsCmp` | `setlt`、`setult` |
| `SetlePatsCmp` | `setle`、`setule` |
| `SetgtPatsCmp` | `setgt`、`setugt` |
| `SetgePatsCmp` | `setge`、`setuge` |

例子——`a == b`（读零标志 bit1，右移 1 得布尔值）：
```tablegen
def : Pat<(seteq RC:$lhs, RC:$rhs),
          (SHR (ANDi (CMP RC:$lhs, RC:$rhs), 2), 1)>;
```

例子——`a <= b`（等价于 `!(b < a)`，读负标志 bit0，XOR 取反）：
```tablegen
def : Pat<(setle RC:$lhs, RC:$rhs),
          (XORi (ANDi (CMP RC:$rhs, RC:$lhs), 1), 1)>;
```

##### SLT 变体（cpu032II，HasSlt）

| 多类 | 覆盖的 setcc 条件 |
|------|-----------------|
| `SeteqPatsSlt`    | `seteq`、`setne` |
| `SetlePatsSlt`    | `setle`、`setule` |
| `SetgtPatsSlt`    | `setgt`、`setugt` |
| `SetgePatsSlt`    | `setge`、`setuge` |
| `SetgeImmPatsSlt` | `setge imm`、`setuge imm` |

例子——`a == b`（通过 XOR+SLTiu 实现）：
```tablegen
def : Pat<(seteq RC:$lhs, RC:$rhs),
          (SLTiu (XOR RC:$lhs, RC:$rhs), 1)>;
```

`SetgeImmPatsSlt` 专门处理与 16 位有符号立即数的比较，利用 `SLTi`/`SLTiu` 直接指令避免额外的寄存器移动。

---

### `Cpu0SEISelLowering.cpp` — SIGN_EXTEND_INREG 展开

```cpp
// Cpu0 has no sext_inreg; expand to shl/sra pairs.
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,    Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8,    Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16,   Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32,   Expand);
setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::Other, Expand);
```

Cpu0 ISA 没有原生的符号扩展指令。若不注册 `Expand`，LLVM 会试图以单条指令发出 `sext_inreg`，无法匹配任何 Cpu0 模式。设置 `Expand` 后，LLVM 自动把 `SIGN_EXTEND_INREG` 分解为 `SHL + SRA` 指令对。

触发场景：C 中对 `i8`/`i16` 类型的比较或赋值操作，编译器需要把窄整数符号扩展到 32 位再做比较。

---

### `Cpu0Subtarget.h` — hasChapter4_2()

```cpp
bool hasChapter4_2() const { return HasChapterDummy; }
```

与 `hasChapter4_1()` 模式完全一致，读取同一个 `HasChapterDummy` 字段。作为章节门控访问器供 pass/lowering 代码使用。

---

### `Cpu0TargetMachine.cpp` — isCpu0CPU() 与 getSubtargetImpl() 修复

#### 新增 `isCpu0CPU()` 辅助函数

```cpp
static bool isCpu0CPU(StringRef CPU) {
  return CPU.empty() || CPU == "generic" || CPU == "cpu032I" ||
         CPU == "cpu032II";
}
```

#### `getSubtargetImpl()` per-function CPU 属性过滤

```cpp
// 旧代码（无条件采用 per-function 属性）：
std::string CPU =
    CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;

// 新代码（只接受合法的 Cpu0 CPU 名）：
std::string CPU = TargetCPU;
if (CPUAttr.isValid()) {
    StringRef FnCPU = CPUAttr.getValueAsString();
    if (isCpu0CPU(FnCPU))
        CPU = FnCPU.str();
}
```

原因：LLVM IR 中的 per-function `"target-cpu"` 属性可能携带来自前端（clang targeting mips）的 CPU 字符串（如 `"mips32r2"`），这对 Cpu0 的 `createSubtargetImpl()` 是非法值，会触发断言。新代码在使用该属性前先校验它是否属于 Cpu0 已知 CPU 列表，否则回退到目标机器级别的 `TargetCPU`。

---

### 如何验证这一步的修改

```bash
# 1. 重新构建
cmake --build . --target LLVMCpu0CodeGen -- -j$(nproc)
cmake --build . --target llc -- -j$(nproc)

# 2. 准备测试输入
cat > /tmp/ch4_2.cpp << 'EOF'
int test_logic(int a, int b) { return (a & b) | (a ^ b); }
int test_cmp(int a, int b)   { return (a == b) + (a < b) + (a > b); }
EOF

# 3. 编译为 LLVM bitcode
clang -O1 -target mips-unknown-linux-gnu -S -emit-llvm /tmp/ch4_2.cpp -o /tmp/ch4_2.ll

# 4. cpu032I（CMP/CMPu 路径）
./bin/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic \
    -filetype=asm /tmp/ch4_2.ll -o -

# 5. cpu032II（SLT 路径）
./bin/llc -march=cpu0 -mcpu=cpu032II -relocation-model=pic \
    -filetype=asm /tmp/ch4_2.ll -o -
```

cpu032I 输出期望包含 `cmp`、`cmpu`、`andi`、`shr`、`xori` 等指令；
cpu032II 输出期望包含 `slt`、`sltu`、`sltiu`、`xor`、`xori` 等指令。

---

### 这一步完成后，Cpu0 backend 的状态

| 能力 | 状态 |
|---|---|
| AND/OR/XOR/NOR 寄存器-寄存器逻辑指令（Ch4_2 门控） | ✅ |
| ANDi / XORi 立即数逻辑指令 | ✅ |
| NOR 通过 `not (or ...)` 模式自动选择 | ✅ |
| `~x` 通过 `NOR x, ZERO` 实现（`not` Pat） | ✅ |
| SLT/SLTu/SLTi/SLTiu（cpu032II，操作码修正） | ✅ |
| CMP/CMPu（cpu032I，操作码修正，CMPu 新增） | ✅ |
| 完整 setcc（`==`/`!=`/`<`/`<=`/`>`/`>=`，两种子目标路径） | ✅ |
| `SIGN_EXTEND_INREG` 展开为 SHL+SRA | ✅ |
| per-function CPU 属性合法性过滤（`isCpu0CPU`） | ✅ |
| 对象文件输出（`-filetype=obj`） | ❌（Chapter 5） |
| 全局变量支持 | ❌（Chapter 6） |
| 控制流语句 | ❌（Chapter 8） |
| 函数调用完整实现 | ❌（Chapter 9） |

### 用一句话总结

**这一步把 Cpu0 从"能编译算术表达式"推进成"能编译包含完整逻辑运算和比较运算的 C 表达式"，核心是引入 CmpInstr/LogicNOR 新指令类、为两种子目标（cpu032I/II）分别实现完整的 setcc 多类 DAG 模式、修正 SLT/CMP 操作码、以及注册 SIGN_EXTEND_INREG 展开以处理窄整数符号扩展。**

---

## 截至 4.2，当前后端状态

### 已具备的能力

- LLVM 能识别 `cpu0` / `cpu0el` / `cpu0eb`，知道其 ELF machine id、data layout 和 relocation 命名
- Cpu0 已进入 LLVM experimental target 列表，三个静态库（Info / Desc / CodeGen）可被正常构建
- `Subtarget` 聚合了 RegisterInfo、InstrInfo、FrameLowering、ISelLowering 的完整骨架
- `ISelDAGToDAG` pass 已接入 codegen pipeline，`SelectAddr` 能正确处理 FrameIndex 节点
- `LowerFormalArguments` / `LowerReturn` 基于调用约定驱动，最小函数能完整走通 codegen
- `MCAsmInfo` 已注册，MC 层初始化不崩溃
- `createMachineFunctionInfo` 已实现（LLVM 20 API），`Cpu0FunctionInfo` 在每个函数中保证非 null
- `emitPrologue` / `emitEpilogue` 已实现，能通过分块 ADDiu 正确分配和释放栈帧
- `storeRegToStackSlot` / `loadRegFromStackSlot` 已实现，寄存器分配器能溢出/恢复 callee-saved 寄存器
- `llc -march=cpu0` 对含局部变量的函数能正确生成包含 prologue/epilogue 的汇编
- HI / LO 寄存器和 HILO 寄存器类已定义，调度 itinerary（IIHiLo / IImul / IIIdiv）已接入
- 完整算术指令集（ADDu、SUBu、ADD、SUB、MUL、ADDiu 等）已实现
- 移位 / 旋转指令（SHL、SHR、SRA、ROL、ROR 及寄存器版本）已实现
- 64 位乘法（MULT / MULTu → HI:LO，MFHI / MFLO 提取）已实现
- 有符号 / 无符号除法（DAG combine 驱动，SDIVREM / UDIVREM → HI:LO）已实现
- 32 位立即数合成（LUi + ORi 两指令序列）已实现
- GPR ↔ HI/LO 物理寄存器拷贝（`copyPhysReg`）已实现
- `EnableOverflow` 命令行选项已接入，可在 ADD/SUB（有溢出检测）与 ADDu/SUBu 间切换
- 逻辑指令（AND/OR/XOR/NOR 寄存器版；ANDi/ORi/XORi 立即数版）已实现
- 按位取反（`~x`）通过 `NOR x, ZERO` Pat 匹配
- SLT/SLTu/SLTi/SLTiu（cpu032II）和 CMP/CMPu（cpu032I）已实现，操作码已修正
- 完整 setcc 模式（`==`/`!=`/`<`/`<=`/`>`/`>=`）为两种子目标分别实现
- `SIGN_EXTEND_INREG` 展开为 SHL+SRA，处理窄整数符号扩展
- per-function `"target-cpu"` 属性经合法性过滤，防止外来 CPU 名触发断言

### 还未完成的部分

| 功能 | 对应章节 |
|------|---------|
| Load / Store 指令 | Chapter 4.x |
| 分支与跳转指令 | Chapter 4.x |
| 对象文件输出（`-filetype=obj`） | Chapter 5 |
| 全局变量支持 | Chapter 6 |
| 其他数据类型 | Chapter 7 |
| 控制流语句 | Chapter 8 |
| 函数调用完整实现 | Chapter 9 |
| ELF 完整支持 | Chapter 10 |
| 汇编器 / 反汇编器 | Chapter 11 |
| C++ 支持 | Chapter 12 |
| 大帧最优实现（LUi+ORi+ADDU 替代分块 ADDiu） | 待定 |

### 用一句话总结当前整体状态

**Cpu0 backend 已经具备完整的算术/逻辑指令集、setcc 比较支持和栈帧管理能力，能用 `llc -march=cpu0` 编译含乘除法和逻辑比较的 C 表达式；接下来的工作是补全 Load/Store、分支跳转和对象文件输出。**
