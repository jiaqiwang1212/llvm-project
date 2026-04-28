# TableGen 概述

> 原文：https://llvm.org/docs/TableGen/index.html  
> 翻译自：`llvm/docs/TableGen/index.rst`

---

## 目录

1. [简介](#简介)
2. [TableGen 程序](#tablegen-程序)
   - [运行 TableGen](#运行-tablegen)
   - [示例](#示例)
3. [语法](#语法)
   - [基本概念](#基本概念)
4. [TableGen 后端](#tablegen-后端)
5. [工具与资源](#工具与资源)
6. [TableGen 的不足](#tablegen-的不足)

---

## 简介

TableGen 的目的是帮助开发者开发和维护领域特定信息的记录。由于这类记录数量可能非常庞大，TableGen 被专门设计为允许编写灵活的描述，并将记录的公共特性抽取出来复用。这减少了描述中的重复内容，降低了出错的概率，并使领域特定信息更容易结构化。

TableGen 前端负责解析文件、实例化声明，并将结果交给领域特定的[后端](#tablegen-后端)进行处理。请参阅 *TableGen 程序员参考手册*（`ProgRef`）以获取 TableGen 的深入描述。请参阅 *tblgen - 从描述到 C++ 代码*（`tblgen` 命令指南）以了解运行各种 TableGen 变体的 `*-tblgen` 命令的详细信息。

TableGen 目前的主要用户是 *LLVM 目标无关代码生成器*（`CodeGenerator`）和 *Clang 诊断与属性系统*。

注意：如果您经常使用 TableGen 并使用 emacs 或 vim，可以在 LLVM 发行版的 `llvm/utils/emacs` 和 `llvm/utils/vim` 目录中分别找到 emacs 的"TableGen 模式"和 vim 语言文件。

---

## TableGen 程序

TableGen 文件由 TableGen 程序解释：`llvm-tblgen`，位于构建目录的 `bin` 子目录下。它不会安装到系统中（或您设置的 sysroot 位置），因为它在 LLVM 构建过程之外没有实际用途。

### 运行 TableGen

TableGen 的运行方式与其他任何 LLVM 工具相同。第一个（可选）参数指定要读取的文件。如果未指定文件名，`llvm-tblgen` 从标准输入读取。

`-o` 选项指定输出文件，使用 `-` 表示输出到标准输出（stdout）。当 TableGen 生成多个输出文件时，该选项指定主输出文件的名称，该名称也作为其他输出文件的名称前缀。

要发挥实际作用，必须使用某个[后端](#tablegen-后端)。这些后端可以在命令行上选择（输入 `llvm-tblgen -help` 获取列表）。例如，要获取所有子类化某个特定类型的定义列表（这对于构建这些记录的枚举列表很有用），可以使用 `-print-enums` 选项：

```bash
$ llvm-tblgen X86.td -print-enums -class=Register
AH, AL, AX, BH, BL, BP, BPL, BX, CH, CL, CX, DH, DI, DIL, DL, DX, EAX, EBP, EBX,
ECX, EDI, EDX, EFLAGS, EIP, ESI, ESP, FP0, FP1, FP2, FP3, FP4, FP5, FP6, IP,
MM0, MM1, MM2, MM3, MM4, MM5, MM6, MM7, R10, R10B, R10D, R10W, R11, R11B, R11D,
R11W, R12, R12B, R12D, R12W, R13, R13B, R13D, R13W, R14, R14B, R14D, R14W, R15,
R15B, R15D, R15W, R8, R8B, R8D, R8W, R9, R9B, R9D, R9W, RAX, RBP, RBX, RCX, RDI,
RDX, RIP, RSI, RSP, SI, SIL, SP, SPL, ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7,
XMM0, XMM1, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15, XMM2, XMM3, XMM4, XMM5,
XMM6, XMM7, XMM8, XMM9,

$ llvm-tblgen X86.td -print-enums -class=Instruction
ABS_F, ABS_Fp32, ABS_Fp64, ABS_Fp80, ADC32mi, ADC32mi8, ADC32mr, ADC32ri,
ADC32ri8, ADC32rm, ADC32rr, ADC64mi32, ADC64mi8, ADC64mr, ADC64ri32, ADC64ri8,
ADC64rm, ADC64rr, ADD16mi, ADD16mi8, ADD16mr, ADD16ri, ADD16ri8, ADD16rm,
ADD16rr, ADD32mi, ADD32mi8, ADD32mr, ADD32ri, ADD32ri8, ADD32rm, ADD32rr,
ADD64mi32, ADD64mi8, ADD64mr, ADD64ri32, ...
```

默认后端会打印出所有记录。还有一个通用后端，使用 `-dump-json` 选项可将所有记录以 JSON 数据结构的形式输出。

如果您计划使用 TableGen，很可能需要编写一个[后端](#tablegen-后端)，用于提取您所需的特定信息并以适当的格式输出。您可以通过在 C++ 中扩展 TableGen 本身，或编写任何能够处理 JSON 输出的语言脚本来实现这一点。

### 示例

不带其他参数时，`llvm-tblgen` 解析指定文件并打印出所有类，然后是所有定义。这是查看各种定义完全展开后形态的好方法。在 `X86.td` 文件上运行时（截至撰写本文时）会打印以下内容：

```text
...
def ADD32rr {   // Instruction X86Inst I
  string Namespace = "X86";
  dag OutOperandList = (outs GR32:$dst);
  dag InOperandList = (ins GR32:$src1, GR32:$src2);
  string AsmString = "add{l}\t{$src2, $dst|$dst, $src2}";
  list<dag> Pattern = [(set GR32:$dst, (add GR32:$src1, GR32:$src2))];
  list<Register> Uses = [];
  list<Register> Defs = [EFLAGS];
  list<Predicate> Predicates = [];
  int CodeSize = 3;
  int AddedComplexity = 0;
  bit isReturn = 0;
  bit isBranch = 0;
  bit isIndirectBranch = 0;
  bit isBarrier = 0;
  bit isCall = 0;
  bit canFoldAsLoad = 0;
  bit mayLoad = 0;
  bit mayStore = 0;
  bit isImplicitDef = 0;
  bit isConvertibleToThreeAddress = 1;
  bit isCommutable = 1;
  bit isTerminator = 0;
  bit isReMaterializable = 0;
  bit isPredicable = 0;
  bit hasDelaySlot = 0;
  bit usesCustomInserter = 0;
  bit hasCtrlDep = 0;
  bit isNotDuplicable = 0;
  bit hasSideEffects = 0;
  InstrItinClass Itinerary = NoItinerary;
  string Constraints = "";
  string DisableEncoding = "";
  bits<8> Opcode = { 0, 0, 0, 0, 0, 0, 0, 1 };
  Format Form = MRMDestReg;
  bits<6> FormBits = { 0, 0, 0, 0, 1, 1 };
  ImmType ImmT = NoImm;
  bits<3> ImmTypeBits = { 0, 0, 0 };
  bit hasOpSizePrefix = 0;
  bit hasAdSizePrefix = 0;
  bits<4> Prefix = { 0, 0, 0, 0 };
  bit hasREX_WPrefix = 0;
  FPFormat FPForm = ?;
  bits<3> FPFormBits = { 0, 0, 0 };
}
...
```

这个定义对应 x86 架构的 32 位寄存器-寄存器 `add` 指令。`def ADD32rr` 定义了一个名为 `ADD32rr` 的记录，行末的注释指示了该定义的超类。记录的主体包含了 TableGen 为该记录组装的所有数据，指明该指令属于 `X86` 命名空间、指令被代码生成器选择时的模式、它是一条双地址指令、具有特定编码，等等。记录中信息的内容和语义是 X86 后端的特定需求，在此仅作示例展示。

如您所见，代码生成器支持的每条指令都需要大量信息，手动指定所有这些信息将难以维护、容易出错，而且一开始就很繁琐。由于我们使用了 TableGen，所有信息都是从以下定义派生出来的：

```text
let Defs = [EFLAGS],
    isCommutable = 1,                  // X = ADD Y,Z --> X = ADD Z,Y
    isConvertibleToThreeAddress = 1 in // 可以转换为 LEA
def ADD32rr  : I<0x01, MRMDestReg, (outs GR32:$dst),
                                   (ins GR32:$src1, GR32:$src2),
                 "add{l}\t{$src2, $dst|$dst, $src2}",
                 [(set GR32:$dst, (add GR32:$src1, GR32:$src2))]>;
```

这个定义使用了自定义类 `I`（从自定义类 `X86Inst` 扩展而来），该类在 X86 特定的 TableGen 文件中定义，用于提取同类指令共有的特性。TableGen 的一个关键特性是，它允许最终用户定义描述信息时偏好使用的抽象层次。

---

## 语法

TableGen 的语法大致基于 C++ 模板，具有内置类型和规格说明。此外，TableGen 的语法还引入了一些自动化概念，如 `multiclass`、`foreach`、`let` 等。

### 基本概念

TableGen 文件由两个关键部分组成：**类（classes）**和**定义（definitions）**，两者都被视为"记录（records）"。

**TableGen 记录**具有唯一的名称、一个值列表和一个超类列表。值列表是 TableGen 为每条记录构建的主要数据；正是这些数据保存了应用程序的领域特定信息。这些数据的解释留给特定的[后端](#tablegen-后端)处理，但结构和格式规则由 TableGen 负责并固定下来。

**TableGen 定义**是"记录"的具体形式。它们通常没有任何未定义的值，并以 `def` 关键字标记。

```text
def FeatureFPARMv8 : SubtargetFeature<"fp-armv8", "HasFPARMv8", "true",
                                      "Enable ARMv8 FP">;
```

在这个例子中，`FeatureFPARMv8` 是用某些值初始化的 `SubtargetFeature` 记录。类的名称通过关键字 `class` 在同一文件或其他被包含的文件中定义。大多数目标的 TableGen 文件会包含 `include/llvm/Target` 中的通用文件。

**TableGen 类**是抽象记录，用于构建和描述其他记录。这些类允许最终用户为其所针对的领域构建抽象（如 LLVM 代码生成器中的 `Register`、`RegisterClass` 和 `Instruction`），或允许实现者帮助提取记录的公共属性（如 `FPInst`，用于在 X86 后端中表示浮点指令）。TableGen 会跟踪用于构建定义的所有类，因此后端可以找到某个特定类（如 `Instruction`）的所有定义。

```text
class ProcNoItin<string Name, list<SubtargetFeature> Features>
      : Processor<Name, NoItineraries, Features>;
```

这里，类 `ProcNoItin` 接收类型为 `string` 的参数 `Name` 和目标特性列表，通过向下传递参数并硬编码 `NoItineraries` 来特化 `Processor` 类。

**TableGen 多类（multiclasses）**是一组抽象记录，会被一次性全部实例化。每次实例化可以产生多个 TableGen 定义。如果一个多类继承自另一个多类，子多类中的定义将成为当前多类的一部分，就好像它们是在当前多类中声明的一样。

```text
multiclass ro_signed_pats<string T, string Rm, dag Base, dag Offset, dag Extend,
                        dag address, ValueType sty> {
def : Pat<(i32 (!cast<SDNode>("sextload" # sty) address)),
          (!cast<Instruction>("LDRS" # T # "w_" # Rm # "_RegOffset")
            Base, Offset, Extend)>;

def : Pat<(i64 (!cast<SDNode>("sextload" # sty) address)),
          (!cast<Instruction>("LDRS" # T # "x_" # Rm # "_RegOffset")
            Base, Offset, Extend)>;
}

defm : ro_signed_pats<"B", Rm, Base, Offset, Extend,
                      !foreach(decls.pattern, address,
                               !subst(SHIFT, imm_eq0, decls.pattern)),
                      i8>;
```

请参阅 *TableGen 程序员参考手册*（`ProgRef`）以获取 TableGen 的深入描述。

---

## TableGen 后端

没有后端，TableGen 文件没有实际意义。运行 `*-tblgen` 时的默认操作是以文本格式打印信息，但这仅对调试 TableGen 文件本身有用。TableGen 的强大之处在于，它能将源文件解释为一种内部表示，并从中生成您想要的任何内容。

TableGen 目前的典型用法是创建包含表格的大型头文件，您可以直接包含这些文件（如果输出使用的是您正在编写的语言），或通过包含文件周围的宏在预处理阶段使用它们。

如果后端已经以 C 格式打印表格，或者输出只是字符串列表（用于错误和警告消息），则可以使用直接输出。如果同样的信息需要在不同的上下文中使用（如指令名称），则应使用预处理输出，因此您的后端应打印一个元信息列表，该列表可以在不同的编译时格式中成型。

请参阅 *TableGen 后端*（`BackEnds`）文档以获取可用后端列表，并参阅 *TableGen 后端开发者指南*（`BackGuide`）以了解如何编写和调试新后端的信息。

---

## 工具与资源

除本文档外，TableGen 的工具和资源列表可在 TableGen 的
[README](https://github.com/llvm/llvm-project/blob/main/llvm/utils/TableGen/README.md)
中找到。

---

## TableGen 的不足

尽管 TableGen 非常通用，但它存在一些多次被指出的不足之处。共同的主题是：虽然 TableGen 允许您构建领域特定语言，但最终创建的语言缺乏其他 DSL 的能力，这反过来大大增加了 TableGen 文件的大小和复杂性。

与此同时，TableGen 允许您通过定制后端为基本概念创建几乎任何含义，这可能会扭曲原始设计，使新手很难理解晦涩的 TableGen 文件。

有些人主张进一步扩展语义，但要确保后端遵守严格的规则。另一些人则建议转向更少、更强大的、为特定目的而设计的 DSL，甚至重用现有的 DSL。

---

*本文档翻译自 LLVM 官方文档 [TableGen Overview](https://llvm.org/docs/TableGen/index.html)。*
