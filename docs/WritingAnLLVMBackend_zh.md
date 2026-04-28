# 编写 LLVM 后端

> 原文：https://llvm.org/docs/WritingAnLLVMBackend.html  
> 翻译自：`llvm/docs/WritingAnLLVMBackend.rst`

---

## 目录

1. [简介](#简介)
   - [读者对象](#读者对象)
   - [预备阅读材料](#预备阅读材料)
   - [基本步骤](#基本步骤)
   - [准备工作](#准备工作)
2. [目标机器](#目标机器)
3. [目标注册](#目标注册)
4. [寄存器集与寄存器类](#寄存器集与寄存器类)
   - [定义寄存器](#定义寄存器)
   - [定义寄存器类](#定义寄存器类)
   - [实现 TargetRegisterInfo 子类](#实现-targetregisterinfo-子类)
5. [指令集](#指令集)
   - [指令操作数映射](#指令操作数映射)
   - [指令操作数名称映射](#指令操作数名称映射)
   - [指令操作数类型](#指令操作数类型)
   - [指令调度](#指令调度)
   - [指令关系映射](#指令关系映射)
   - [实现 TargetInstrInfo 子类](#实现-targetinstrinfo-子类)
   - [分支折叠与 If 转换](#分支折叠与-if-转换)
6. [指令选择器](#指令选择器)
   - [SelectionDAG 合法化阶段](#selectiondag-合法化阶段)
   - [调用约定](#调用约定)
7. [汇编打印器](#汇编打印器)
8. [子目标支持](#子目标支持)
9. [JIT 支持](#jit-支持)
   - [机器码发射器](#机器码发射器)
   - [目标 JIT 信息](#目标-jit-信息)

---

## 简介

本文档介绍编写编译器后端的技术，这些后端将 LLVM 中间表示（IR）转换为特定机器或其他语言的代码。面向特定机器的代码可以是汇编代码，也可以是二进制代码（可用于 JIT 编译器）。

LLVM 的后端具有一个目标无关的代码生成器，可以为多种目标 CPU 生成输出，包括 X86、PowerPC、ARM 和 SPARC。该后端还可用于生成面向 Cell 处理器的 SPU 或 GPU 的代码，以支持计算内核的执行。

本文档重点介绍在下载的 LLVM 版本中 `llvm/lib/Target` 子目录下的现有示例。特别地，本文档重点介绍为 SPARC 目标创建静态编译器（生成文本汇编代码）的示例，因为 SPARC 具有相当标准的特性，例如 RISC 指令集和简单直接的调用约定。

### 读者对象

本文档面向任何需要编写 LLVM 后端以为特定硬件或软件目标生成代码的读者。

### 预备阅读材料

在阅读本文档之前，必须先阅读以下重要文档：

- **LLVM 语言参考手册**（`LangRef.html`）—— LLVM 汇编语言的参考手册。

- **代码生成器文档**（`CodeGenerator`）—— 关于将 LLVM 内部表示转换为特定目标机器代码的组件（类和代码生成算法）的指南。请特别注意代码生成各阶段的描述：指令选择、调度与构建、基于 SSA 的优化、寄存器分配、序言/尾声代码插入、后期机器码优化以及代码发射。

- **TableGen 文档**（`TableGen/index`）—— 描述 TableGen（`tblgen`）应用程序的文档，该应用程序管理领域特定信息以支持 LLVM 代码生成。TableGen 处理来自目标描述文件（`.td` 后缀）的输入，并生成可用于代码生成的 C++ 代码。

- **编写 LLVM Pass 文档**（`WritingAnLLVMPass`）—— 汇编打印器是一个 `FunctionPass`，多个 `SelectionDAG` 处理步骤也是如此。

要跟随本文档中的 SPARC 示例，请参考 *《SPARC 体系结构手册，版本 8》*。有关 ARM 指令集的详细信息，请参考 *ARM 体系结构参考手册*。有关 GNU 汇编器格式（`GAS`）的更多信息，请参见 *Using As*，特别是关于汇编打印器的部分。

### 基本步骤

要编写一个将 LLVM IR 转换为特定目标（机器或其他语言）代码的 LLVM 编译器后端，请执行以下步骤：

1. **创建 `TargetMachine` 子类**，描述目标机器的特性。复制现有的具体 `TargetMachine` 类和头文件；例如，从 `SparcTargetMachine.cpp` 和 `SparcTargetMachine.h` 开始，但为您的目标更改文件名，并将代码中引用 "Sparc" 的地方改为引用您的目标。

2. **描述目标的寄存器集**。使用 TableGen 从目标特定的 `RegisterInfo.td` 输入文件生成寄存器定义、寄存器别名和寄存器类的代码。您还应该为 `TargetRegisterInfo` 类的子类编写额外代码，该类表示用于寄存器分配的类寄存器文件数据，并描述寄存器之间的交互关系。

3. **描述目标的指令集**。使用 TableGen 从 `TargetInstrFormats.td` 和 `TargetInstrInfo.td` 的目标特定版本生成目标特定指令的代码。您应该为 `TargetInstrInfo` 类的子类编写额外代码，以表示目标机器支持的机器指令。

4. **描述 LLVM IR 的选择与转换**，即将有向无环图（DAG）表示的指令转换为原生的目标特定指令。使用 TableGen 生成匹配模式并基于 `TargetInstrInfo.td` 目标特定版本中的附加信息选择指令的代码。在 `XXXISelDAGToDAG.cpp` 中编写代码（其中 `XXX` 标识特定目标）以执行模式匹配和 DAG 到 DAG 的指令选择。还应在 `XXXISelLowering.cpp` 中编写代码，以替换或删除 SelectionDAG 中原生不支持的操作和数据类型。

5. **为汇编打印器编写代码**，将 LLVM IR 转换为目标机器的 GAS 格式。您应该在目标特定版本的 `TargetInstrInfo.td` 中定义的指令中添加汇编字符串。您还应该为 `AsmPrinter` 的子类编写代码，该类执行 LLVM 到汇编的转换，以及 `TargetAsmInfo` 的简单子类。

6. **（可选）添加子目标支持**（即具有不同能力的变体）。您还应该为 `TargetSubtarget` 类的子类编写代码，该类允许您使用 `-mcpu=` 和 `-mattr=` 命令行选项。

7. **（可选）添加 JIT 支持**，创建一个机器码发射器（`TargetJITInfo` 的子类），用于将二进制代码直接写入内存。

在 `.cpp` 和 `.h` 文件中，初期先将这些方法的实现留为桩代码，之后再逐步实现。初期您可能还不知道该类需要哪些私有成员，以及哪些组件需要被子类化。

### 准备工作

要实际创建您的编译器后端，您需要创建和修改一些文件。下面讨论所需的最低限度。但要实际使用 LLVM 目标无关代码生成器，您必须执行 *LLVM 目标无关代码生成器* 文档中描述的步骤。

首先，您应该在 `lib/Target` 下创建一个子目录来存放与目标相关的所有文件。如果您的目标叫做 "Dummy"，则创建目录 `lib/Target/Dummy`。

在这个新目录中，创建一个 `CMakeLists.txt`。最简单的方法是复制另一个目标的 `CMakeLists.txt` 并修改它。它至少应包含 `LLVM_TARGET_DEFINITIONS` 变量。库可以命名为 `LLVMDummy`（例如，参见 MIPS 目标）。或者，您可以将库拆分为 `LLVMDummyCodeGen` 和 `LLVMDummyAsmPrinter`，后者应在 `lib/Target/Dummy` 下的子目录中实现（例如，参见 PowerPC 目标）。

请注意，这两种命名方案被硬编码到 `llvm-config` 中。使用任何其他命名方案都会使 `llvm-config` 混乱，并在链接 `llc` 时产生大量（表面上不相关的）链接器错误。

要让目标真正做点事情，您需要实现 `TargetMachine` 的子类。该实现通常位于文件 `lib/Target/DummyTargetMachine.cpp` 中，但 `lib/Target` 目录中的任何文件都会被构建并应该能正常工作。要使用 LLVM 的目标无关代码生成器，您应该像所有当前的机器后端那样：创建 `CodeGenTargetMachineImpl` 的子类。（要从头创建一个目标，可以创建 `TargetMachine` 的子类。）

要让 LLVM 实际构建并链接您的目标，您需要使用 `-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Dummy` 运行 `cmake`。这将构建您的目标，而无需将其添加到所有目标的列表中。

一旦您的目标稳定下来，您可以将其添加到主 `CMakeLists.txt` 中的 `LLVM_ALL_TARGETS` 变量中。

---

## 目标机器

`CodeGenTargetMachineImpl` 被设计为使用 LLVM 目标无关代码生成器实现的目标的基类。`CodeGenTargetMachineImpl` 类应该被实现了各种虚方法的具体目标类所特化。`CodeGenTargetMachineImpl` 被定义为 `include/llvm/CodeGen/CodeGenTargetMachineImpl.h` 中 `TargetMachine` 的子类。`TargetMachine` 类的实现（`include/llvm/Target/TargetMachine.cpp`）还处理许多命令行选项。

要创建 `CodeGenTargetMachineImpl` 的具体目标特定子类，请先复制一个现有的 `TargetMachine` 类和头文件。您创建的文件应反映您的特定目标。例如，对于 SPARC 目标，文件名为 `SparcTargetMachine.h` 和 `SparcTargetMachine.cpp`。

对于目标机器 `XXX`，`XXXTargetMachine` 的实现必须具有访问表示目标组件的对象的访问方法。这些方法命名为 `get*Info`，旨在获取指令集（`getInstrInfo`）、寄存器集（`getRegisterInfo`）、栈帧布局（`getFrameInfo`）以及类似信息。`XXXTargetMachine` 还必须实现 `getDataLayout` 方法，以访问具有目标特定数据特性（如数据类型大小和对齐要求）的对象。

例如，对于 SPARC 目标，头文件 `SparcTargetMachine.h` 声明了几个 `get*Info` 和 `getDataLayout` 方法的原型，这些方法简单地返回类成员：

```cpp
namespace llvm {

class Module;

class SparcTargetMachine : public CodeGenTargetMachineImpl {
  const DataLayout DataLayout;       // 计算类型大小和对齐
  SparcSubtarget Subtarget;
  SparcInstrInfo InstrInfo;
  TargetFrameInfo FrameInfo;

protected:
  virtual const TargetAsmInfo *createTargetAsmInfo() const;

public:
  SparcTargetMachine(const Module &M, const std::string &FS);

  virtual const SparcInstrInfo *getInstrInfo() const {return &InstrInfo; }
  virtual const TargetFrameInfo *getFrameInfo() const {return &FrameInfo; }
  virtual const TargetSubtarget *getSubtargetImpl() const{return &Subtarget; }
  virtual const TargetRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }
  virtual const DataLayout *getDataLayout() const { return &DataLayout; }

  // Pass 流水线配置
  virtual bool addInstSelector(PassManagerBase &PM, bool Fast);
  virtual bool addPreEmitPass(PassManagerBase &PM, bool Fast);
};

} // end namespace llvm
```

需要实现的访问方法包括：

- `getInstrInfo()`
- `getRegisterInfo()`
- `getFrameInfo()`
- `getDataLayout()`
- `getSubtargetImpl()`

对于某些目标，您还需要支持以下方法：

- `getTargetLowering()`
- `getJITInfo()`

某些体系结构（如 GPU）不支持跳转到任意程序位置，而是使用掩码执行实现分支，并使用循环体周围的特殊指令实现循环。为了避免引入此类硬件无法处理的不可规约控制流的 CFG 修改，目标在初始化时必须调用 `setRequiresStructuredCFG(true)`。

此外，`XXXTargetMachine` 的构造函数应指定一个 `TargetDescription` 字符串，该字符串确定目标机器的数据布局，包括指针大小、对齐和字节序等特性。例如，`SparcTargetMachine` 的构造函数包含以下内容：

```cpp
SparcTargetMachine::SparcTargetMachine(const Module &M, const std::string &FS)
  : DataLayout("E-p:32:32-f128:128:128"),
    Subtarget(M, FS), InstrInfo(Subtarget),
    FrameInfo(TargetFrameInfo::StackGrowsDown, 8, 0) {
}
```

`TargetDescription` 字符串的各部分用连字符分隔：

- 字符串中大写的 "`E`" 表示大端目标数据模型；小写的 "`e`" 表示小端。

- "`p:`" 后跟指针信息：大小、ABI 对齐和首选对齐。如果 "`p:`" 后只跟两个数字，则第一个值是指针大小，第二个值同时是 ABI 对齐和首选对齐。

- 然后是数值类型对齐的字母："`i`"、"`f`"、"`v`" 或 "`a`"（分别对应整数、浮点、向量或聚合）。"`i`"、"`v`" 或 "`a`" 后跟 ABI 对齐和首选对齐。"`f`" 后跟三个值：第一个表示 long double 的大小，然后是 ABI 对齐，最后是 ABI 首选对齐。

---

## 目标注册

您还必须向 `TargetRegistry` 注册您的目标，其他 LLVM 工具使用它在运行时查找和使用您的目标。`TargetRegistry` 可以直接使用，但对于大多数目标，有辅助模板可以为您处理这些工作。

所有目标都应声明一个全局 `Target` 对象，用于在注册期间表示该目标。然后，在目标的 `TargetInfo` 库中，目标应定义该对象并使用 `RegisterTarget` 模板注册目标。例如，Sparc 的注册代码如下：

```cpp
Target llvm::getTheSparcTarget();

extern "C" void LLVMInitializeSparcTargetInfo() {
  RegisterTarget<Triple::sparc, /*HasJIT=*/false>
    X(getTheSparcTarget(), "sparc", "Sparc");
}
```

这允许 `TargetRegistry` 通过名称或目标三元组查找目标。此外，大多数目标还将注册在单独库中可用的附加功能。这些注册步骤是分开的，因为某些客户端可能只希望链接目标的某些部分——例如，JIT 代码生成器不需要使用汇编打印器。以下是注册 Sparc 汇编打印器的示例：

```cpp
extern "C" void LLVMInitializeSparcAsmPrinter() {
  RegisterAsmPrinter<SparcAsmPrinter> X(getTheSparcTarget());
}
```

---

## 寄存器集与寄存器类

您应该描述一个具体的目标特定类来表示目标机器的寄存器文件。该类称为 `XXXRegisterInfo`（其中 `XXX` 标识目标），表示用于寄存器分配的类寄存器文件数据，并描述寄存器之间的交互关系。

您还需要定义寄存器类来对相关寄存器进行分类。对于以相同方式处理某条指令的一组寄存器，应添加一个寄存器类。典型示例是整数、浮点或向量寄存器的寄存器类。寄存器分配器允许指令使用指定寄存器类中的任何寄存器以类似方式执行指令。寄存器类从这些集合中为指令分配虚拟寄存器，并让目标无关寄存器分配器自动选择实际寄存器。

寄存器的大部分代码，包括寄存器定义、寄存器别名和寄存器类，由 TableGen 从 `XXXRegisterInfo.td` 输入文件生成，并放置在 `XXXGenRegisterInfo.h.inc` 和 `XXXGenRegisterInfo.inc` 输出文件中。`XXXRegisterInfo` 实现中的部分代码需要手工编写。

### 定义寄存器

`XXXRegisterInfo.td` 文件通常以目标机器的寄存器定义开始。`Target.td` 中指定的 `Register` 类用于为每个寄存器定义一个对象。指定的字符串 `n` 成为寄存器的 `Name`。基本的 `Register` 对象没有任何子寄存器，也不指定任何别名。

```
class Register<string n> {
  string Namespace = "";
  string AsmName = n;
  string Name = n;
  int SpillSize = 0;
  int SpillAlignment = 0;
  list<Register> Aliases = [];
  list<Register> SubRegs = [];
  list<int> DwarfNumbers = [];
}
```

例如，在 `X86RegisterInfo.td` 文件中，有使用 `Register` 类的寄存器定义，如：

```
def AL : Register<"AL">, DwarfRegNum<[0, 0, 0]>;
```

这定义了寄存器 `AL`，并赋予它由 `gcc`、`gdb` 或调试信息写入器使用的值（通过 `DwarfRegNum`）来标识寄存器。对于寄存器 `AL`，`DwarfRegNum` 接受一个包含 3 个值的数组，表示 3 种不同模式：第一个元素用于 X86-64，第二个用于 X86-32 上的异常处理（EH），第三个是通用的。-1 是一个特殊的 Dwarf 编号，表示 gcc 编号未定义；-2 表示寄存器编号在此模式下无效。

从 `X86RegisterInfo.td` 文件中前面描述的行，TableGen 在 `X86GenRegisterInfo.inc` 文件中生成以下代码：

```cpp
static const unsigned GR8[] = { X86::AL, ... };

const unsigned AL_AliasSet[] = { X86::AX, X86::EAX, X86::RAX, 0 };

const TargetRegisterDesc RegisterDescriptors[] = {
  ...
{ "AL", "AL", AL_AliasSet, Empty_SubRegsSet, Empty_SubRegsSet, AL_SuperRegsSet }, ...
```

从寄存器信息文件中，TableGen 为每个寄存器生成一个 `TargetRegisterDesc` 对象。`TargetRegisterDesc` 在 `include/llvm/Target/TargetRegisterInfo.h` 中定义，具有以下字段：

```cpp
struct TargetRegisterDesc {
  const char     *AsmName;      // 寄存器的汇编语言名称
  const char     *Name;         // 可打印名称（用于调试）
  const unsigned *AliasSet;     // 寄存器别名集
  const unsigned *SubRegs;      // 子寄存器集
  const unsigned *ImmSubRegs;   // 直接子寄存器集
  const unsigned *SuperRegs;    // 超寄存器集
};
```

TableGen 使用整个目标描述文件（`.td`）来确定寄存器的文本名称（在 `TargetRegisterDesc` 的 `AsmName` 和 `Name` 字段中）以及其他寄存器与已定义寄存器的关系（在其他 `TargetRegisterDesc` 字段中）。在这个例子中，其他定义将寄存器 "`AX`"、"`EAX`" 和 "`RAX`" 互相建立为别名，因此 TableGen 为这个寄存器别名集生成一个以 null 结尾的数组（`AL_AliasSet`）。

`Register` 类通常用作更复杂类的基类。在 `Target.td` 中，`Register` 类是 `RegisterWithSubRegs` 类的基类，后者用于定义需要在 `SubRegs` 列表中指定子寄存器的寄存器，如下所示：

```
class RegisterWithSubRegs<string n, list<Register> subregs> : Register<n> {
  let SubRegs = subregs;
}
```

在 `SparcRegisterInfo.td` 中，为 SPARC 定义了额外的寄存器类：一个 `Register` 子类 `SparcReg`，以及进一步的子类：`Ri`、`Rf` 和 `Rd`。SPARC 寄存器由 5 位 ID 号标识，这是这些子类的共同特性。注意使用 "`let`" 表达式来覆盖超类中最初定义的值（例如 `Rd` 类中的 `SubRegs` 字段）：

```
class SparcReg<string n> : Register<n> {
  field bits<5> Num;
  let Namespace = "SP";
}
// Ri - 32 位整数寄存器
class Ri<bits<5> num, string n> : SparcReg<n> {
  let Num = num;
}
// Rf - 32 位浮点寄存器
class Rf<bits<5> num, string n> : SparcReg<n> {
  let Num = num;
}
// Rd - FP 寄存器文件中用于 64 位浮点值的槽
class Rd<bits<5> num, string n, list<Register> subregs> : SparcReg<n> {
  let Num = num;
  let SubRegs = subregs;
}
```

在 `SparcRegisterInfo.td` 文件中，有使用这些 `Register` 子类的寄存器定义，如：

```
def G0 : Ri< 0, "G0">, DwarfRegNum<[0]>;
def G1 : Ri< 1, "G1">, DwarfRegNum<[1]>;
...
def F0 : Rf< 0, "F0">, DwarfRegNum<[32]>;
def F1 : Rf< 1, "F1">, DwarfRegNum<[33]>;
...
def D0 : Rd< 0, "F0", [F0, F1]>, DwarfRegNum<[32]>;
def D1 : Rd< 2, "F2", [F2, F3]>, DwarfRegNum<[34]>;
```

上面显示的最后两个寄存器（`D0` 和 `D1`）是双精度浮点寄存器，它们是成对单精度浮点子寄存器的别名。除别名外，已定义寄存器的子寄存器和超寄存器关系也在寄存器的 `TargetRegisterDesc` 字段中。

### 定义寄存器类

`Target.td` 中指定的 `RegisterClass` 类用于定义表示一组相关寄存器的对象，并定义寄存器的默认分配顺序。使用 `Target.td` 的目标描述文件 `XXXRegisterInfo.td` 可以使用以下类构建寄存器类：

```
class RegisterClass<string namespace,
list<ValueType> regTypes, int alignment, dag regList> {
  string Namespace = namespace;
  list<ValueType> RegTypes = regTypes;
  int Size = 0;  // 溢出大小（位）；零表示让 tblgen 选择大小
  int Alignment = alignment;

  // CopyCost 是在两个寄存器之间复制值的代价
  // 默认值 1 表示单条指令
  // 负值表示复制极其昂贵或不可能
  int CopyCost = 1;
  dag MemberList = regList;

  // 对于是此类子寄存器的寄存器类
  list<RegisterClass> SubRegClassList = [];

  code MethodProtos = [{}];  // 插入任意代码
  code MethodBodies = [{}];
}
```

要定义 `RegisterClass`，使用以下 4 个参数：

- **第一个参数**是命名空间的名称。

- **第二个参数**是 `ValueType` 寄存器类型值的列表，这些值在 `include/llvm/CodeGen/ValueTypes.td` 中定义。定义的值包括整数类型（如 `i16`、`i32`，以及用于布尔值的 `i1`）、浮点类型（`f32`、`f64`）和向量类型（例如，`v8i16` 表示 `8 x i16` 向量）。`RegisterClass` 中的所有寄存器必须具有相同的 `ValueType`，但某些寄存器可能以不同配置存储向量数据。例如，能处理 128 位向量的寄存器可能能够处理 16 个 8 位整数元素、8 个 16 位整数、4 个 32 位整数等。

- **第三个参数**指定寄存器在存储或加载到内存时所需的对齐方式。

- **最后一个参数** `regList`，指定该类中有哪些寄存器。如果未指定替代分配顺序方法，则 `regList` 也定义寄存器分配器使用的分配顺序。除了简单地使用 `(add R0, R1, ...)` 列出寄存器外，还有更高级的集合运算符可用。详见 `include/llvm/Target/Target.td`。

在 `SparcRegisterInfo.td` 中，定义了三个 `RegisterClass` 对象：`FPRegs`、`DFPRegs` 和 `IntRegs`。对于所有三个寄存器类，第一个参数定义了字符串 "`SP`" 的命名空间。`FPRegs` 定义了一组 32 个单精度浮点寄存器（`F0` 到 `F31`）；`DFPRegs` 定义了一组 16 个双精度寄存器（`D0-D15`）。

```
// F0, F1, F2, ..., F31
def FPRegs : RegisterClass<"SP", [f32], 32, (sequence "F%u", 0, 31)>;

def DFPRegs : RegisterClass<"SP", [f64], 64,
                            (add D0, D1, D2, D3, D4, D5, D6, D7, D8,
                                 D9, D10, D11, D12, D13, D14, D15)>;

def IntRegs : RegisterClass<"SP", [i32], 32,
    (add L0, L1, L2, L3, L4, L5, L6, L7,
         I0, I1, I2, I3, I4, I5,
         O0, O1, O2, O3, O4, O5, O7,
         G1,
         // 不可分配的寄存器：
         G2, G3, G4,
         O6,        // 栈指针
         I6,        // 帧指针
         I7,        // 返回地址
         G0,        // 常量零
         G5, G6, G7 // 为内核保留
    )>;
```

将 `SparcRegisterInfo.td` 与 TableGen 一起使用会生成几个输出文件，这些文件打算包含在您编写的其他源代码中。`SparcRegisterInfo.td` 生成 `SparcGenRegisterInfo.h.inc`，该文件应包含在您编写的 SPARC 寄存器实现的头文件（`SparcRegisterInfo.h`）中。在 `SparcGenRegisterInfo.h.inc` 中，定义了一个名为 `SparcGenRegisterInfo` 的新结构，以 `TargetRegisterInfo` 作为其基类。它还根据定义的寄存器类指定了类型：`DFPRegsClass`、`FPRegsClass` 和 `IntRegsClass`。

`SparcRegisterInfo.td` 还生成 `SparcGenRegisterInfo.inc`，该文件包含在 SPARC 寄存器实现文件 `SparcRegisterInfo.cpp` 的底部。下面的代码仅显示生成的整数寄存器和关联的寄存器类。`IntRegs` 中寄存器的顺序反映了目标描述文件中 `IntRegs` 定义的顺序：

```cpp
// IntRegs 寄存器类...
static const unsigned IntRegs[] = {
  SP::L0, SP::L1, SP::L2, SP::L3, SP::L4, SP::L5,
  SP::L6, SP::L7, SP::I0, SP::I1, SP::I2, SP::I3,
  SP::I4, SP::I5, SP::O0, SP::O1, SP::O2, SP::O3,
  SP::O4, SP::O5, SP::O7, SP::G1, SP::G2, SP::G3,
  SP::G4, SP::O6, SP::I6, SP::I7, SP::G0, SP::G5,
  SP::G6, SP::G7,
};

// IntRegs 寄存器类值类型...
static const MVT::ValueType IntRegsVTs[] = {
  MVT::i32, MVT::Other
};

namespace SP {   // 寄存器类实例
  DFPRegsClass    DFPRegsRegClass;
  FPRegsClass     FPRegsRegClass;
  IntRegsClass    IntRegsRegClass;
...
  // IntRegs 子寄存器类...
  static const TargetRegisterClass* const IntRegsSubRegClasses [] = {
    NULL
  };
...
  // IntRegs 超寄存器类...
  static const TargetRegisterClass* const IntRegsSuperRegClasses [] = {
    NULL
  };
...
  // IntRegs 寄存器类子类...
  static const TargetRegisterClass* const IntRegsSubclasses [] = {
    NULL
  };
...
  // IntRegs 寄存器类超类...
  static const TargetRegisterClass* const IntRegsSuperclasses [] = {
    NULL
  };

  IntRegsClass::IntRegsClass() : TargetRegisterClass(IntRegsRegClassID,
    IntRegsVTs, IntRegsSubclasses, IntRegsSuperclasses, IntRegsSubRegClasses,
    IntRegsSuperRegClasses, 4, 4, 1, IntRegs, IntRegs + 32) {}
}
```

寄存器分配器会避免使用保留寄存器，被调用者保存寄存器在所有易失性寄存器都已使用之后才会使用。这通常已经足够好了，但在某些情况下可能需要提供自定义分配顺序。

### 实现 TargetRegisterInfo 子类

最后一步是手动编写 `XXXRegisterInfo` 的部分代码，该代码实现 `TargetRegisterInfo.h` 中描述的接口。除非被覆盖，否则这些函数返回 `0`、`NULL` 或 `false`。以下是在 `SparcRegisterInfo.cpp` 中为 SPARC 实现覆盖的函数列表：

- `getCalleeSavedRegs` —— 按所需的被调用者保存栈帧偏移顺序返回被调用者保存寄存器的列表。

- `getReservedRegs` —— 返回一个按物理寄存器编号索引的位集，指示特定寄存器是否不可用。

- `hasFP` —— 返回一个布尔值，指示函数是否应该有专用的帧指针寄存器。

- `eliminateCallFramePseudoInstr` —— 如果使用了调用帧设置或销毁伪指令，可以调用此方法来消除它们。

- `eliminateFrameIndex` —— 从可能使用抽象帧索引的指令中消除抽象帧索引。

- `emitPrologue` —— 将序言代码插入函数中。

- `emitEpilogue` —— 将尾声代码插入函数中。

---

## 指令集

在代码生成的早期阶段，LLVM IR 代码被转换为一个 `SelectionDAG`，其节点是包含目标指令的 `SDNode` 类的实例。`SDNode` 具有操作码、操作数、类型要求和操作属性。例如，操作是否可交换，操作是否从内存加载。各种操作节点类型在 `include/llvm/CodeGen/SelectionDAGNodes.h` 文件中描述（`ISD` 命名空间中 `NodeType` 枚举的值）。

TableGen 使用以下目标描述（`.td`）输入文件生成大部分指令定义代码：

- `Target.td` —— 定义 `Instruction`、`Operand`、`InstrInfo` 和其他基本类的地方。

- `TargetSelectionDAG.td` —— 由 `SelectionDAG` 指令选择生成器使用，包含 `SDTC*` 类（SelectionDAG 类型约束）、`SelectionDAG` 节点的定义（如 `imm`、`cond`、`bb`、`add`、`fadd`、`sub`）以及模式支持（`Pattern`、`Pat`、`PatFrag`、`PatLeaf`、`ComplexPattern`）。

- `XXXInstrFormats.td` —— 目标特定指令定义的模式。

- `XXXInstrInfo.td` —— 指令模板、条件码和指令集指令的目标特定定义。对于体系结构修改，可能使用不同的文件名。例如，对于带 SSE 指令的 Pentium，此文件是 `X86InstrSSE.td`；对于带 MMX 的 Pentium，此文件是 `X86InstrMMX.td`。

还有一个目标特定的 `XXX.td` 文件，其中 `XXX` 是目标的名称。`XXX.td` 文件包含其他 `.td` 输入文件，但其内容仅对子目标直接重要。

您应该描述一个具体的目标特定类 `XXXInstrInfo`，它表示目标机器支持的机器指令。`XXXInstrInfo` 包含一个 `XXXInstrDescriptor` 对象数组，每个对象描述一条指令。指令描述符定义：

- 操作码助记符
- 操作数数量
- 隐式寄存器定义和使用的列表
- 目标无关属性（如内存访问、是否可交换）
- 目标特定标志

`Instruction` 类（在 `Target.td` 中定义）主要用作更复杂指令类的基类：

```
class Instruction {
  string Namespace = "";
  dag OutOperandList;    // 包含 MI def 操作数列表的 dag
  dag InOperandList;     // 包含 MI use 操作数列表的 dag
  string AsmString = ""; // 打印指令的 .s 格式
  list<dag> Pattern;     // 此指令的 DAG 模式
  list<Register> Uses = [];
  list<Register> Defs = [];
  list<Predicate> Predicates = [];  // 谓词转换为 isel 匹配代码
  ... 其余部分省略 ...
}
```

`SelectionDAG` 节点（`SDNode`）应包含一个表示目标特定指令的对象，该指令在 `XXXInstrInfo.td` 中定义。指令对象应表示目标机器的体系结构手册中的指令（如 SPARC 目标的 SPARC 体系结构手册中的指令）。

来自体系结构手册的单条指令通常被建模为多个目标指令，具体取决于其操作数。例如，手册可能描述一条接受寄存器或立即数操作数的加法指令。LLVM 目标可以用名为 `ADDri` 和 `ADDrr` 的两条指令来建模。

您应该为每个指令类别定义一个类，并将每个操作码定义为该类别的子类，带有适当的参数，如操作码和扩展操作码的固定二进制编码。您应该将寄存器位映射到指令中对应编码的位（用于 JIT）。您还应该指定在使用自动汇编打印器时如何打印指令。

如 SPARC 体系结构手册版本 8 所述，指令有三种主要的 32 位格式。格式 1 仅用于 `CALL` 指令。格式 2 用于条件码分支和 `SETHI`（设置寄存器高位）指令。格式 3 用于其他指令。

这些格式中的每一种在 `SparcInstrFormat.td` 中都有相应的类。`InstSP` 是其他指令类的基类。为更精确的格式指定了额外的基类：例如，在 `SparcInstrFormat.td` 中，`F2_1` 用于 `SETHI`，`F2_2` 用于分支。还有三个其他基类：`F3_1` 用于寄存器/寄存器操作，`F3_2` 用于寄存器/立即数操作，`F3_3` 用于浮点操作。`SparcInstrInfo.td` 还为合成 SPARC 指令添加了基类 `Pseudo`。

`SparcInstrInfo.td` 主要由 SPARC 目标的操作数和指令定义组成。在 `SparcInstrInfo.td` 中，以下目标描述文件条目 `LDrr` 定义了从内存地址加载到寄存器的加载整数字指令（`LD` SPARC 操作码）。第一个参数（值 3，即 `11`₂）是此类操作的操作值。第二个参数（`000000`₂）是 `LD`/加载字的特定操作值。第三个参数是输出目的地，即在 `Register` 目标描述文件（`IntRegs`）中定义的寄存器操作数：

```
def LDrr : F3_1 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMrr $rs1, $rs2):$addr),
                 "ld [$addr], $dst",
                 [(set i32:$dst, (load ADDRrr:$addr))]>;
```

第四个参数是输入源，使用在 `SparcInstrInfo.td` 中较早定义的地址操作数 `MEMrr`：

```
def MEMrr : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops IntRegs, IntRegs);
}
```

第五个参数是汇编打印器使用的字符串，在实现汇编打印器接口之前可以留为空字符串。第六个也是最后一个参数是在 SelectionDAG 选择阶段匹配指令时使用的模式（详见代码生成器文档）。

指令类定义不为不同的操作数类型重载，因此需要为寄存器、内存或立即数值操作数提供单独版本的指令。例如，要执行从立即数操作数到寄存器的加载整数字指令，定义了以下指令类：

```
def LDri : F3_2 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMri $rs1, $simm13):$addr),
                 "ld [$addr], $dst",
                 [(set i32:$rd, (load ADDRri:$addr))]>;
```

为如此多的相似指令编写这些定义可能涉及大量复制粘贴。在 `.td` 文件中，`multiclass` 指令允许创建模板以一次定义多个指令类（使用 `defm` 指令）。例如，在 `SparcInstrInfo.td` 中，`multiclass` 模式 `F3_12` 被定义为在每次调用 `F3_12` 时创建 2 个指令类：

```
multiclass F3_12 <string OpcStr, bits<6> Op3Val, SDNode OpNode> {
  def rr  : F3_1 <2, Op3Val,
                 (outs IntRegs:$rd), (ins IntRegs:$rs1, IntRegs:$rs1),
                 !strconcat(OpcStr, " $rs1, $rs2, $rd"),
                 [(set i32:$rd, (OpNode i32:$rs1, i32:$rs2))]>;
  def ri  : F3_2 <2, Op3Val,
                 (outs IntRegs:$rd), (ins IntRegs:$rs1, i32imm:$simm13),
                 !strconcat(OpcStr, " $rs1, $simm13, $rd"),
                 [(set i32:$rd, (OpNode i32:$rs1, simm13:$simm13))]>;
}
```

因此，当 `defm` 指令用于 `XOR` 和 `ADD` 指令时，如下所示，它创建了四个指令对象：`XORrr`、`XORri`、`ADDrr` 和 `ADDri`：

```
defm XOR   : F3_12<"xor", 0b000011, xor>;
defm ADD   : F3_12<"add", 0b000000, add>;
```

`SparcInstrInfo.td` 还包含分支指令引用的条件码定义。以下 `SparcInstrInfo.td` 中的定义指示 SPARC 条件码的位位置。例如，第 10 位表示整数的"大于"条件，第 22 位表示浮点的"大于"条件：

```
def ICC_NE  : ICC_VAL< 9>;  // 不等
def ICC_E   : ICC_VAL< 1>;  // 等
def ICC_G   : ICC_VAL<10>;  // 大于
...
def FCC_U   : FCC_VAL<23>;  // 无序
def FCC_G   : FCC_VAL<22>;  // 大于
def FCC_UG  : FCC_VAL<21>;  // 无序或大于
...
```

（注意，`Sparc.h` 也定义了与相同 SPARC 条件码对应的枚举。必须注意确保 `Sparc.h` 中的值与 `SparcInstrInfo.td` 中的值对应，即 `SPCC::ICC_NE = 9`，`SPCC::FCC_U = 23` 等。）

### 指令操作数映射

代码生成器后端将指令操作数映射到指令中的字段。每当指令编码 `Inst` 中的某位被分配给没有具体值的字段时，`outs` 或 `ins` 列表中具有匹配名称的操作数就会填充该未定义字段。例如，Sparc 目标将 `XNORrr` 指令定义为具有三个操作数的 `F3_1` 格式指令：输出 `$rd`，以及输入 `$rs1` 和 `$rs2`：

```
def XNORrr  : F3_1<2, 0b000111,
                   (outs IntRegs:$rd), (ins IntRegs:$rs1, IntRegs:$rs2),
                   "xnor $rs1, $rs2, $rd",
                   [(set i32:$rd, (not (xor i32:$rs1, i32:$rs2)))]>;
```

`SparcInstrFormats.td` 中的指令模板显示了 `F3_1` 的基类 `InstSP`：

```
class InstSP<dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
  field bits<32> Inst;
  let Namespace = "SP";
  bits<2> op;
  let Inst{31-30} = op;
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
}
```

`InstSP` 定义了 `op` 字段，并使用它定义指令的第 30 和 31 位，但未为其赋值。

```
class F3<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstSP<outs, ins, asmstr, pattern> {
  bits<5> rd;
  bits<6> op3;
  bits<5> rs1;
  let op{1} = 1;   // Op = 2 或 3
  let Inst{29-25} = rd;
  let Inst{24-19} = op3;
  let Inst{18-14} = rs1;
}
```

`F3` 定义了 `rd`、`op3` 和 `rs1` 字段，并在指令中使用它们，同样没有赋值。

```
class F3_1<bits<2> opVal, bits<6> op3val, dag outs, dag ins,
           string asmstr, list<dag> pattern> : F3<outs, ins, asmstr, pattern> {
  bits<8> asi = 0; // asi 目前未使用
  bits<5> rs2;
  let op         = opVal;
  let op3        = op3val;
  let Inst{13}   = 0;     // i 字段 = 0
  let Inst{12-5} = asi;   // 地址空间标识符
  let Inst{4-0}  = rs2;
}
```

`F3_1` 为 `op` 和 `op3` 字段赋值，并定义了 `rs2` 字段。因此，`F3_1` 格式指令将需要 `rd`、`rs1` 和 `rs2` 的定义才能完全指定指令编码。

`XNORrr` 指令随后在其 OutOperandList 和 InOperandList 中提供了这三个操作数，它们绑定到相应的字段，从而完成指令编码。

对于某些指令，单个操作数可能包含子操作数。如前所示，指令 `LDrr` 使用类型为 `MEMrr` 的输入操作数。此操作数类型包含两个寄存器子操作数，由 `MIOperandInfo` 值定义为 `(ops IntRegs, IntRegs)`。

```
def LDrr : F3_1 <3, 0b000000, (outs IntRegs:$rd), (ins (MEMrr $rs1, $rs2):$addr),
                 "ld [$addr], $dst",
                 [(set i32:$dst, (load ADDRrr:$addr))]>;
```

由于此指令也是 `F3_1` 格式，它也需要名为 `rd`、`rs1` 和 `rs2` 的操作数。为了允许这一点，复杂操作数可以选择为其每个子操作数命名。在此示例中，`MEMrr` 的第一个子操作数名为 `$rs1`，第二个名为 `$rs2`，操作数整体也被命名为 `$addr`。

当特定指令不使用指令格式定义的所有操作数时，可以将常量值绑定到其中一个或所有操作数。例如，`RDASR` 指令只接受单个寄存器操作数，因此我们将常量零赋给 `rs2`：

```
let rs2 = 0 in
  def RDASR : F3_1<2, 0b101000,
                   (outs IntRegs:$rd), (ins ASRRegs:$rs1),
                   "rd $rs1, $rd", []>;
```

### 指令操作数名称映射

TableGen 还会生成一个名为 `getNamedOperandIdx()` 的函数，该函数可用于根据 TableGen 名称查找 MachineInstr 中操作数的索引。在指令的 TableGen 定义中设置 `UseNamedOperandTable` 位将把其所有操作数添加到枚举 `llvm::XXX::OpName` 中，并为其在 OperandMap 表中添加一个条目，该条目可使用 `getNamedOperandIdx()` 查询：

```cpp
int DstIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::dst); // => 0
int BIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::b);     // => 1
int CIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::c);     // => 2
int DIndex = SP::getNamedOperandIdx(SP::XNORrr, SP::OpName::d);     // => -1
```

OpName 枚举中的条目直接取自 TableGen 定义，因此具有小写名称的操作数在枚举中将有小写条目。

要在您的后端中包含 `getNamedOperandIdx()` 函数，您需要在 `XXXInstrInfo.cpp` 和 `XXXInstrInfo.h` 中定义一些预处理器宏。例如：

`XXXInstrInfo.cpp`：

```cpp
// 用于 getNamedOperandIdx() 函数定义
#define GET_INSTRINFO_NAMED_OPS
#include "XXXGenInstrInfo.inc"
```

`XXXInstrInfo.h`：

```cpp
// 用于 OpName 枚举和 getNamedOperandIdx 声明
#define GET_INSTRINFO_OPERAND_ENUM
#include "XXXGenInstrInfo.inc"
```

### 指令操作数类型

TableGen 还会生成一个枚举，由后端中定义的所有命名 Operand 类型组成，位于 `llvm::XXX::OpTypes` 命名空间中。一些常见的立即数 Operand 类型（例如 `i8`、`i32`、`i64`、`f32`、`f64`）在 `include/llvm/Target/Target.td` 中为所有目标定义，在每个目标的 OpTypes 枚举中均可用。此外，只有命名的 Operand 类型出现在枚举中：匿名类型被忽略。

例如，X86 后端定义了 `brtarget` 和 `brtarget8`，它们都是 TableGen `Operand` 类的实例，表示分支目标操作数：

```
def brtarget : Operand<OtherVT>;
def brtarget8 : Operand<OtherVT>;
```

这产生：

```cpp
namespace X86 {
namespace OpTypes {
enum OperandType {
  ...
  brtarget,
  brtarget8,
  ...
  i32imm,
  i64imm,
  ...
  OPERAND_TYPE_LIST_END
} // End namespace OpTypes
} // End namespace X86
```

按照典型的 TableGen 方式，要使用该枚举，您需要定义一个预处理器宏：

```cpp
#define GET_INSTRINFO_OPERAND_TYPES_ENUM // 用于 OpTypes 枚举
#include "XXXGenInstrInfo.inc"
```

### 指令调度

指令迭代器可以使用 `MCDesc::getSchedClass()` 查询。该值可以由 TableGen 在 `XXXGenInstrInfo.inc` 中生成的 `llvm::XXX::Sched` 命名空间中的枚举命名。调度类的名称与 `XXXSchedule.td` 中提供的名称相同，另加一个默认的 `NoItinerary` 类。

调度模型由 TableGen 通过 SubtargetEmitter 使用 `CodeGenSchedModels` 类生成。这与指定机器资源使用的迭代器方法不同。工具 `utils/schedcover.py` 可用于确定哪些指令已被调度模型描述覆盖，哪些未被覆盖。第一步是使用以下指令创建输出文件，然后在输出文件上运行 `schedcover.py`：

```shell
$ <src>/utils/schedcover.py <build>/lib/Target/AArch64/tblGenSubtarget.with
instruction, default, CortexA53Model, CortexA57Model, ...
ABSv16i8, WriteV, , , CyWriteV3, ...
```

要捕获生成调度模型的调试输出，切换到适当的目标目录并使用以下命令：

```shell
$ <build>/bin/llvm-tblgen -debug-only=subtarget-emitter -gen-subtarget \
  -I <src>/lib/Target/<target> -I <src>/include \
  -I <src>/lib/Target <src>/lib/Target/<target>/<target>.td \
  -o <build>/lib/Target/<target>/<target>GenSubtargetInfo.inc.tmp \
  > tblGenSubtarget.dbg 2>&1
```

要确认上述命令是否是所需命令，可以使用以下方法从构建中捕获确切的 TableGen 命令：

```shell
$ VERBOSE=1 make ...
```

然后在输出中搜索 `llvm-tblgen` 命令。

### 指令关系映射

此 TableGen 功能用于将指令相互关联。当您有多种指令格式并需要在指令选择后在它们之间切换时，它特别有用。整个功能由关系模型驱动，这些模型可以根据目标特定指令集在 `XXXInstrInfo.td` 文件中定义。关系模型使用 `InstrMapping` 类作为基类定义。TableGen 解析所有模型并使用指定信息生成指令关系映射。关系映射作为表格发射到 `XXXGenInstrInfo.inc` 文件中，以及查询它们的函数。有关如何使用此功能的详细信息，请参阅 `HowToUseInstrMappings` 文档。

### 实现 TargetInstrInfo 子类

最后一步是手动编写 `XXXInstrInfo` 的部分代码，该代码实现 `TargetInstrInfo.h` 中描述的接口。除非被覆盖，这些函数返回 `0`、布尔值或断言。以下是在 `SparcInstrInfo.cpp` 中为 SPARC 实现覆盖的函数列表：

- `isLoadFromStackSlot` —— 如果指定的机器指令是从栈槽的直接加载，则返回目标地址的寄存器编号和栈槽的 `FrameIndex`。

- `isStoreToStackSlot` —— 如果指定的机器指令是到栈槽的直接存储，则返回目标地址的寄存器编号和栈槽的 `FrameIndex`。

- `copyPhysReg` —— 在一对物理寄存器之间复制值。

- `storeRegToStackSlot` —— 将寄存器值存储到栈槽。

- `loadRegFromStackSlot` —— 从栈槽加载寄存器值。

- `storeRegToAddr` —— 将寄存器值存储到内存。

- `loadRegFromAddr` —— 从内存加载寄存器值。

- `foldMemoryOperand` —— 尝试将指定操作数的任何加载或存储指令合并为一条指令。

### 分支折叠与 If 转换

通过合并指令或消除永远不会到达的指令，可以提高性能。`XXXInstrInfo` 中的 `analyzeBranch` 方法可以实现为检查条件指令并删除不必要的指令。`analyzeBranch` 查看机器基本块（MBB）的末尾以寻找改进机会，如分支折叠和 if 转换。`BranchFolder` 和 `IfConverter` 机器函数 Pass（见 `lib/CodeGen` 目录中的源文件 `BranchFolding.cpp` 和 `IfConversion.cpp`）调用 `analyzeBranch` 来改进代表指令的控制流图。

`analyzeBranch` 的几个实现（针对 ARM、Alpha 和 X86）可以作为您自己的 `analyzeBranch` 实现的模型。由于 SPARC 没有实现有用的 `analyzeBranch`，下面显示 ARM 目标的实现。

`analyzeBranch` 返回一个布尔值并接受四个参数：

- `MachineBasicBlock &MBB` —— 要检查的传入块。

- `MachineBasicBlock *&TBB` —— 返回的目标块。对于计算结果为 true 的条件分支，`TBB` 是目标。

- `MachineBasicBlock *&FBB` —— 对于计算结果为 false 的条件分支，`FBB` 作为目标返回。

- `std::vector<MachineOperand> &Cond` —— 用于评估条件分支条件的操作数列表。

在最简单的情况下，如果块在没有分支的情况下结束，则它会顺序进入后继块。`TBB` 和 `FBB` 都未指定目标块，因此两个参数都返回 `NULL`。`analyzeBranch` 的开始（见下面 ARM 目标的代码）显示了函数参数和最简单情况的代码：

```cpp
bool ARMInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 std::vector<MachineOperand> &Cond) const
{
  MachineBasicBlock::iterator I = MBB.end();
  if (I == MBB.begin() || !isUnpredicatedTerminator(--I))
    return false;
```

如果块以单个无条件分支指令结束，则 `analyzeBranch`（如下所示）应在 `TBB` 参数中返回该分支的目标：

```cpp
  if (LastOpc == ARM::B || LastOpc == ARM::tB) {
    TBB = LastInst->getOperand(0).getMBB();
    return false;
  }
```

如果块以两个无条件分支结束，则第二个分支永远不会到达。在这种情况下，如下所示，删除最后一条分支指令，并在 `TBB` 参数中返回倒数第二条分支：

```cpp
  if ((SecondLastOpc == ARM::B || SecondLastOpc == ARM::tB) &&
      (LastOpc == ARM::B || LastOpc == ARM::tB)) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    I = LastInst;
    I->eraseFromParent();
    return false;
  }
```

块可能以单个条件分支指令结束，如果条件计算为 false，则顺序进入后继块。在这种情况下，`analyzeBranch`（如下所示）应在 `TBB` 参数中返回条件分支的目标，并在 `Cond` 参数中返回用于评估条件的操作数列表：

```cpp
  if (LastOpc == ARM::Bcc || LastOpc == ARM::tBcc) {
    // 块以顺序执行的条件分支结束
    TBB = LastInst->getOperand(0).getMBB();
    Cond.push_back(LastInst->getOperand(1));
    Cond.push_back(LastInst->getOperand(2));
    return false;
  }
```

如果块同时以条件分支和紧随其后的无条件分支结束，则 `analyzeBranch`（如下所示）应在 `TBB` 参数中返回条件分支目标（假设它对应于条件评估为 "`true`" 的情况），在 `FBB` 中返回无条件分支目标（对应于条件评估为 "`false`" 的情况）。用于评估条件的操作数列表应在 `Cond` 参数中返回：

```cpp
  unsigned SecondLastOpc = SecondLastInst->getOpcode();

  if ((SecondLastOpc == ARM::Bcc && LastOpc == ARM::B) ||
      (SecondLastOpc == ARM::tBcc && LastOpc == ARM::tB)) {
    TBB =  SecondLastInst->getOperand(0).getMBB();
    Cond.push_back(SecondLastInst->getOperand(1));
    Cond.push_back(SecondLastInst->getOperand(2));
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }
```

对于最后两种情况（以单个条件分支结束，或以一个条件分支和一个无条件分支结束），在 `Cond` 参数中返回的操作数可以传递给其他指令的方法，以创建新分支或执行其他操作。`analyzeBranch` 的实现需要辅助方法 `removeBranch` 和 `insertBranch` 来管理后续操作。

在大多数情况下，`analyzeBranch` 应该返回 false 表示成功。只有当方法不知道该怎么做时，例如块有三个终止分支，`analyzeBranch` 才应返回 true。如果 `analyzeBranch` 遇到无法处理的终止符（如间接分支），也可以返回 true。

---

## 指令选择器

LLVM 使用 `SelectionDAG` 来表示 LLVM IR 指令，`SelectionDAG` 的节点理想情况下表示原生目标指令。在代码生成期间，执行指令选择 Pass，将非原生 DAG 指令转换为原生目标特定指令。`XXXISelDAGToDAG.cpp` 中描述的 Pass 用于匹配模式并执行 DAG 到 DAG 的指令选择。可选地，可以定义一个 Pass（在 `XXXBranchSelector.cpp` 中）以对分支指令执行类似的 DAG 到 DAG 操作。之后，`XXXISelLowering.cpp` 中的代码替换或删除 `SelectionDAG` 中原生不支持（合法化）的操作和数据类型。

TableGen 使用以下目标描述输入文件生成指令选择代码：

- `XXXInstrInfo.td` —— 包含目标特定指令集中指令的定义，生成 `XXXGenDAGISel.inc`，该文件包含在 `XXXISelDAGToDAG.cpp` 中。

- `XXXCallingConv.td` —— 包含目标体系结构的调用和返回值约定，生成 `XXXGenCallingConv.inc`，该文件包含在 `XXXISelLowering.cpp` 中。

指令选择 Pass 的实现必须包含一个声明 `FunctionPass` 类或 `FunctionPass` 子类的头文件。在 `XXXTargetMachine.cpp` 中，Pass 管理器（PM）应将每个指令选择 Pass 添加到要运行的 Pass 队列中。

LLVM 静态编译器（`llc`）是可视化 DAG 内容的出色工具。要在特定处理阶段之前或之后显示 `SelectionDAG`，请使用 `llc` 的命令行选项。

要描述指令选择器行为，您应该在 `XXXInstrInfo.td` 中指令定义的最后一个参数中添加用于将 LLVM 代码降低到 `SelectionDAG` 的模式。例如，在 `SparcInstrInfo.td` 中，此条目定义了一个寄存器存储操作，最后一个参数描述了带有存储 DAG 运算符的模式：

```
def STrr  : F3_1< 3, 0b000100, (outs), (ins MEMrr:$addr, IntRegs:$src),
                 "st $src, [$addr]", [(store i32:$src, ADDRrr:$addr)]>;
```

`ADDRrr` 是一种内存模式，也在 `SparcInstrInfo.td` 中定义：

```
def ADDRrr : ComplexPattern<i32, 2, "SelectADDRrr", [], []>;
```

`ADDRrr` 的定义引用了 `SelectADDRrr`，这是在指令选择器实现（如 `SparcISelDAGToDAG.cpp`）中定义的函数。

在 `lib/Target/TargetSelectionDAG.td` 中，存储的 DAG 运算符定义如下：

```
def store : PatFrag<(ops node:$val, node:$ptr),
                    (unindexedstore node:$val, node:$ptr)> {
  let IsStore = true;
  let IsTruncStore = false;
}
```

`XXXInstrInfo.td` 还在 `XXXGenDAGISel.inc` 中生成 `SelectCode` 方法，该方法用于为指令调用适当的处理方法。在此示例中，`SelectCode` 为 `ISD::STORE` 操作码调用 `Select_ISD_STORE`：

```cpp
SDNode *SelectCode(SDValue N) {
  ...
  MVT::ValueType NVT = N.getNode()->getValueType(0);
  switch (N.getOpcode()) {
  case ISD::STORE: {
    switch (NVT) {
    default:
      return Select_ISD_STORE(N);
      break;
    }
    break;
  }
  ...
```

匹配了 `STrr` 的模式，因此在 `XXXGenDAGISel.inc` 的其他地方，为 `Select_ISD_STORE` 创建了 `STrr` 的代码。`Emit_22` 方法也在 `XXXGenDAGISel.inc` 中生成，以完成此指令的处理：

```cpp
SDNode *Select_ISD_STORE(const SDValue &N) {
  SDValue Chain = N.getOperand(0);
  if (Predicate_store(N.getNode())) {
    SDValue N1 = N.getOperand(1);
    SDValue N2 = N.getOperand(2);
    SDValue CPTmp0;
    SDValue CPTmp1;

    // 模式：(st:void i32:i32:$src, ADDRrr:i32:$addr)<<P:Predicate_store>>
    // 发射：(STrr:void ADDRrr:i32:$addr, IntRegs:i32:$src)
    // 模式复杂度 = 13  代价 = 1  大小 = 0
    if (SelectADDRrr(N, N2, CPTmp0, CPTmp1) &&
        N1.getNode()->getValueType(0) == MVT::i32 &&
        N2.getNode()->getValueType(0) == MVT::i32) {
      return Emit_22(N, SP::STrr, CPTmp0, CPTmp1);
    }
...
```

### SelectionDAG 合法化阶段

合法化阶段将 DAG 转换为使用目标原生支持的类型和操作。对于原生不支持的类型和操作，您需要在目标特定的 `XXXTargetLowering` 实现中添加代码，以将不支持的类型和操作转换为支持的类型和操作。

在 `XXXTargetLowering` 类的构造函数中，首先使用 `addRegisterClass` 方法指定支持哪些类型以及与它们关联的寄存器类。寄存器类的代码由 TableGen 从 `XXXRegisterInfo.td` 生成并放置在 `XXXGenRegisterInfo.h.inc` 中。例如，`SparcTargetLowering` 类的构造函数实现（在 `SparcISelLowering.cpp` 中）以以下代码开始：

```cpp
addRegisterClass(MVT::i32, SP::IntRegsRegisterClass);
addRegisterClass(MVT::f32, SP::FPRegsRegisterClass);
addRegisterClass(MVT::f64, SP::DFPRegsRegisterClass);
```

您应该检查 `ISD` 命名空间中的节点类型（`include/llvm/CodeGen/SelectionDAGNodes.h`），并确定目标原生支持哪些操作。对于**没有**原生支持的操作，向 `XXXTargetLowering` 类的构造函数添加回调，以便指令选择过程知道该怎么做。`TargetLowering` 类的回调方法（在 `llvm/Target/TargetLowering.h` 中声明）包括：

- `setOperationAction` —— 通用操作。
- `setLoadExtAction` —— 带扩展的加载。
- `setTruncStoreAction` —— 截断存储。
- `setIndexedLoadAction` —— 索引加载。
- `setIndexedStoreAction` —— 索引存储。
- `setConvertAction` —— 类型转换。
- `setCondCodeAction` —— 对给定条件码的支持。

注意：在较旧的版本中，使用 `setLoadXAction` 而不是 `setLoadExtAction`。此外，在较旧的版本中，`setCondCodeAction` 可能不受支持。请检查您的版本以了解具体支持哪些方法。

这些回调用于确定某个操作是否适用于指定类型（或类型）。在所有情况下，第三个参数是一个 `LegalAction` 类型枚举值：`Promote`、`Expand`、`Custom` 或 `Legal`。`SparcISelLowering.cpp` 包含所有四种 `LegalAction` 值的示例。

#### Promote（提升）

对于在给定类型上没有原生支持的操作，可以将指定类型提升为受支持的更大类型。例如，SPARC 不支持布尔值（`i1` 类型）的符号扩展加载，因此在 `SparcISelLowering.cpp` 中，以下第三个参数 `Promote` 在加载之前将 `i1` 类型值更改为更大类型：

```cpp
setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);
```

#### Expand（展开）

对于没有原生支持的类型，值可能需要进一步分解，而不是提升。对于没有原生支持的操作，可以使用其他操作的组合来达到类似效果。在 SPARC 中，浮点正弦和余弦三角运算通过展开到其他操作来支持，如第三个参数 `Expand` 向 `setOperationAction` 所示：

```cpp
setOperationAction(ISD::FSIN, MVT::f32, Expand);
setOperationAction(ISD::FCOS, MVT::f32, Expand);
```

#### Custom（自定义）

对于某些操作，简单的类型提升或操作展开可能不够。在某些情况下，必须实现特殊的内部函数。

例如，常量值可能需要特殊处理，或者操作可能需要在栈中溢出和恢复寄存器，并与寄存器分配器协作。

如 `SparcISelLowering.cpp` 代码所示，要执行从浮点值到有符号整数的类型转换，首先应以 `Custom` 作为第三个参数调用 `setOperationAction`：

```cpp
setOperationAction(ISD::FP_TO_SINT, MVT::i32, Custom);
```

在 `LowerOperation` 方法中，对于每个 `Custom` 操作，应添加一个 case 语句来指示要调用哪个函数。在以下代码中，`FP_TO_SINT` 操作码将调用 `LowerFP_TO_SINT` 方法：

```cpp
SDValue SparcTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) {
  switch (Op.getOpcode()) {
  case ISD::FP_TO_SINT: return LowerFP_TO_SINT(Op, DAG);
  ...
  }
}
```

最后，实现 `LowerFP_TO_SINT` 方法，使用 FP 寄存器将浮点值转换为整数：

```cpp
static SDValue LowerFP_TO_SINT(SDValue Op, SelectionDAG &DAG) {
  assert(Op.getValueType() == MVT::i32);
  Op = DAG.getNode(SPISD::FTOI, MVT::f32, Op.getOperand(0));
  return DAG.getNode(ISD::BITCAST, MVT::i32, Op);
}
```

#### Legal（合法）

`Legal` `LegalizeAction` 枚举值简单地表示操作**是**原生支持的。`Legal` 是默认条件，因此很少使用。在 `SparcISelLowering.cpp` 中，`CTPOP`（计算整数中设置位数的操作）的操作仅在 SPARC v9 上原生支持。以下代码为非 v9 SPARC 实现启用 `Expand` 转换技术：

```cpp
setOperationAction(ISD::CTPOP, MVT::i32, Expand);
...
if (TM.getSubtarget<SparcSubtarget>().isV9())
  setOperationAction(ISD::CTPOP, MVT::i32, Legal);
```

### 调用约定

为了支持目标特定的调用约定，`XXXGenCallingConv.td` 使用在 `lib/Target/TargetCallingConv.td` 中定义的接口（如 `CCIfType` 和 `CCAssignToReg`）。TableGen 可以获取目标描述符文件 `XXXGenCallingConv.td` 并生成头文件 `XXXGenCallingConv.inc`，该文件通常包含在 `XXXISelLowering.cpp` 中。您可以使用 `TargetCallingConv.td` 中的接口来指定：

- 参数分配的顺序。
- 参数和返回值的位置（即在栈上还是在寄存器中）。
- 可以使用哪些寄存器。
- 是调用者还是被调用者展开栈。

以下示例演示了 `CCIfType` 和 `CCAssignToReg` 接口的使用。如果 `CCIfType` 谓词为 true（即当前参数的类型是 `f32` 或 `f64`），则执行该操作。在这种情况下，`CCAssignToReg` 操作将参数值分配给第一个可用寄存器：`R0` 或 `R1`：

```
CCIfType<[f32,f64], CCAssignToReg<[R0, R1]>>
```

`SparcCallingConv.td` 包含目标特定返回值调用约定（`RetCC_Sparc32`）和基本的 32 位 C 调用约定（`CC_Sparc32`）的定义。`RetCC_Sparc32` 的定义（如下所示）指示哪些寄存器用于指定的标量返回类型。单精度浮点返回到寄存器 `F0`，双精度浮点返回到寄存器 `D0`。32 位整数在寄存器 `I0` 或 `I1` 中返回：

```
def RetCC_Sparc32 : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[I0, I1]>>,
  CCIfType<[f32], CCAssignToReg<[F0]>>,
  CCIfType<[f64], CCAssignToReg<[D0]>>
]>;
```

`SparcCallingConv.td` 中 `CC_Sparc32` 的定义引入了 `CCAssignToStack`，它将值分配给具有指定大小和对齐的栈槽。在以下示例中，第一个参数 4 表示槽的大小，第二个参数也是 4，表示沿 4 字节单位的栈对齐。（特殊情况：如果大小为零，则使用 ABI 大小；如果对齐为零，则使用 ABI 对齐。）

```
def CC_Sparc32 : CallingConv<[
  // 如果有空间，所有参数都在整数寄存器中传递
  CCIfType<[i32, f32, f64], CCAssignToReg<[I0, I1, I2, I3, I4, I5]>>,
  CCAssignToStack<4, 4>
]>;
```

`CCDelegateTo` 是另一个常用接口，它尝试查找指定的子调用约定，如果找到匹配，则调用它。在以下示例（在 `X86CallingConv.td` 中），`RetCC_X86_32_C` 的定义以 `CCDelegateTo` 结尾。在当前值被分配给寄存器 `ST0` 或 `ST1` 后，调用 `RetCC_X86Common`：

```
def RetCC_X86_32_C : CallingConv<[
  CCIfType<[f32], CCAssignToReg<[ST0, ST1]>>,
  CCIfType<[f64], CCAssignToReg<[ST0, ST1]>>,
  CCDelegateTo<RetCC_X86Common>
]>;
```

`CCIfCC` 是一个尝试将给定名称与当前调用约定匹配的接口。如果名称标识当前调用约定，则调用指定的操作。在以下示例（在 `X86CallingConv.td` 中），如果使用 `Fast` 调用约定，则调用 `RetCC_X86_32_Fast`；如果使用 `SSECall` 调用约定，则调用 `RetCC_X86_32_SSE`：

```
def RetCC_X86_32 : CallingConv<[
  CCIfCC<"CallingConv::Fast", CCDelegateTo<RetCC_X86_32_Fast>>,
  CCIfCC<"CallingConv::X86_SSECall", CCDelegateTo<RetCC_X86_32_SSE>>,
  CCDelegateTo<RetCC_X86_32_C>
]>;
```

`CCAssignToRegAndStack` 与 `CCAssignToReg` 相同，但在使用某个寄存器时还分配一个栈槽。基本上，它的工作方式类似于：`CCIf<CCAssignToReg<regList>, CCAssignToStack<size, align>>`：

```
class CCAssignToRegAndStack<list<Register> regList, int size, int align>
    : CCAssignToReg<regList> {
  int Size = size;
  int Align = align;
}
```

其他调用约定接口包括：

- `CCIf <predicate, action>` —— 如果谓词匹配，则应用操作。
- `CCIfInReg <action>` —— 如果参数标记了 "`inreg`" 属性，则应用操作。
- `CCIfNest <action>` —— 如果参数标记了 "`nest`" 属性，则应用操作。
- `CCIfNotVarArg <action>` —— 如果当前函数不接受可变数量的参数，则应用操作。
- `CCAssignToRegWithShadow <registerList, shadowList>` —— 类似于 `CCAssignToReg`，但带有影子寄存器列表。
- `CCPassByVal <size, align>` —— 将值分配给具有最小指定大小和对齐的栈槽。
- `CCPromoteToType <type>` —— 将当前值提升为指定类型。
- `CallingConv <[actions]>` —— 定义每个支持的调用约定。

---

## 汇编打印器

在代码发射阶段，代码生成器可以利用 LLVM pass 来产生汇编输出。为此，您需要实现一个打印器的代码，该打印器将 LLVM IR 转换为目标机器的 GAS 格式汇编语言，步骤如下：

1. **定义目标的所有汇编字符串**，将它们添加到 `XXXInstrInfo.td` 文件中定义的指令中（参见指令集一节）。TableGen 将生成一个输出文件（`XXXGenAsmWriter.inc`），其中包含 `XXXAsmPrinter` 类的 `printInstruction` 方法的实现。

2. **编写 `XXXTargetAsmInfo.h`**，包含 `XXXTargetAsmInfo` 类（`TargetAsmInfo` 的子类）的基本声明。

3. **编写 `XXXTargetAsmInfo.cpp`**，包含 `TargetAsmInfo` 属性的目标特定值，有时还包括方法的新实现。

4. **编写 `XXXAsmPrinter.cpp`**，实现执行 LLVM 到汇编转换的 `AsmPrinter` 类。

`XXXTargetAsmInfo.h` 中的代码通常是 `XXXTargetAsmInfo` 类的简单声明，用于 `XXXTargetAsmInfo.cpp`。类似地，`XXXTargetAsmInfo.cpp` 通常有几个 `XXXTargetAsmInfo` 替换值的声明，这些声明覆盖 `TargetAsmInfo.cpp` 中的默认值。例如，在 `SparcTargetAsmInfo.cpp` 中：

```cpp
SparcTargetAsmInfo::SparcTargetAsmInfo(const SparcTargetMachine &TM) {
  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  Data64bitsDirective = 0;  // .xword 仅在 V9 中支持
  ZeroDirective = "\t.skip\t";
  CommentString = "!";
  ConstantPoolSection = "\t.section \".rodata\",#alloc\n";
}
```

X86 汇编打印器实现（`X86TargetAsmInfo`）是目标特定 `TargetAsmInfo` 类使用覆盖方法 `ExpandInlineAsm` 的一个示例。

`AsmPrinter` 的目标特定实现写在 `XXXAsmPrinter.cpp` 中，实现了将 LLVM 转换为可打印汇编的 `AsmPrinter` 类。该实现必须包含以下头文件，这些头文件包含 `AsmPrinter` 和 `MachineFunctionPass` 类的声明。`MachineFunctionPass` 是 `FunctionPass` 的子类。

```cpp
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
```

作为 `FunctionPass`，`AsmPrinter` 首先调用 `doInitialization` 来设置 `AsmPrinter`。在 `SparcAsmPrinter` 中，实例化一个 `Mangler` 对象来处理变量名。

在 `XXXAsmPrinter.cpp` 中，必须为 `XXXAsmPrinter` 实现 `runOnMachineFunction` 方法（在 `MachineFunctionPass` 中声明）。在 `MachineFunctionPass` 中，`runOnFunction` 方法调用 `runOnMachineFunction`。`runOnMachineFunction` 的目标特定实现各不相同，但通常执行以下操作来处理每个机器函数：

- 调用 `SetupMachineFunction` 执行初始化。
- 调用 `EmitConstantPool` 将已溢出到内存的常量打印到输出流。
- 调用 `EmitJumpTableInfo` 打印当前函数使用的跳转表。
- 打印当前函数的标签。
- 打印函数的代码，包括基本块标签和指令的汇编（使用 `printInstruction`）。

`XXXAsmPrinter` 实现还必须包含 TableGen 在 `XXXGenAsmWriter.inc` 文件中输出的代码。`XXXGenAsmWriter.inc` 中的代码包含 `printInstruction` 方法的实现，该方法可能调用以下方法：

- `printOperand`
- `printMemOperand`
- `printCCOperand`（用于条件语句）
- `printDataDirective`
- `printDeclare`
- `printImplicitDef`
- `printInlineAsm`

`AsmPrinter.cpp` 中 `printDeclare`、`printImplicitDef`、`printInlineAsm` 和 `printLabel` 的实现通常足够打印汇编，不需要覆盖。

`printOperand` 方法使用一个长 `switch`/`case` 语句实现，用于操作数的类型：寄存器、立即数、基本块、外部符号、全局地址、常量池索引或跳转表索引。对于具有内存地址操作数的指令，应实现 `printMemOperand` 方法以生成正确的输出。类似地，`printCCOperand` 应用于打印条件操作数。

`doFinalization` 应在 `XXXAsmPrinter` 中覆盖，应调用它来关闭汇编打印器。在 `doFinalization` 期间，全局变量和常量被打印到输出中。

---

## 子目标支持

子目标支持用于将给定芯片集的指令集变体信息传达给代码生成过程。例如，LLVM SPARC 实现涵盖了 SPARC 微处理器体系结构的三个主要版本：版本 8（V8，32 位体系结构）、版本 9（V9，64 位体系结构）和 UltraSPARC 体系结构。V8 有 16 个双精度浮点寄存器，也可用作 32 个单精度或 8 个四精度寄存器。V8 也是纯大端的。V9 有 32 个双精度浮点寄存器，也可用作 16 个四精度寄存器，但不能用作单精度寄存器。UltraSPARC 体系结构将 V9 与 UltraSPARC 视觉指令集扩展相结合。

如果需要子目标支持，您应该为您的体系结构实现一个目标特定的 `XXXSubtarget` 类。此类应处理命令行选项 `-mcpu=` 和 `-mattr=`。

TableGen 使用 `Target.td` 和 `Sparc.td` 文件中的定义在 `SparcGenSubtarget.inc` 中生成代码。在 `Target.td` 中（如下所示），定义了 `SubtargetFeature` 接口。`SubtargetFeature` 接口的前 4 个字符串参数是特性名称、由特性设置的 XXXSubtarget 字段、XXXSubtarget 字段的值以及特性的描述。（第五个参数是其存在所暗示的特性列表，其默认值是空数组。）

如果该字段的值是字符串 "true" 或 "false"，则假定该字段是布尔值，并且只有一个 SubtargetFeature 应该引用它。否则，假定它是整数。整数值可以是枚举常量的名称。如果多个特性使用相同的整数字段，则该字段将设置为共享该字段的所有已启用特性的最大值。

```
class SubtargetFeature<string n, string f, string v, string d,
                       list<SubtargetFeature> i = []> {
  string Name = n;
  string FieldName = f;
  string Value = v;
  string Desc = d;
  list<SubtargetFeature> Implies = i;
}
```

在 `Sparc.td` 文件中，`SubtargetFeature` 用于定义以下特性：

```
def FeatureV9 : SubtargetFeature<"v9", "IsV9", "true",
                     "启用 SPARC-V9 指令">;
def FeatureV8Deprecated : SubtargetFeature<"deprecated-v8",
                     "UseV8DeprecatedInsts", "true",
                     "在 V9 模式下启用已弃用的 V8 指令">;
def FeatureVIS : SubtargetFeature<"vis", "IsVIS", "true",
                     "启用 UltraSPARC 视觉指令集扩展">;
```

在 `Sparc.td` 的其他地方，定义了 `Proc` 类，然后用于定义可能具有前面描述特性的特定 SPARC 处理器子类型：

```
class Proc<string Name, list<SubtargetFeature> Features>
  : Processor<Name, NoItineraries, Features>;

def : Proc<"generic",         []>;
def : Proc<"v8",              []>;
def : Proc<"supersparc",      []>;
def : Proc<"sparclite",       []>;
def : Proc<"f934",            []>;
def : Proc<"hypersparc",      []>;
def : Proc<"sparclite86x",    []>;
def : Proc<"sparclet",        []>;
def : Proc<"tsc701",          []>;
def : Proc<"v9",              [FeatureV9]>;
def : Proc<"ultrasparc",      [FeatureV9, FeatureV8Deprecated]>;
def : Proc<"ultrasparc3",     [FeatureV9, FeatureV8Deprecated]>;
def : Proc<"ultrasparc3-vis", [FeatureV9, FeatureV8Deprecated, FeatureVIS]>;
```

从 `Target.td` 和 `Sparc.td` 文件，生成的 `SparcGenSubtarget.inc` 指定用于标识特性的枚举值、表示 CPU 特性和 CPU 子类型的常量数组，以及解析设置指定子目标选项的特性字符串的 `ParseSubtargetFeatures` 方法。生成的 `SparcGenSubtarget.inc` 文件应包含在 `SparcSubtarget.cpp` 中。`XXXSubtarget` 方法的目标特定实现应遵循以下伪代码：

```cpp
XXXSubtarget::XXXSubtarget(const Module &M, const std::string &FS) {
  // 设置默认特性
  // 确定 CPU 的默认和用户指定特性
  // 调用 ParseSubtargetFeatures(FS, CPU) 解析特性字符串
  // 执行任何其他操作
}
```

---

## JIT 支持

目标机器的实现可以选择包含一个即时（JIT）代码生成器，该生成器将机器码和辅助结构作为二进制输出发射，这些输出可以直接写入内存。为此，通过执行以下步骤来实现 JIT 代码生成：

1. **编写 `XXXCodeEmitter.cpp` 文件**，包含一个机器函数 Pass，将目标机器指令转换为可重定位的机器码。

2. **编写 `XXXJITInfo.cpp` 文件**，实现目标特定代码生成活动（如发射机器码和存根）的 JIT 接口。

3. **修改 `XXXTargetMachine`**，使其通过 `getJITInfo` 方法提供 `TargetJITInfo` 对象。

编写 JIT 支持代码有几种不同的方法。例如，TableGen 和目标描述符文件可用于创建 JIT 代码生成器，但不是强制性的。对于 Alpha 和 PowerPC 目标机器，TableGen 用于生成 `XXXGenCodeEmitter.inc`，其中包含机器指令的二进制编码和用于访问这些编码的 `getBinaryCodeForInstr` 方法。其他 JIT 实现则不使用。

`XXXJITInfo.cpp` 和 `XXXCodeEmitter.cpp` 都必须包含 `llvm/CodeGen/MachineCodeEmitter.h` 头文件，该文件定义了 `MachineCodeEmitter` 类，其中包含多个回调函数的代码，这些函数将数据（以字节、字、字符串等形式）写入输出流。

### 机器码发射器

在 `XXXCodeEmitter.cpp` 中，`Emitter` 类的目标特定版本被实现为函数 Pass（`MachineFunctionPass` 的子类）。`runOnMachineFunction` 的目标特定实现（由 `MachineFunctionPass` 中的 `runOnFunction` 调用）遍历 `MachineBasicBlock`，调用 `emitInstruction` 处理每条指令并发射二进制代码。`emitInstruction` 主要通过对 `XXXInstrInfo.h` 中定义的指令类型进行 case 语句来实现。

例如，在 `X86CodeEmitter.cpp` 中，`emitInstruction` 方法围绕以下 `switch`/`case` 语句构建：

```cpp
switch (Desc->TSFlags & X86::FormMask) {
case X86II::Pseudo:  // 用于尚未实现的指令
   ...               // 或伪指令
   break;
case X86II::RawFrm:  // 用于具有固定操作码值的指令
   ...
   break;
case X86II::AddRegFrm: // 用于将一个寄存器操作数添加到操作码的指令
   ...
   break;
case X86II::MRMDestReg:// 用于使用 Mod/RM 字节指定目标（寄存器）的指令
   ...
   break;
case X86II::MRMDestMem:// 用于使用 Mod/RM 字节指定目标（内存）的指令
   ...
   break;
case X86II::MRMSrcReg: // 用于使用 Mod/RM 字节指定源（寄存器）的指令
   ...
   break;
case X86II::MRMSrcMem: // 用于使用 Mod/RM 字节指定源（内存）的指令
   ...
   break;
case X86II::MRM0r: case X86II::MRM1r:  // 用于操作 REGISTER r/m 操作数
case X86II::MRM2r: case X86II::MRM3r:  // 并使用 Mod/RM 字节和字段
case X86II::MRM4r: case X86II::MRM5r:  // 保存扩展操作码数据的指令
case X86II::MRM6r: case X86II::MRM7r:
   ...
   break;
case X86II::MRM0m: case X86II::MRM1m:  // 用于操作 MEMORY r/m 操作数
case X86II::MRM2m: case X86II::MRM3m:  // 并使用 Mod/RM 字节和字段
case X86II::MRM4m: case X86II::MRM5m:  // 保存扩展操作码数据的指令
case X86II::MRM6m: case X86II::MRM7m:
   ...
   break;
case X86II::MRMInitReg: // 用于源和目标是同一寄存器的指令
   ...
   break;
}
```

这些 case 语句的实现通常首先发射操作码，然后获取操作数。然后根据操作数，可以调用辅助方法来处理操作数。例如，在 `X86CodeEmitter.cpp` 中，对于 `X86II::AddRegFrm` case，第一个发射的数据（由 `emitByte`）是添加到寄存器操作数的操作码。然后提取表示机器操作数的对象 `MO1`。辅助方法如 `isImmediate`、`isGlobalAddress`、`isExternalSymbol`、`isConstantPoolIndex` 和 `isJumpTableIndex` 确定操作数类型。（`X86CodeEmitter.cpp` 还具有私有方法如 `emitConstant`、`emitGlobalAddress`、`emitExternalSymbolAddress`、`emitConstPoolAddress` 和 `emitJumpTableAddress`，这些方法将数据发射到输出流中。）

```cpp
case X86II::AddRegFrm:
  MCE.emitByte(BaseOpcode + getX86RegNum(MI.getOperand(CurOp++).getReg()));

  if (CurOp != NumOps) {
    const MachineOperand &MO1 = MI.getOperand(CurOp++);
    unsigned Size = X86InstrInfo::sizeOfImm(Desc);
    if (MO1.isImmediate())
      emitConstant(MO1.getImm(), Size);
    else {
      unsigned rt = Is64BitMode ? X86::reloc_pcrel_word
        : (IsPIC ? X86::reloc_picrel_word : X86::reloc_absolute_word);
      if (Opcode == X86::MOV64ri)
        rt = X86::reloc_absolute_dword;  // FIXME: 添加 X86II 标志？
      if (MO1.isGlobalAddress()) {
        bool NeedStub = isa<Function>(MO1.getGlobal());
        bool isLazy = gvNeedsLazyPtr(MO1.getGlobal());
        emitGlobalAddress(MO1.getGlobal(), rt, MO1.getOffset(), 0,
                          NeedStub, isLazy);
      } else if (MO1.isExternalSymbol())
        emitExternalSymbolAddress(MO1.getSymbolName(), rt);
      else if (MO1.isConstantPoolIndex())
        emitConstPoolAddress(MO1.getIndex(), rt);
      else if (MO1.isJumpTableIndex())
        emitJumpTableAddress(MO1.getIndex(), rt);
    }
  }
  break;
```

在前面的示例中，`XXXCodeEmitter.cpp` 使用变量 `rt`，它是一个 `RelocationType` 枚举，可用于重定位地址（例如，具有 PIC 基偏移的全局地址）。目标的 `RelocationType` 枚举在简短的目标特定 `XXXRelocations.h` 文件中定义。`RelocationType` 由 `XXXJITInfo.cpp` 中定义的 `relocate` 方法使用，以重写引用的全局符号的地址。

例如，`X86Relocations.h` 为 X86 地址指定以下重定位类型。在所有四种情况下，重定位值都被添加到内存中已有的值。对于 `reloc_pcrel_word` 和 `reloc_picrel_word`，还有一个额外的初始调整：

```cpp
enum RelocationType {
  reloc_pcrel_word = 0,    // 在调整 PC 位置后添加重定位值
  reloc_picrel_word = 1,   // 在调整 PIC 基后添加重定位值
  reloc_absolute_word = 2, // 绝对重定位；无需额外调整
  reloc_absolute_dword = 3 // 绝对重定位；无需额外调整
};
```

### 目标 JIT 信息

`XXXJITInfo.cpp` 实现了目标特定代码生成活动（如发射机器码和存根）的 JIT 接口。至少，`XXXJITInfo` 的目标特定版本实现以下内容：

- `getLazyResolverFunction` —— 初始化 JIT，给目标一个用于编译的函数。

- `emitFunctionStub` —— 为回调函数返回一个具有指定地址的原生函数。

- `relocate` —— 根据重定位类型更改引用的全局符号的地址。

- 回调函数，是在真实目标最初未知时使用的函数存根的包装器。

`getLazyResolverFunction` 通常很简单。它将传入参数作为全局 `JITCompilerFunction` 并返回将用作函数包装器的回调函数。对于 Alpha 目标（在 `AlphaJITInfo.cpp` 中），`getLazyResolverFunction` 的实现很简单：

```cpp
TargetJITInfo::LazyResolverFn AlphaJITInfo::getLazyResolverFunction(
                                            JITCompilerFn F) {
  JITCompilerFunction = F;
  return AlphaCompilationCallback;
}
```

对于 X86 目标，`getLazyResolverFunction` 的实现稍微复杂一些，因为它为具有 SSE 指令和 XMM 寄存器的处理器返回不同的回调函数。

回调函数首先保存然后恢复被调用者寄存器值、传入参数以及帧和返回地址。回调函数需要低级访问寄存器或栈，因此通常使用汇编实现。

---

*本文档翻译自 LLVM 官方文档 [Writing an LLVM Backend](https://llvm.org/docs/WritingAnLLVMBackend.html)。*
