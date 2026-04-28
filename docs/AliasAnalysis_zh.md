# LLVM 别名分析基础设施

> 原文：https://llvm.org/docs/AliasAnalysis.html
> LLVM 23.0.0git 文档

---

## 目录

- [简介](#简介)
- [`AliasAnalysis` 类概览](#aliasanalysis-类概览)
  - [指针的表示方式](#指针的表示方式)
  - [`alias` 方法](#alias-方法)
    - [必定别名、可能别名与无别名响应](#必定别名可能别名与无别名响应)
  - [`getModRefInfo` 方法](#getmodrefinfo-方法)
  - [其他有用的 `AliasAnalysis` 方法](#其他有用的-aliasanalysis-方法)
    - [`getModRefInfoMask` 方法](#getmodrefinfomask-方法)
    - [`doesNotAccessMemory` 与 `onlyReadsMemory` 方法](#doesnotaccessmemory-与-onlyreadsmemory-方法)
- [编写新的 `AliasAnalysis` 实现](#编写新的-aliasanalysis-实现)
  - [不同的 Pass 风格](#不同的-pass-风格)
  - [必要的初始化调用](#必要的初始化调用)
  - [可以指定的接口](#可以指定的接口)
  - [`AliasAnalysis` 链式行为](#aliasanalysis-链式行为)
  - [为变换更新分析结果](#为变换更新分析结果)
    - [`deleteValue` 方法](#deletevalue-方法)
    - [`copyValue` 方法](#copyvalue-方法)
    - [`replaceWithNewValue` 方法](#replacewithnewvalue-方法)
    - [`addEscapingUse` 方法](#addescapinguse-方法)
  - [效率问题](#效率问题)
  - [局限性](#局限性)
- [使用别名分析结果](#使用别名分析结果)
  - [使用 `MemoryDependenceAnalysis` Pass](#使用-memorydependenceanalysis-pass)
  - [使用 `AliasSetTracker` 类](#使用-aliassettracker-类)
    - [AliasSetTracker 的实现](#aliassettracker-的实现)
  - [直接使用 `AliasAnalysis` 接口](#直接使用-aliasanalysis-接口)
- [现有的别名分析实现与客户端](#现有的别名分析实现与客户端)
  - [可用的 `AliasAnalysis` 实现](#可用的-aliasanalysis-实现)
    - [`-basic-aa` Pass](#-basic-aa-pass)
    - [`-globalsmodref-aa` Pass](#-globalsmodref-aa-pass)
    - [`-steens-aa` Pass](#-steens-aa-pass)
    - [`-ds-aa` Pass](#-ds-aa-pass)
    - [`-scev-aa` Pass](#-scev-aa-pass)
  - [别名分析驱动的变换](#别名分析驱动的变换)
    - [`-adce` Pass](#-adce-pass)
    - [`-licm` Pass](#-licm-pass)
    - [`-argpromotion` Pass](#-argpromotion-pass)
    - [`-gvn`、`-memcpyopt` 与 `-dse` Pass](#-gvn-memcpyopt-与--dse-pass)
  - [用于调试和评估实现的客户端](#用于调试和评估实现的客户端)
    - [`-print-alias-sets` Pass](#-print-alias-sets-pass)
    - [`-aa-eval` Pass](#-aa-eval-pass)
- [内存依赖分析](#内存依赖分析)

---

## 简介

别名分析（又称指针分析，Alias Analysis / Pointer Analysis）是一类技术，用于判断两个指针是否可能在内存中指向同一个对象。别名分析有许多不同的算法和分类方式：流敏感（flow-sensitive）vs. 流不敏感（flow-insensitive）、上下文敏感（context-sensitive）vs. 上下文不敏感（context-insensitive）、字段敏感（field-sensitive）vs. 字段不敏感（field-insensitive）、基于合一（unification-based）vs. 基于子集（subset-based）等等。传统上，别名分析对查询的响应为**必定别名（Must）**、**可能别名（May）**或**无别名（No Alias）**，分别表示两个指针总是指向同一对象、可能指向同一对象，或已知永远不指向同一对象。

LLVM 的 [AliasAnalysis](https://llvm.org/doxygen/classllvm_1_1AliasAnalysis.html) 类是 LLVM 系统中别名分析客户端和实现之间的主要接口。这个类是别名分析信息消费者与提供者之间的公共接口，被设计为支持广泛的实现和客户端（但目前所有客户端都被假定为流不敏感的）。除了简单的别名分析信息外，这个类还会从那些能够提供信息的实现中暴露 Mod/Ref 信息，从而使强大的分析和变换能够协同工作。

本文档包含成功实现、使用和测试该接口两端所需的信息，同时也解释了一些关于结果含义的细节。

---

## `AliasAnalysis` 类概览

[AliasAnalysis](https://llvm.org/doxygen/classllvm_1_1AliasAnalysis.html) 类定义了各种别名分析实现应支持的接口。该类导出两个重要的枚举：`AliasResult` 和 `ModRefResult`，分别表示别名查询或 mod/ref 查询的结果。

`AliasAnalysis` 接口以多种方式暴露内存相关信息。具体而言，内存对象以起始地址和大小来表示，函数调用以实际执行调用的 `call` 或 `invoke` 指令来表示。`AliasAnalysis` 接口还暴露了一些辅助方法，允许你为任意指令获取 mod/ref 信息。

所有 `AliasAnalysis` 接口都要求：在涉及多个值的查询中，非[常量](https://llvm.org/docs/LangRef.html#constants)的值必须都定义在同一函数内。

### 指针的表示方式

最重要的是，`AliasAnalysis` 类提供了若干方法，用于查询两个内存对象是否互为别名、函数调用是否可以修改或读取某个内存对象等。对于所有这些查询，内存对象均以一对（起始地址（LLVM `Value*` 符号值），静态大小）来表示。

将内存对象表示为起始地址和大小，对于正确的别名分析至关重要。例如，考虑如下（虽然愚蠢但可能出现的）C 代码：

```c
int i;
char C[2];
char A[10];
/* ... */
for (i = 0; i != 10; ++i) {
  C[0] = A[i];          /* 单字节存储 */
  C[1] = A[9-i];        /* 单字节存储 */
}
```

在这种情况下，`basic-aa` Pass 能够区分对 `C[0]` 和 `C[1]` 的存储，因为它们是对两个相距一字节的不同位置的访问，且每次访问只有一个字节。因此，循环不变代码外提（LICM）Pass 可以利用存储外提将这些存储移出循环。

相比之下，以下代码：

```c
int i;
char C[2];
char A[10];
/* ... */
for (i = 0; i != 10; ++i) {
  ((short*)C)[0] = A[i];  /* 双字节存储！ */
  C[1] = A[9-i];          /* 单字节存储 */
}
```

在这种情况下，对 C 的两次存储互为别名，因为对 `&C[0]` 元素的访问是双字节访问。如果查询中没有大小信息，即使是第一种情况也必须保守地假设这些访问互为别名。

### `alias` 方法

`alias` 方法是用于判断两个内存对象是否互为别名的主要接口。它接受两个内存对象作为输入，并酌情返回 `MustAlias`、`PartialAlias`、`MayAlias` 或 `NoAlias`。

与所有 `AliasAnalysis` 接口一样，`alias` 方法要求两个指针值要么定义在同一函数内，要么其中至少一个是[常量](https://llvm.org/docs/LangRef.html#constants)。

#### 必定别名、可能别名与无别名响应

当基于某指针的任何内存引用与基于另一指针的任何内存引用之间从不存在直接依赖时，可以使用 `NoAlias` 响应。最明显的例子是两个指针指向不重叠的内存范围；另一种情况是两个指针仅用于读取内存；还有一种情况是内存在通过一个指针访问和通过另一个指针访问之间被释放并重新分配——此时存在依赖，但由释放和重新分配所中介。

有一个例外：使用 [noalias](https://llvm.org/docs/LangRef.html#noalias) 关键字时，"无关"依赖会被忽略。

当两个指针可能引用同一对象时，使用 `MayAlias` 响应。

当两个内存对象已知在某种程度上重叠（无论它们是否从同一地址开始）时，使用 `PartialAlias` 响应。

`MustAlias` 响应只能在两个内存对象保证总是从完全相同的位置开始时才能返回。`MustAlias` 响应并不意味着指针值相等。

### `getModRefInfo` 方法

`getModRefInfo` 方法返回关于某条指令的执行是否可以读取或修改某个内存位置的信息。Mod/Ref 信息始终是保守的：如果某条指令**可能**读取或写入某个位置，则返回 `ModRef`。

`AliasAnalysis` 类还提供了一个用于测试函数调用之间依赖的 `getModRefInfo` 方法。该方法接受两个调用点（`CS1` 和 `CS2`），若两者都不向对方读写的内存写入，则返回 `NoModRef`；若 `CS1` 读取 `CS2` 写入的内存，则返回 `Ref`；若 `CS1` 向 `CS2` 读写的内存写入，则返回 `Mod`；若 `CS1` 可能读写 `CS2` 写入的内存，则返回 `ModRef`。注意，该关系不满足交换律。

### 其他有用的 `AliasAnalysis` 方法

许多别名分析实现通常会收集若干其他信息，这些信息可被各种客户端加以利用。

#### `getModRefInfoMask` 方法

`getModRefInfoMask` 方法根据指针是否指向全局常量内存（返回 `NoModRef`）或局部不变内存（返回 `Ref`），为所提供的指针返回 Mod/Ref 信息的上界。

全局常量内存包括函数、常量全局变量和空指针。局部不变内存是指我们知道在其 SSA 值生命周期内不变（但不一定在整个程序生命周期内不变）的内存——例如，`readonly noalias` 参数所指向的内存，在对应函数调用的持续时间内是已知不变的。

给定内存位置 `Loc` 的 Mod/Ref 信息 `MRI`，可以用如下语句来细化它：`MRI &= AA.getModRefInfoMask(Loc);`。另一个有用的惯用法是 `isModSet(AA.getModRefInfoMask(Loc))`，用于检查给定位置是否可以被修改。为方便起见，还有一个方法 `pointsToConstantMemory(Loc)`，等价于 `isNoModRef(AA.getModRefInfoMask(Loc))`。

#### `doesNotAccessMemory` 与 `onlyReadsMemory` 方法

这两个方法为函数调用提供非常简单的 mod/ref 信息。

`doesNotAccessMemory` 方法对某个函数返回 true，如果分析能够证明该函数从不读取或写入内存，或者该函数只从常量内存中读取。具有此属性的函数没有副作用，且仅依赖于其输入参数，从而可以在它们构成公共子表达式时被消除，或从循环中提出。许多常见函数以这种方式运作（如 `sin` 和 `cos`），但许多其他函数则不然（如 `acos`，它会修改 `errno` 变量）。

`onlyReadsMemory` 方法对某个函数返回 true，如果分析能够证明该函数（最多）只从非 volatile 内存中读取。具有此属性的函数没有副作用，仅依赖于其输入参数和被调用时的内存状态。只要没有改变内存内容的 store 指令，就可以消除和移动对这些函数的调用。注意，所有满足 `doesNotAccessMemory` 的函数也都满足 `onlyReadsMemory`。

---

## 编写新的 `AliasAnalysis` 实现

为 LLVM 编写新的别名分析实现相当直接。LLVM 中已有若干可供参考的实现，以下信息应能帮助你填补任何细节。例如，可以查看 LLVM 附带的[各种别名分析实现](#现有的别名分析实现与客户端)。

### 不同的 Pass 风格

第一步是确定你的别名分析需要使用哪种类型的 [LLVM Pass](https://llvm.org/docs/WritingAnLLVMPass.html)。与大多数其他分析和变换一样，答案应从你试图解决的问题类型中不难看出：

- 如果你需要过程间分析，应使用 `Pass`。
- 如果你是函数局部分析，应继承 `FunctionPass`。
- 如果你根本不需要查看程序，应继承 `ImmutablePass`。

除了你所继承的 Pass 之外，当然还应继承 `AliasAnalysis` 接口，并使用 `RegisterAnalysisGroup` 模板将自己注册为 `AliasAnalysis` 的实现。

### 必要的初始化调用

你的 `AliasAnalysis` 子类需要在 `AliasAnalysis` 基类上调用两个方法：`getAnalysisUsage` 和 `InitializeAliasAnalysis`。特别地，你对 `getAnalysisUsage` 的实现除了声明你的 Pass 所具有的 Pass 依赖关系外，还应显式调用 `AliasAnalysis::getAnalysisUsage` 方法。因此，你应该有类似这样的代码：

```cpp
void getAnalysisUsage(AnalysisUsage &AU) const {
  AliasAnalysis::getAnalysisUsage(AU);
  // 在此声明你的依赖关系
}
```

此外，你还必须从你的分析运行方法（`Pass` 的 `run`、`FunctionPass` 的 `runOnFunction` 或 `ImmutablePass` 的 `InitializePass`）中调用 `InitializeAliasAnalysis` 方法。例如（作为 `Pass` 的一部分）：

```cpp
bool run(Module &M) {
  InitializeAliasAnalysis(this);
  // 在此执行分析...
  return false;
}
```

### 可以指定的接口

所有 [AliasAnalysis](https://llvm.org/doxygen/classllvm_1_1AliasAnalysis.html) 虚方法默认会链式转发到另一个别名分析实现，最终返回保守正确的信息（对别名查询返回"May"别名，对 mod/ref 查询返回"Mod/Ref"）。根据你所实现的分析能力，你只需重写你能够改进的接口即可。

### `AliasAnalysis` 链式行为

每个别名分析 Pass 都会链式到另一个别名分析实现（例如，用户可以指定 "`-basic-aa -ds-aa -licm`" 以从两种别名分析中获得最大收益）。对于你未重写的方法，别名分析类会自动处理大部分链式工作。对于你确实重写的方法，在返回保守 MayAlias 或 Mod/Ref 结果的代码路径中，只需返回父类计算的结果即可。例如：

```cpp
AliasResult alias(const Value *V1, unsigned V1Size,
                  const Value *V2, unsigned V2Size) {
  if (...)
    return NoAlias;
  ...

  // 无法确定 MustAlias 或 NoAlias 结果
  return AliasAnalysis::alias(V1, V1Size, V2, V2Size);
}
```

除了分析查询之外，如果你重写了 LLVM [更新通知](#为变换更新分析结果)方法，还必须无条件地将调用转发给父类，以确保链中所有别名分析在变更时都能得到更新。

### 为变换更新分析结果

别名分析信息最初是针对程序的静态快照计算的，但客户端会使用这些信息来对代码进行变换。除了最简单的别名分析形式外，都需要更新其分析结果以反映这些变换所做的更改。

`AliasAnalysis` 接口暴露了四个方法，用于将程序变更从客户端传递给分析实现。各种别名分析实现应使用这些方法确保其内部数据结构随程序变化保持更新（例如，当某条指令被删除时），而别名分析的客户端必须确保适当地调用这些接口。

#### `deleteValue` 方法

当变换从程序中删除某条指令或任何其他值（包括不使用指针的值）时，会调用 `deleteValue` 方法。通常别名分析会维护针对程序中每个值都有条目的数据结构。当该方法被调用时，它们应删除指定值的所有条目（如果存在）。

#### `copyValue` 方法

当向程序中引入新值时使用 `copyValue` 方法。没有任何方式可以向程序中引入此前不存在的值（这对于安全的编译器变换没有意义），因此这是引入新值的唯一方式。该方法表示新值与被复制的值具有完全相同的属性。

#### `replaceWithNewValue` 方法

这是一个简单的辅助方法，旨在使客户端更易于使用。它通过将旧的分析信息复制到新值，然后删除旧值来实现。该方法不能被别名分析实现所重写。

#### `addEscapingUse` 方法

当指针值的使用方式发生变化，可能使预计算的分析信息失效时，使用 `addEscapingUse` 方法。实现可以使用此回调为自分析时间以来使用方式已改变的点提供保守响应，也可以重新计算部分或全部内部状态以继续提供精确响应。

一般来说，指针值的任何新使用都被视为逃逸使用，必须通过此回调报告，但以下使用除外：

- 对指针的 `bitcast` 或 `getelementptr`
- 通过指针的 `store`（但不是对指针本身的 `store`）
- 通过指针的 `load`

### 效率问题

从 LLVM 的角度来看，提供高效别名分析所需做的唯一一件事是确保别名分析**查询**能被快速响应。别名分析结果的实际计算（"run"方法）只执行一次，但可能会执行许多（可能是重复的）查询。因此，应尽量（在合理范围内）将尽可能多的计算移到 run 方法中。

### 局限性

AliasAnalysis 基础设施有若干局限性，使得编写新的 `AliasAnalysis` 实现较为困难。

**无法覆盖默认别名分析。** 理想情况下，能够执行类似 "`opt -my-aa -O2`" 并让它为所有需要 AliasAnalysis 的 Pass 使用 `-my-aa` 会非常有用，但目前没有对此的支持，除非修改源代码并重新编译。类似地，也没有办法将一系列分析设置为默认值。

**变换 Pass 无法声明它们保留了 `AliasAnalysis` 实现。** `AliasAnalysis` 接口包含 `deleteValue` 和 `copyValue` 方法，旨在允许 Pass 保持 AliasAnalysis 的一致性；然而，没有办法让 Pass 在其 `getAnalysisUsage` 中声明它这样做了。某些 Pass 尝试使用 `AU.addPreserved<AliasAnalysis>`；然而，这实际上没有任何效果。

类似地，`opt -p` 选项会在每个 Pass 之间引入 `ModulePass`，这阻止了 `FunctionPass` 别名分析 Pass 的使用。

`AliasAnalysis` API 确实具有在值被删除或复制时通知实现的函数；然而，这些函数并不够用。LLVM IR 还可以以许多其他方式被修改，这些修改可能与 `AliasAnalysis` 实现有关，但无法被表达。

`AliasAnalysisDebugger` 工具似乎表明 `AliasAnalysis` 实现可以期望在别名查询中出现任何相关 `Value` 之前会被通知到。然而，`GVN` 等常用客户端并不支持这一点，已知在与 `AliasAnalysisDebugger` 一起运行时会触发错误。

`AliasSetTracker` 类（被 `LICM` 使用）会发出不确定数量的别名查询。这可能导致涉及在预定数量的查询后暂停执行的调试技术变得不可靠。

许多别名查询可以用其他别名查询来重新表述。当多个 `AliasAnalysis` 查询链接在一起时，从链的开头开始这些查询（注意避免无限循环）是有意义的；然而，目前想要这样做的实现只能从它自身开始这些查询。

---

## 使用别名分析结果

有几种不同的方式来使用别名分析结果。按优先顺序排列如下：

### 使用 `MemoryDependenceAnalysis` Pass

`memdep` Pass 使用别名分析来提供关于使用内存的指令的高级依赖信息。例如，它会告诉你哪个 store 给某个 load 提供了数据。它使用缓存和其他技术来提高效率，并被死存储消除（Dead Store Elimination）、GVN 和 memcpy 优化所使用。

### 使用 `AliasSetTracker` 类

许多变换需要在某个作用域内活跃的别名**集合**信息，而不是关于成对别名关系的信息。[AliasSetTracker](https://llvm.org/doxygen/classllvm_1_1AliasSetTracker.html) 类用于从 `AliasAnalysis` 接口提供的成对别名分析信息中高效地构建这些别名集合。

首先，通过使用 `add` 方法向 AliasSetTracker 中添加关于你感兴趣的作用域内各种可能别名指令的信息来初始化它。一旦所有别名集合构建完成，你的 Pass 应使用 `AliasSetTracker` 的 `begin()`/`end()` 方法简单地迭代已构建的别名集合。

由 `AliasSetTracker` 形成的 `AliasSet` 保证是不相交的，会计算集合的 mod/ref 信息和 volatile 性，并跟踪集合中所有指针是否都是 Must 别名。AliasSetTracker 还确保由于调用指令而正确合并集合，并可以提供每个集合中的指针列表。

作为一个使用示例，[循环不变代码外提（LICM）](https://llvm.org/doxygen/structLICM.html) Pass 使用 `AliasSetTracker` 为每个循环嵌套计算别名集合。如果循环中的某个 `AliasSet` 未被修改，则来自该集合的所有 load 指令都可以被提出循环。如果某些别名集合被写入**且**是 must 别名集合，则这些 store 可以被沉降到循环外，在循环嵌套期间将内存位置提升为寄存器。这两种变换仅在指针参数是循环不变的情况下才适用。

#### AliasSetTracker 的实现

AliasSetTracker 类被实现为尽可能高效。它使用 union-find 算法在向 AliasSetTracker 插入与多个集合别名的指针时高效地合并 AliasSet。主要数据结构是一个将指针映射到其所在 AliasSet 的哈希表。

AliasSetTracker 类必须维护每个 AliasSet 中所有 LLVM `Value*` 的列表。由于哈希表已经有每个感兴趣的 LLVM `Value*` 的条目，因此 AliasSet 通过这些哈希表节点穿线链表，以避免不必要地分配内存，并使合并别名集合极为高效（链表合并是常数时间的）。

如果你只是 AliasSetTracker 的客户端，不需要理解这些细节，但如果你查看代码，希望这个简短的描述有助于理解为什么事情被如此设计。

### 直接使用 `AliasAnalysis` 接口

如果这两个工具类都不满足你的 Pass 需求，则应直接使用 `AliasAnalysis` 类暴露的接口。尽量使用高级方法（例如，尽可能使用 mod/ref 信息而不是直接使用 `alias` 方法），以获得最佳的精度和效率。

---

## 现有的别名分析实现与客户端

如果你要使用 LLVM 别名分析基础设施，应该了解哪些别名分析客户端和实现是可用的。特别地，如果你在实现别名分析，应该了解对于监控和评估不同实现很有用的[客户端](#用于调试和评估实现的客户端)。

### 可用的 `AliasAnalysis` 实现

本节列出 `AliasAnalysis` 接口的各种实现。所有这些实现都[链式](#aliasanalysis-链式行为)到其他别名分析实现。

#### `-basic-aa` Pass

`-basic-aa` Pass 是一个激进的局部分析，**知道**许多重要的事实：

- 不同的全局变量、栈分配和堆分配永远不会互为别名。
- 全局变量、栈分配和堆分配永远不与空指针互为别名。
- 结构体的不同字段不互为别名。
- 具有静态不同下标的数组索引不能互为别名。
- 许多常见的标准 C 库函数[从不访问内存或只读取内存](#doesnotaccessmemory-与-onlyreadsmemory-方法)。
- 明显指向常量全局变量的指针 "`pointToConstantMemory`"。
- 如果函数调用从不逃逸出分配它们的函数（这是自动数组的常见情况），则不能修改或引用栈分配。

#### `-globalsmodref-aa` Pass

此 Pass 为不"取得其地址"的内部全局变量实现了简单的上下文敏感 mod/ref 和别名分析。如果全局变量未取得其地址，则 Pass 知道没有指针与该全局变量别名。此 Pass 还跟踪它知道从不访问内存或从不读取内存的函数，允许某些优化（如 GVN）完全消除调用指令。

此 Pass 的真正威力在于它为调用指令提供了上下文敏感的 mod/ref 信息。这允许优化器知道对某个函数的调用不会破坏或读取全局变量的值，从而允许消除 load 和 store。

> **注意**：此 Pass 在其范围上有些受限（只支持未取得地址的全局变量），但分析速度非常快。

#### `-steens-aa` Pass

`-steens-aa` Pass 实现了过程间别名分析的著名"Steensgaard 算法"的变体。Steensgaard 算法是一种基于合一的、流不敏感的、上下文不敏感的、字段不敏感的别名分析，同时也具有很好的可扩展性（实际上是线性时间）。

LLVM 的 `-steens-aa` Pass 使用数据结构分析框架实现了 Steensgaard 算法的"推测字段**敏感**"版本。这在保持良好分析可扩展性的同时，提供了比标准算法高得多的精度。

> **注意**：`-steens-aa` 在可选的"poolalloc"模块中可用，不是 LLVM 核心的一部分。

#### `-ds-aa` Pass

`-ds-aa` Pass 实现了完整的数据结构分析算法。数据结构分析是一种模块化的、基于合一的、流不敏感的、上下文**敏感**的、推测字段**敏感**的别名分析，具有相当好的可扩展性，通常为 `O(n * log(n))`。

该算法能够响应全范围的别名分析查询，并可以提供上下文敏感的 mod/ref 信息。目前唯一未实现的主要功能是对 must-alias 信息的支持。

> **注意**：`-ds-aa` 在可选的"poolalloc"模块中可用，不是 LLVM 核心的一部分。

#### `-scev-aa` Pass

`-scev-aa` Pass 通过将别名分析查询转换为 ScalarEvolution 查询来实现它们。这使它比其他别名分析对 `getelementptr` 指令和循环归纳变量有更完整的理解。

### 别名分析驱动的变换

LLVM 包含若干由别名分析驱动的变换，可以与上述任何实现一起使用。

#### `-adce` Pass

`-adce` Pass 实现了激进死代码消除（Aggressive Dead Code Elimination），使用 `AliasAnalysis` 接口删除对没有副作用且未被使用的函数的调用。

#### `-licm` Pass

`-licm` Pass 实现了各种与循环不变代码外提（Loop Invariant Code Motion）相关的变换。它将 `AliasAnalysis` 接口用于若干不同的变换：

- 使用 mod/ref 信息将 load 指令提出循环（如果循环中没有指令修改被加载的内存）。
- 使用 mod/ref 信息将不写内存且是循环不变的函数调用提出循环。
- 使用别名信息将循环中被加载和存储的内存对象提升为寄存器（如果没有对该加载/存储内存位置的 may alias）。

#### `-argpromotion` Pass

`-argpromotion` Pass 将引用传递参数提升为按值传递。特别地，如果指针参数只被加载，则将加载的值而非地址传入函数。此 Pass 使用别名信息来确保从参数指针加载的值在函数入口和任何指针加载之间未被修改。

#### `-gvn`、`-memcpyopt` 与 `-dse` Pass

这些 Pass 使用 AliasAnalysis 信息来推断 load 和 store。

### 用于调试和评估实现的客户端

这些 Pass 对于评估各种别名分析实现很有用。可以通过如下命令使用它们：

```
% opt -ds-aa -aa-eval foo.bc -disable-output -stats
```

#### `-print-alias-sets` Pass

`-print-alias-sets` Pass 作为 `opt` 工具的一部分暴露出来，用于打印由 `AliasSetTracker` 类形成的别名集合。如果你正在使用 `AliasSetTracker` 类，这很有用。使用方式如下：

```
% opt -ds-aa -print-alias-sets -disable-output
```

#### `-aa-eval` Pass

`-aa-eval` Pass 简单地迭代函数中所有的指针对，并询问别名分析这些指针是否互为别名。这给出了别名分析精度的一个指标。统计数据会打印出找到的无别名/可能别名/必定别名的百分比（更精确的算法将具有更少的可能别名数量）。

---

## 内存依赖分析

> **注意**：我们目前正在将事物从 `MemoryDependenceAnalysis` 迁移到 [MemorySSA](https://llvm.org/docs/MemorySSA.html)，请尽量使用后者。

如果你只是想成为别名分析信息的客户端，可以考虑使用内存依赖分析（Memory Dependence Analysis）接口。MemDep 是构建在别名分析之上的懒性缓存层，能够回答给定指令在块内或跨块级别依赖于哪些先前的内存操作这一问题。由于其懒性和缓存策略，使用 MemDep 相较于直接访问别名分析可以显著提升性能。

---

*© 版权所有 2003-2026，LLVM 项目。最后更新于 2026-04-22。*
