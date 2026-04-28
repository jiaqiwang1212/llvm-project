# LLVM 分析与变换 Pass 文档

> 原文：<https://llvm.org/docs/Passes.html>
> 版本：LLVM 23.0.0git，最后更新：2026-04-22

---

## 目录

1. [简介](#简介)
2. [分析 Pass](#分析-pass)
   - [aa-eval：别名分析精度评估器](#aa-eval别名分析精度评估器)
   - [basic-aa：基本别名分析](#basic-aa基本别名分析)
   - [basiccg：基本调用图构建](#basiccg基本调用图构建)
   - [da：依赖分析](#da依赖分析)
   - [domfrontier：支配前沿构建](#domfrontier支配前沿构建)
   - [domtree：支配树构建](#domtree支配树构建)
   - [dot-callgraph：将调用图输出为 dot 文件](#dot-callgraph将调用图输出为-dot-文件)
   - [dot-cfg：将函数 CFG 输出为 dot 文件](#dot-cfg将函数-cfg-输出为-dot-文件)
   - [dot-cfg-only：将函数 CFG 输出为 dot 文件（无函数体）](#dot-cfg-only将函数-cfg-输出为-dot-文件无函数体)
   - [dot-dom：将支配树输出为 dot 文件](#dot-dom将支配树输出为-dot-文件)
   - [dot-dom-only：将支配树输出为 dot 文件（无函数体）](#dot-dom-only将支配树输出为-dot-文件无函数体)
   - [dot-post-dom：将后支配树输出为 dot 文件](#dot-post-dom将后支配树输出为-dot-文件)
   - [dot-post-dom-only：将后支配树输出为 dot 文件（无函数体）](#dot-post-dom-only将后支配树输出为-dot-文件无函数体)
   - [globals-aa：全局变量 mod/ref 分析](#globals-aa全局变量-modref-分析)
   - [instcount：统计指令数量](#instcount统计指令数量)
   - [iv-users：归纳变量使用者分析](#iv-users归纳变量使用者分析)
   - [kernel-info：GPU 内核信息](#kernel-infogpu-内核信息)
   - [lazy-value-info：惰性值信息分析](#lazy-value-info惰性值信息分析)
   - [lint：静态检查 LLVM IR](#lint静态检查-llvm-ir)
   - [loops：自然循环信息](#loops自然循环信息)
   - [memdep：内存依赖分析](#memdep内存依赖分析)
   - [print\<module-debuginfo\>：解码模块级调试信息](#printmodule-debuginfo解码模块级调试信息)
   - [postdomtree：后支配树构建](#postdomtree后支配树构建)
   - [print-alias-sets：别名集合打印器](#print-alias-sets别名集合打印器)
   - [print-callgraph：打印调用图](#print-callgraph打印调用图)
   - [print-callgraph-sccs：打印调用图的 SCC](#print-callgraph-sccs打印调用图的-scc)
   - [print-cfg-sccs：打印函数 CFG 的 SCC](#print-cfg-sccs打印函数-cfg-的-scc)
   - [function(print)：将函数打印到 stderr](#functionprint将函数打印到-stderr)
   - [module(print)：将模块打印到 stderr](#moduleprint将模块打印到-stderr)
   - [regions：检测单入口单出口区域](#regions检测单入口单出口区域)
   - [scalar-evolution：标量演化分析](#scalar-evolution标量演化分析)
   - [scev-aa：基于标量演化的别名分析](#scev-aa基于标量演化的别名分析)
   - [stack-safety：栈安全分析](#stack-safety栈安全分析)
3. [变换 Pass](#变换-pass)
   - [adce：激进死代码消除](#adce激进死代码消除)
   - [always-inline：always_inline 函数内联器](#always-inlinealways_inline-函数内联器)
   - [argpromotion：将引用传递参数提升为值传递](#argpromotion将引用传递参数提升为值传递)
   - [block-placement：基于 Profile 的基本块放置](#block-placement基于-profile-的基本块放置)
   - [break-crit-edges：打断 CFG 中的关键边](#break-crit-edges打断-cfg-中的关键边)
   - [codegenprepare：为代码生成优化](#codegenprepare为代码生成优化)
   - [constmerge：合并重复的全局常量](#constmerge合并重复的全局常量)
   - [dce：死代码消除](#dce死代码消除)
   - [deadargelim：死参数消除](#deadargelim死参数消除)
   - [dse：死存储消除](#dse死存储消除)
   - [function-attrs：推导函数属性](#function-attrs推导函数属性)
   - [globaldce：死全局变量消除](#globaldce死全局变量消除)
   - [globalopt：全局变量优化器](#globalopt全局变量优化器)
   - [gvn：全局值编号](#gvn全局值编号)
   - [indvars：规范化归纳变量](#indvars规范化归纳变量)
   - [inline：函数集成/内联](#inline函数集成内联)
   - [instcombine：合并冗余指令](#instcombine合并冗余指令)
   - [aggressive-instcombine：合并表达式模式](#aggressive-instcombine合并表达式模式)
   - [internalize：内部化全局符号](#internalize内部化全局符号)
   - [ipsccp：过程间稀疏条件常量传播](#ipsccp过程间稀疏条件常量传播)
   - [normalize：将 IR 规范化以便 diff](#normalize将-ir-规范化以便-diff)
   - [jump-threading：跳转线程化](#jump-threading跳转线程化)
   - [lcssa：循环封闭 SSA 形式](#lcssa循环封闭-ssa-形式)
   - [licm：循环不变代码外提](#licm循环不变代码外提)
   - [loop-deletion：删除死循环](#loop-deletion删除死循环)
   - [loop-extract：将循环提取为新函数](#loop-extract将循环提取为新函数)
   - [loop-reduce：循环强度削减](#loop-reduce循环强度削减)
   - [loop-rotate：循环旋转](#loop-rotate循环旋转)
   - [loop-simplify：规范化自然循环](#loop-simplify规范化自然循环)
   - [loop-unroll：循环展开](#loop-unroll循环展开)
   - [loop-unroll-and-jam：展开并融合循环](#loop-unroll-and-jam展开并融合循环)
   - [lower-global-dtors：降低全局析构函数](#lower-global-dtors降低全局析构函数)
   - [lower-atomic：将原子内联函数降为非原子形式](#lower-atomic将原子内联函数降为非原子形式)
   - [lower-invoke：将 invoke 降为 call](#lower-invoke将-invoke-降为-call)
   - [lower-switch：将 switch 指令降为分支](#lower-switch将-switch-指令降为分支)
   - [mem2reg：将内存引用提升为寄存器引用](#mem2reg将内存引用提升为寄存器引用)
   - [memcpyopt：memcpy 优化](#memcpyoptmemcpy-优化)
   - [mergefunc：合并函数](#mergefunc合并函数)
   - [mergereturn：统一函数出口节点](#mergereturn统一函数出口节点)
   - [partial-inliner：部分内联](#partial-inliner部分内联)
   - [reassociate：重新关联表达式](#reassociate重新关联表达式)
   - [rel-lookup-table-converter：相对查找表转换器](#rel-lookup-table-converter相对查找表转换器)
   - [reg2mem：将所有值降到栈槽](#reg2mem将所有值降到栈槽)
   - [sroa：聚合类型的标量替换](#sroa聚合类型的标量替换)
   - [sccp：稀疏条件常量传播](#sccp稀疏条件常量传播)
   - [simplifycfg：简化 CFG](#simplifycfg简化-cfg)
   - [sink：代码下沉](#sink代码下沉)
   - [simple-loop-unswitch：循环反开关](#simple-loop-unswitch循环反开关)
   - [strip：去除模块中的所有符号](#strip去除模块中的所有符号)
   - [strip-dead-debug-info：去除未使用符号的调试信息](#strip-dead-debug-info去除未使用符号的调试信息)
   - [strip-dead-prototypes：去除未使用的函数原型](#strip-dead-prototypes去除未使用的函数原型)
   - [strip-debug-declare：去除所有 llvm.dbg.declare 内联函数](#strip-debug-declare去除所有-llvmdbgdeclare-内联函数)
   - [strip-nondebug：去除非调试符号](#strip-nondebug去除非调试符号)
   - [tailcallelim：尾调用消除](#tailcallelim尾调用消除)
4. [工具 Pass](#工具-pass)
   - [deadarghaX0r：死参数黑客（仅供 bugpoint 使用）](#deadarghax0r死参数黑客仅供-bugpoint-使用)
   - [extract-blocks：从模块中提取基本块](#extract-blocks从模块中提取基本块)
   - [instnamer：为匿名指令赋名](#instnamer为匿名指令赋名)
   - [verify：模块验证器](#verify模块验证器)
   - [view-cfg：查看函数的 CFG](#view-cfg查看函数的-cfg)
   - [view-cfg-only：查看函数的 CFG（无函数体）](#view-cfg-only查看函数的-cfg无函数体)
   - [view-dom：查看函数的支配树](#view-dom查看函数的支配树)
   - [view-dom-only：查看函数的支配树（无函数体）](#view-dom-only查看函数的支配树无函数体)
   - [view-post-dom：查看函数的后支配树](#view-post-dom查看函数的后支配树)
   - [view-post-dom-only：查看函数的后支配树（无函数体）](#view-post-dom-only查看函数的后支配树无函数体)
   - [transform-warning：报告未应用的强制变换](#transform-warning报告未应用的强制变换)

---

## 简介

> **警告**
>
> 本文档更新不够频繁，Pass 列表很可能不完整。可以使用以下命令列出 `opt` 工具已知的所有 Pass：
> ```
> opt -print-passes
> ```

本文档是 LLVM 优化功能的高层次概要。优化通过 **Pass** 实现，每个 Pass 遍历程序的某个部分以收集信息或对程序进行变换。以下按三类划分 LLVM 提供的 Pass：

- **分析 Pass（Analysis Passes）**：计算供其他 Pass 使用的信息，或用于调试和程序可视化。
- **变换 Pass（Transform Passes）**：可以使用（或使之失效）分析 Pass 的结果，并以某种方式对程序进行修改。
- **工具 Pass（Utility Passes）**：提供某些实用功能，但不适合归入以上两类。例如，将函数提取为位码或将模块写入位码的 Pass，既不是分析 Pass 也不是变换 Pass。

---

## 分析 Pass

本节介绍 LLVM 的分析 Pass。

---

### aa-eval：别名分析精度评估器

一个简单的 N² 别名分析精度评估器。基本原理是：对程序中的每个函数，查询别名分析实现对函数中每对指针之间的别名关系如何作答。

灵感来源并改编自 Naveen Neelakantam、Francesco Spadini 和 Wojciech Stryjewski 的代码。

---

### basic-aa：基本别名分析

一个基本的别名分析 Pass，实现了一些恒等式（例如两个不同的全局变量不能互为别名），但不进行任何状态化分析。

---

### basiccg：基本调用图构建

尚未编写文档。

---

### da：依赖分析

依赖分析框架，用于检测内存访问中的依赖关系。

---

### domfrontier：支配前沿构建

一个用于求前向支配前沿的简单支配构建算法。

---

### domtree：支配树构建

一个用于求前向支配节点的简单支配构建算法。

---

### dot-callgraph：将调用图输出为 dot 文件

该 Pass 仅在 `opt` 中可用，将调用图输出为 `.dot` 图文件。可使用 `dot` 工具将其转换为 PostScript 或其他合适的格式。

---

### dot-cfg：将函数 CFG 输出为 dot 文件

该 Pass 仅在 `opt` 中可用，将控制流图输出为 `.dot` 图文件，然后可用 `dot` 工具将其转换为 PostScript 等格式。此外，`-cfg-func-name=<substring>` 选项可用于过滤要打印的函数——所有名称中包含指定子串的函数都会被打印。

---

### dot-cfg-only：将函数 CFG 输出为 dot 文件（无函数体）

该 Pass 仅在 `opt` 中可用，将控制流图输出为 `.dot` 图文件，但省略函数体。可用 `dot` 工具将其转换为 PostScript 等格式。同样支持 `-cfg-func-name=<substring>` 过滤选项。

---

### dot-dom：将支配树输出为 dot 文件

该 Pass 仅在 `opt` 中可用，将支配树输出为 `.dot` 图文件，可用 `dot` 工具转换为 PostScript 等格式。

---

### dot-dom-only：将支配树输出为 dot 文件（无函数体）

该 Pass 仅在 `opt` 中可用，将支配树输出为 `.dot` 图文件，但省略函数体，可用 `dot` 工具转换为 PostScript 等格式。

---

### dot-post-dom：将后支配树输出为 dot 文件

该 Pass 仅在 `opt` 中可用，将后支配树输出为 `.dot` 图文件，可用 `dot` 工具转换为 PostScript 等格式。

---

### dot-post-dom-only：将后支配树输出为 dot 文件（无函数体）

该 Pass 仅在 `opt` 中可用，将后支配树输出为 `.dot` 图文件，但省略函数体，可用 `dot` 工具转换为 PostScript 等格式。

---

### globals-aa：全局变量 mod/ref 分析

该简单 Pass 为未被取地址的全局值提供别名信息和 mod/ref 信息，并跟踪函数是否读取或写入内存（即是否为"纯函数"）。对于这种简单但极为常见的情况，可以提供相当准确且有用的信息。

---

### instcount：统计指令数量

该 Pass 统计所有指令的数量并报告结果。

---

### iv-users：归纳变量使用者分析

对由归纳变量计算的表达式的"有趣"使用者进行记录。

---

### kernel-info：GPU 内核信息

报告为 GPU 编译的代码的各种统计信息，该 Pass 有单独的文档。

---

### lazy-value-info：惰性值信息分析

提供值约束信息的惰性计算接口。

---

### lint：静态检查 LLVM IR

该 Pass 对 LLVM IR 中常见且容易识别的会产生未定义行为或可能非预期行为的构造进行静态检查。

它不能保证正确性，原因有二：其一，它并不全面——还有一些可以静态检查的条件尚未实现（部分以 TODO 注释标注，但也不完整）；其二，许多条件无法静态检查，该 Pass 不进行动态插桩，因此无法检查所有可能的问题。

另一个限制是它假设所有代码都会被执行。一个通过空指针进行的存储操作，如果位于永远不会执行的基本块中，实际上是无害的，但该 Pass 仍然会发出警告。

优化 Pass 可能会使该 Pass 所检查的条件变得更明显或更不明显。如果某个优化 Pass 看起来引入了警告，可能是该优化 Pass 只是暴露了代码中已有的问题。

该代码可能在 `instcombine` 之前运行。在许多情况下，`instcombine` 检查相同类型的问题，并将具有未定义行为的指令转换为 `unreachable`（或等价形式）。因此，该 Pass 会尝试透过位转换等操作进行查看。

---

### loops：自然循环信息

该分析用于识别自然循环，并确定 CFG 中各节点的循环深度。注意，识别出的循环实际上可能是共享同一头节点的多个自然循环，而不仅仅是单个自然循环。

---

### memdep：内存依赖分析

该分析针对给定的内存操作，确定它依赖于哪些前驱内存操作。它基于别名分析信息构建，并尝试为常见类型的别名信息查询提供一个惰性、带缓存的接口。

---

### print\<module-debuginfo\>：解码模块级调试信息

该 Pass 解码模块中的调试信息元数据，并以人类可读的形式打印到标准输出。

---

### postdomtree：后支配树构建

一个用于求后支配节点的简单后支配构建算法。

---

### print-alias-sets：别名集合打印器

尚未编写文档。

---

### print-callgraph：打印调用图

该 Pass 仅在 `opt` 中可用，以人类可读的形式将调用图打印到标准错误输出。

---

### print-callgraph-sccs：打印调用图的 SCC

该 Pass 仅在 `opt` 中可用，以人类可读的形式将调用图的强连通分量（SCC）打印到标准错误输出。

---

### print-cfg-sccs：打印函数 CFG 的 SCC

该 Pass 仅在 `opt` 中可用，以人类可读的形式将每个函数 CFG 的强连通分量打印到标准错误输出。

---

### function(print)：将函数打印到 stderr

`PrintFunctionPass` 类设计为与其他 `FunctionPass` 流水线配合使用，在处理模块中的函数时将其打印出来。

---

### module(print)：将模块打印到 stderr

该 Pass 在执行时将整个模块打印出来。

---

### regions：检测单入口单出口区域

`RegionInfo` Pass 检测函数中的单入口单出口区域，其中区域定义为任何与其余图仅通过两个点相连的子图。此外，还会构建一棵层次化的区域树。

---

### scalar-evolution：标量演化分析

`ScalarEvolution` 分析可用于分析和归类循环中的标量表达式，专门用于识别一般形式的归纳变量，并用抽象且不透明的 `SCEV` 类来表示它们。借助该分析，可以获得循环的迭代次数及其他重要属性。

该分析主要用于归纳变量替换和强度削减。

---

### scev-aa：基于标量演化的别名分析

基于 `ScalarEvolution` 查询实现的简单别名分析。

与传统的循环依赖分析不同，它测试的是循环**单次迭代内**的依赖关系，而非不同迭代之间的依赖。

`ScalarEvolution` 对指针算术的理解比 `BasicAliasAnalysis` 中各种特设分析更为完整。

---

### stack-safety：栈安全分析

`StackSafety` 分析可用于确定栈上分配的变量是否可以被认为不受内存访问漏洞的影响。

该分析的主要用途是供 sanitizer 使用，以避免对安全变量进行不必要的插桩。

---

## 变换 Pass

本节介绍 LLVM 的变换 Pass。

---

### adce：激进死代码消除

ADCE（Aggressive Dead Code Elimination）以激进方式尝试消除代码。该 Pass 类似于 DCE，但它假设值是死的，直到被证明是活的。这与 SCCP 类似，只是应用于值的活跃性分析。

---

### always-inline：always_inline 函数内联器

一个自定义内联器，仅处理标记为 `always inline` 的函数。

---

### argpromotion：将引用传递参数提升为值传递

该 Pass 将"引用传递"的参数提升为"值传递"的参数。实际操作是：查找具有指针参数的内部函数，如果能通过别名分析证明某个参数只被加载，则可以将值本身传入函数，而不是值的地址。这可以触发代码的递归简化，并导致 `alloca` 的消除（在 C++ 模板代码（如 STL）中尤为常见）。

该 Pass 还处理传入函数的聚合类型参数：如果聚合的元素只被加载，则对其进行标量化。注意，它拒绝对需要向函数传递超过三个操作数的聚合类型进行标量化，因为为大型数组或结构体传递数千个操作数是不合算的。

注意，该变换也可以用于只被存储到的参数（改为返回值），但目前尚未实现。当且仅当 LLVM 开始支持函数多返回值时，这种情况才能得到最好的处理。

---

### block-placement：基于 Profile 的基本块放置

一个非常简单的基于 Profile 的基本块放置算法。其思路是将频繁执行的基本块放在函数开头，以期增加直通条件分支的数量。如果某个函数没有 Profile 信息，该 Pass 基本上按深度优先顺序排列基本块。

---

### break-crit-edges：打断 CFG 中的关键边

通过插入一个虚拟基本块来打断 CFG 中的所有关键边。某些无法处理关键边的 Pass 可能会"需要"该 Pass。此变换会使 CFG 失效，但可以更新前向支配信息（集合、直接支配节点、树和前沿）。

---

### codegenprepare：为代码生成优化

该 Pass 对输入函数中的代码进行调整，以更好地为基于 SelectionDAG 的代码生成做准备，绕过其逐基本块处理方式的局限性。该 Pass 最终应被移除。

---

### constmerge：合并重复的全局常量

将重复的全局常量合并为一个共享的单一常量。这在某些 Pass（如 TraceValues）向程序中插入大量字符串常量时非常有用，而不管该字符串是否已存在。

---

### dce：死代码消除

死代码消除类似于死指令消除，但它会重新检查被移除指令所使用的指令，以判断它们是否也成为了死代码。

---

### deadargelim：死参数消除

该 Pass 从内部函数中删除死参数。死参数消除会删除直接死亡的参数，以及仅作为其他函数的死参数传入调用的参数。该 Pass 也以类似方式删除死参数。

该 Pass 通常用作清理 Pass，在执行了激进的过程间 Pass（可能会添加死参数）之后运行。

---

### dse：死存储消除

一个简单的死存储消除，仅考虑基本块本地的冗余存储。

---

### function-attrs：推导函数属性

一个简单的过程间 Pass，遍历调用图，查找不访问或只读取非局部内存的函数，并将其标记为 `readnone`/`readonly`。此外，它还将函数参数（指针类型）标记为 `nocapture`——如果调用该函数不会创建任何超过调用生命周期的指针值副本，即该指针只被解引用，既不从函数返回也不存储到全局变量中。该 Pass 以调用图的自底向上遍历方式实现。

---

### globaldce：死全局变量消除

该变换用于从程序中消除不可达的内部全局变量。它使用一种激进算法，搜索已知存活的全局变量。找到所有需要的全局变量后，删除其余内容，从而可以删除程序中不可达的递归代码块。

---

### globalopt：全局变量优化器

该 Pass 变换从未被取地址的简单全局变量。如果明显为真，则将读写全局变量标记为常量，删除仅被存储的变量等。

---

### gvn：全局值编号

该 Pass 执行全局值编号，以消除完全和部分冗余的指令，同时也执行冗余加载消除。

---

### indvars：规范化归纳变量

该变换分析并将归纳变量（及从中派生的计算）变换为更简单的形式，以便后续分析和变换。

对每个具有可识别归纳变量的循环，该变换进行以下更改：

- 将所有循环变换为具有单一规范归纳变量（从零开始，步长为一）。
- 规范归纳变量保证是循环头基本块中的第一个 PHI 节点。
- 所有指针算术递推都被提升为使用数组下标。

如果循环的迭代次数可计算，该 Pass 还会进行以下更改：

- 循环的退出条件被规范化为将归纳值与退出值进行比较，例如将：
  ```
  for (i = 7; i*i < 1000; ++i)
  ```
  变换为：
  ```
  for (i = 0; i != 25; ++i)
  ```
- 循环外对由归纳变量派生的表达式的任何使用，都被改为在循环外计算派生值，从而消除对归纳变量退出值的依赖。如果循环的唯一目的是计算某个派生表达式的退出值，该变换将使循环成为死代码。

在完成所有期望的循环变换后，应跟随强度削减 Pass。此外，在有利可图的目标上，循环可以被变换为向下计数到零（即"do 循环"优化）。

---

### inline：函数集成/内联

自底向上地将函数内联到调用者。

---

### instcombine：合并冗余指令

将指令合并为数量更少、形式更简单的指令。该 Pass 不修改 CFG，代数化简在此 Pass 中进行。

该 Pass 合并如下形式：

```llvm
%Y = add i32 %X, 1
%Z = add i32 %Y, 1
```

变换为：

```llvm
%Z = add i32 %X, 2
```

这是一个简单的工作列表驱动算法。

该 Pass 保证对程序执行以下规范化：

- 如果二元运算符有常量操作数，将其移至右侧。
- 带常量操作数的位运算符总是被分组，使得移位首先执行，然后是 or，再是 and，最后是 xor。
- 比较指令中，若可能则将 `<`、`>`、`≤`、`≥` 转换为 `=` 或 `≠`。
- 布尔值上的所有 `cmp` 指令替换为逻辑运算。
- `add X, X` 表示为 `mul X, 2` ⇒ `shl X, 1`。
- 与常量 2 的幂次方相乘的乘法变换为移位。
- 等等……

该 Pass 还可以简化对特定已知函数的调用（例如运行时库函数）。例如，在 `main()` 函数中调用 `exit(3)` 可以变换为简单的 `return 3`。是否简化库调用由 `-function-attrs` Pass 和 LLVM 对不同目标上库调用的了解来控制。

---

### aggressive-instcombine：合并表达式模式

将表达式模式合并为指令数量更少、形式更简单的表达式。

例如，该 Pass 将后置支配于 `TruncInst` 的表达式的宽度缩减为更小的宽度（如适用）。

它与 `instcombine` Pass 的区别在于：它可以修改 CFG，且包含复杂度高于 O(1) 的模式优化，因此运行次数应少于 `instcombine` Pass。

---

### internalize：内部化全局符号

该 Pass 遍历输入模块中的所有函数，查找 `main` 函数。如果找到 `main` 函数，则将所有其他函数以及所有带初始化器的全局变量标记为内部（`internal`）。

---

### ipsccp：过程间稀疏条件常量传播

稀疏条件常量传播的过程间变体。

---

### normalize：将 IR 规范化以便 diff

该 Pass 旨在通过重新排序和重命名指令（同时保持相同的语义）将 LLVM 模块变换为规范形式。规范化使得在比较经历了两个不同 Pass 处理的两个模块时，更容易发现语义差异。

---

### jump-threading：跳转线程化

跳转线程化尝试找出穿过基本块的不同控制流线程。该 Pass 查找具有多个前驱和多个后继的基本块。如果能证明一个或多个前驱总是会跳转到某个后继，则通过复制该基本块的内容，将从前驱到后继的边直接转发。

一个典型示例：

```c
if (...) { ...
  X = 4;
}
if (X < 3) {
```

在这种情况下，第一个 `if` 末尾的无条件跳转可以被重定向到第二个 `if` 的假分支。

---

### lcssa：循环封闭 SSA 形式

该 Pass 通过在循环结束处为所有跨越循环边界的活跃值放置 phi 节点来变换循环。例如，将左侧代码变换为右侧代码：

```
for (...)                for (...)
    if (c)                   if (c)
        X1 = ...                 X1 = ...
    else                     else
        X2 = ...                 X2 = ...
    X3 = phi(X1, X2)         X3 = phi(X1, X2)
... = X3 + 4              X4 = phi(X3)
                            ... = X4 + 4
```

这仍然是有效的 LLVM IR；额外的 phi 节点是纯冗余的，会被 `InstCombine` 轻松消除。该变换的主要好处是使许多其他循环优化（如 `LoopUnswitching`）更简单。更多内容请参阅循环术语中关于 LCSSA 形式的说明。

---

### licm：循环不变代码外提

该 Pass 执行循环不变代码外提，尽可能将代码从循环体中移出。它通过将代码提升到前驱头块（preheader block），或在安全的情况下将代码下沉到出口块来实现。该 Pass 还将循环中的必然别名（must-aliased）内存位置提升到寄存器中，从而提升和下沉"不变"的加载和存储。

将操作提升出循环是一种规范化变换，可以启用和简化中端的后续优化。将提升的指令重新物化（rematerialization）以降低寄存器压力是后端的职责，后端拥有更准确的寄存器压力信息，并且还处理 LICM 之外其他增加活跃范围的优化。

该 Pass 出于两个目的使用别名分析：

1. **将循环不变的加载和调用移出循环**：如果能确定循环内的一次加载或调用永远不与任何存储目标产生别名，则可以像其他指令一样将其提升或下沉。

2. **内存的标量提升**：如果循环内存在存储指令，尝试将该存储移到循环之后而不是循环内部。这只有在以下条件满足时才可能：
   - 被存储的指针是循环不变的。
   - 循环中没有可能与该指针产生别名的存储或加载，也没有 mod/ref 该指针的调用。

   如果满足这些条件，可以将循环中对该指针的加载和存储提升为使用一个临时 `alloca` 变量，然后使用 `mem2reg` 功能为该变量构建适当的 SSA 形式。

---

### loop-deletion：删除死循环

实现死循环删除 Pass。该 Pass 负责消除具有可计算有限迭代次数、无副作用或 volatile 指令、且不为函数返回值做贡献的循环。

---

### loop-extract：将循环提取为新函数

一个封装了 `ExtractLoop()` 标量变换的 Pass，将每个顶层循环提取到其自己的新函数中。如果给定函数中循环是唯一的循环，则不做处理。该 Pass 最常用于通过 `bugpoint` 进行调试。

---

### loop-reduce：循环强度削减

该 Pass 对循环内以一个或多个循环归纳变量作为分量的数组引用执行强度削减。具体方式是：为第一次迭代时数组访问的初始值创建一个新值，然后在循环中创建一个新的 GEP 指令以按适当的量递增该值。

---

### loop-rotate：循环旋转

一个简单的循环旋转变换。摘要可参阅循环术语中关于旋转循环的说明。

---

### loop-simplify：规范化自然循环

该 Pass 执行多项变换，将自然循环变换为更简单的形式，使后续分析和变换更简单、更有效。摘要可参阅循环术语中的循环简化形式说明。

- **循环前驱头插入**：保证从循环外部到循环头部存在单一的非关键入口边，简化了许多分析和变换，如 LICM。
- **循环出口块插入**：保证循环的所有出口块（循环外部、有循环内部前驱的块）只有来自循环内部的前驱（因此被循环头支配），简化了 LICM 中内置的存储下沉等变换。
- 该 Pass 还保证循环只有一条回边。

注意，`simplifycfg` Pass 会清理被分离出来但结果不必要的基本块，因此使用该 Pass 不应使生成的代码性能下降。

该 Pass 会修改 CFG，但会更新循环信息和支配信息。

---

### loop-unroll：循环展开

该 Pass 实现了一个简单的循环展开器。当循环已经被 `indvars` Pass 规范化后效果最佳，这样可以轻松确定循环的迭代次数。

---

### loop-unroll-and-jam：展开并融合循环

该 Pass 实现了经典的循环展开并融合（unroll and jam）优化。它将如下循环：

```
for i.. i+= 1              for i.. i+= 4
  for j..                    for j..
    code(i, j)                 code(i, j)
                               code(i+1, j)
                               code(i+2, j)
                               code(i+3, j)
                           remainder loop
```

可以理解为展开外层循环并将内层循环"融合"为一个。当新的内层循环中可以共享变量或加载时，这可以带来显著的性能提升。该 Pass 使用依赖分析来证明变换是安全的。

---

### lower-global-dtors：降低全局析构函数

该 Pass 通过创建包装函数来降低全局模块析构函数（`llvm.global_dtors`），这些包装函数被注册为 `llvm.global_ctors` 中的全局构造函数，并包含对 `__cxa_atexit` 的调用以注册其析构函数。

---

### lower-atomic：将原子内联函数降为非原子形式

该 Pass 将原子内联函数降低为非原子形式，用于已知的非可抢占环境。

该 Pass 不验证环境是否为非可抢占环境（通常这需要了解整个程序的调用图，包括可能以位码形式不可用的库）；它只是简单地降低每一个原子内联函数。

---

### lower-invoke：将 invoke 降为 call

该变换设计用于尚不支持栈展开的代码生成器。该 Pass 将 `invoke` 指令转换为 `call` 指令，使任何异常处理的 `landingpad` 块变为死代码（之后可以通过运行 `-simplifycfg` Pass 将其移除）。

---

### lower-switch：将 switch 指令降为分支

将 `switch` 指令重写为一系列分支，使目标可以在方便时再实现 `switch` 指令。

---

### mem2reg：将内存引用提升为寄存器引用

该文件将内存引用提升为寄存器引用，提升仅有加载和存储使用的 `alloca` 指令。`alloca` 通过使用支配前沿放置 phi 节点来变换，然后以深度优先顺序遍历函数以适当地重写加载和存储。这正是构建"剪枝"SSA 形式的标准 SSA 构建算法。

---

### memcpyopt：memcpy 优化

该 Pass 执行与消除 `memcpy` 调用或将一组存储变换为 `memset` 相关的各种变换。

---

### mergefunc：合并函数

该 Pass 查找可合并的等价函数并将其折叠。

在函数集合中引入全序关系：定义一种比较方式，对每两个函数回答哪个更大，从而可以将函数排列成二叉树。

对每个新函数，在树中检查是否存在等价函数。如果存在，则折叠这些函数。如果两个函数都可被覆盖（overridable），则将功能移入一个新的内部函数，并保留两个可覆盖的 thunk 指向它。如果不存在等价函数，则将该函数加入树中。

查找操作的复杂度为 O(log(n))，整个合并过程的复杂度为 O(n·log(n))。

详见[相关论文](https://research.google/pubs/pub36912/)。

---

### mergereturn：统一函数出口节点

确保函数中最多只有一条 `ret` 指令，并记录 CFG 的新出口节点。

---

### partial-inliner：部分内联

该 Pass 执行部分内联，通常是通过内联围绕函数体的 `if` 语句。

---

### reassociate：重新关联表达式

该 Pass 以一种有利于常量传播、GCSE、LICM、PRE 等优化的顺序重新关联交换性表达式。

例如：`4 + (x + 5)` ⇒ `x + (4 + 5)`

在该算法的实现中，常量的秩（rank）= 0，函数参数的秩 = 1，其他值的秩对应于当前函数的逆后序遍历（从 2 开始），这使得深层循环中的值具有比非循环中的值更高的秩。

---

### rel-lookup-table-converter：相对查找表转换器

该 Pass 将查找表转换为对 PIC 友好的相对查找表。

---

### reg2mem：将所有值降到栈槽

该文件将所有寄存器降低为内存引用，是 `mem2reg` 的逆操作。通过转换为加载指令，只有 `alloca` 指令和 phi 节点之前的加载指令才能跨基本块存活。目的是使 CFG 的修改更容易。为了方便后续修改，入口块被分为两个，使得所有引入的 `alloca` 指令（且仅此）位于入口块中。

---

### sroa：聚合类型的标量替换

著名的聚合类型标量替换变换。该变换将聚合类型（结构体或数组）的 `alloca` 指令拆分为每个成员的单独 `alloca` 指令（如可能），然后再尽可能地将各个 `alloca` 指令变换为干净的标量 SSA 形式。

---

### sccp：稀疏条件常量传播

稀疏条件常量传播与合并，可概括为：

- 假设值是常量，直到被证明不是
- 假设基本块是死的，直到被证明不是
- 证明值为常量，并用常量替换它们
- 证明条件分支为无条件分支

注意，该 Pass 往往会使定义变为死代码，因此在运行该 Pass 后运行一次 DCE Pass 是个好主意。

---

### simplifycfg：简化 CFG

执行死代码消除和基本块合并，具体包括：

- 移除没有前驱的基本块。
- 如果基本块只有一个前驱，且前驱只有一个后继，则将其合并到前驱中。
- 消除只有一个前驱的基本块中的 PHI 节点。
- 消除只包含无条件跳转的基本块。

---

### sink：代码下沉

该 Pass 在可能的情况下将指令移到后继块中，使其不在结果不被需要的路径上执行。

---

### simple-loop-unswitch：循环反开关

该 Pass 将包含循环不变条件分支的循环变换为具有多个循环的形式。例如，将左侧代码变换为右侧代码：

```
for (...)                  if (lic)
    A                          for (...)
    if (lic)                       A; B; C
        B                  else
    C                          for (...)
                                   A; C
```

这可能使代码量呈指数级增长（每次反开关都会使其翻倍），因此只有在结果代码小于某个阈值时才进行反开关。

该 Pass 需要在其之前运行 LICM，将不变条件提升出循环，使反开关的机会变得显而易见。

---

### strip：去除模块中的所有符号

执行代码去符号操作，可以删除：

- 虚拟寄存器的名称
- 内部全局变量和函数的符号
- 调试信息

注意，该变换会使代码可读性大大降低，因此只应在需要使用 `strip` 工具的场景下使用，例如减小代码体积或使代码更难被逆向工程。

---

### strip-dead-debug-info：去除未使用符号的调试信息

执行代码去符号操作，类似于 `strip`，但只去除未使用符号的调试信息。

---

### strip-dead-prototypes：去除未使用的函数原型

该 Pass 遍历输入模块中的所有函数，查找死声明并将其删除。死声明是指没有可用实现的函数声明（即未使用的库函数声明）。

---

### strip-debug-declare：去除所有 llvm.dbg.declare 内联函数和 #dbg_declare 记录

执行代码去符号操作，类似于 `strip`，但只去除 `llvm.dbg.declare` 内联函数。

---

### strip-nondebug：去除非调试符号

执行代码去符号操作，类似于 `strip`，但保留调试信息。

---

### tailcallelim：尾调用消除

该文件将当前函数（自递归）的调用后紧跟 `return` 指令的情况变换为跳转到函数入口，从而创建一个循环。该 Pass 还实现了以下基本算法的扩展：

- 调用和返回之间的普通指令不妨碍变换的进行，但目前分析不支持移动任何真正有用的指令（只能移动死代码）。
- 该 Pass 对因关联表达式而无法做尾递归的函数进行变换，使其使用累加变量，从而将典型的朴素阶乘或斐波那契实现编译为高效代码。
- 在以下情况下执行尾递归消除（TRE）：函数返回 `void`；返回值是调用的返回结果；或函数在所有出口处返回一个运行时常量。也有可能（虽然不太常见）返回其他内容（如常量 0），但仍可以进行 TRE——只要函数中所有其他 `return` 指令返回完全相同的值。
- 如果能证明被调用函数不访问其调用者的栈帧，则将其标记为适合尾调用消除（由代码生成器处理）。

---

## 工具 Pass

本节介绍 LLVM 的工具 Pass。

---

### deadarghaX0r：死参数黑客（仅供 bugpoint 使用）

与死参数消除相同，但也删除外部函数的参数。仅供 `bugpoint` 使用。

---

### extract-blocks：从模块中提取基本块

该 Pass 由 `bugpoint` 使用，将模块中的所有基本块提取到各自的函数中。

---

### instnamer：为匿名指令赋名

一个小型工具 Pass，为指令赋予名称。这在对比优化效果的 diff 时很有用，因为删除一条未命名的指令可能会改变其他所有指令的编号，使 diff 变得非常嘈杂。

---

### verify：模块验证器

验证 LLVM IR 代码。在测试中的优化 Pass 之后运行很有用。注意，`llvm-as` 在生成位码之前会验证其输入，并且格式错误的位码很可能会导致 LLVM 崩溃。因此，鼓励所有语言前端在执行优化变换之前验证其输出。

验证内容包括：

- 二元运算符的两个参数类型相同。
- 验证内存访问指令的索引与其他操作数匹配。
- 验证算术等操作只在一等类型（first-class types）上执行；移位和逻辑操作只在整数类型上发生等。
- `switch` 语句中所有常量的类型正确。
- 代码处于有效的 SSA 形式。
- 将标签放入其他类型（如结构体）或返回标签是非法的。
- 只有 phi 节点可以自引用：`%x = add i32 %x, %x` 是非法的。
- PHI 节点每个前驱必须有一个入口，不多不少。
- PHI 节点必须是基本块中的第一批内容，并全部聚集在一起。
- PHI 节点至少要有一个入口。
- 所有基本块只能以终结指令结束，而不能在中间包含终结指令。
- 函数的入口节点不能有前驱。
- 所有指令必须嵌入到某个基本块中。
- 函数不能接受 `void` 类型的参数。
- 验证函数的参数列表与其声明类型一致。
- 为 `void` 类型的值指定名称是非法的。
- 没有初始化器的内部全局值是非法的。
- `ret` 指令返回的值类型与函数返回值类型不匹配是非法的。
- 函数调用的参数类型与函数原型匹配。
- 代码中散布的断言（assert）所测试的所有其他内容。

注意，该 Pass 不提供完整的安全验证（如 Java），而是仅尝试确保代码格式良好。

---

### view-cfg：查看函数的 CFG

使用 GraphViz 工具显示控制流图。`-cfg-func-name=<substring>` 选项可用于过滤要显示的函数。

---

### view-cfg-only：查看函数的 CFG（无函数体）

使用 GraphViz 工具显示控制流图，但省略函数体。`-cfg-func-name=<substring>` 选项可用于过滤要显示的函数。

---

### view-dom：查看函数的支配树

使用 GraphViz 工具显示支配树。

---

### view-dom-only：查看函数的支配树（无函数体）

使用 GraphViz 工具显示支配树，但省略函数体。

---

### view-post-dom：查看函数的后支配树

使用 GraphViz 工具显示后支配树。

---

### view-post-dom-only：查看函数的后支配树（无函数体）

使用 GraphViz 工具显示后支配树，但省略函数体。

---

### transform-warning：报告未应用的强制变换

发出关于尚未应用的强制变换的警告（例如来自 `#pragma omp simd`）。

---

*© Copyright 2003-2026, LLVM Project. 原文最后更新于 2026-04-22。*
