# 副作用与推测执行

本文概述了 MLIR 如何对副作用进行建模，以及推测执行在 MLIR 中的工作方式。

本设计原理仅适用于在[CFG 区域](../LangRef.md/#control-flow-and-ssacfg-regions)中使用的操作。[图区域](../LangRef.md/#graph-regions)中的副作用建模待定（TBD）。

[TOC]

## 概述

许多 MLIR 操作除了消费和产生 SSA 值之外不表现出任何其他行为。这些操作可以在服从 SSA 支配要求的前提下与其他操作重新排序，并可在需要时被消除，甚至被引入（例如用于[重新物化](https://en.wikipedia.org/wiki/Rematerialization)）。

然而，一部分 MLIR 操作具有隐式行为，这些行为未反映在其 SSA 数据流语义中。这些操作需要特殊处理，在没有额外分析的情况下不能被重新排序、消除或引入。

本文引入了对这些操作的分类，并展示了这些操作在 MLIR 中是如何建模的。

## 分类

具有隐式行为的操作可以大致分为以下几类：

1. 具有内存效应的操作。这些操作对某些可变系统资源进行读写，例如堆、栈、硬件寄存器、控制台。它们还可能以其他方式与堆交互，例如分配和释放内存。例如：标准内存读写、`printf`（可建模为"写入"控制台并从输入缓冲区读取）。
1. 具有未定义行为的操作。这些操作在某些输入或某些情况下未被定义——我们不指定传入此类非法输入时会发生什么，而是说行为是未定义的，并可以假定它不会发生。在实践中，在这种情况下，这些操作可能做任何事，从产生垃圾结果到崩溃程序或破坏内存。例如：整数除法（除以零时具有 UB），从已释放指针加载。
1. 不终止的操作。例如条件始终为真的 `scf.while`。
1. 具有非局部控制流的操作。这些操作可能弹出其当前执行帧并直接返回到较早的帧。例如：`longjmp`，抛出异常的操作。

最后，一个给定的操作可能具有上述隐式行为的组合。操作执行期间隐式行为的组合可以是有序的。我们用"阶段（stage）"来标记"op"执行期间隐式行为的顺序。阶段编号较低的隐式行为比阶段编号较高的更早发生。

## 建模

对这些行为的建模必须走一条细线——我们需要赋能更复杂的 pass 来推理此类行为的细微之处，同时不给只需要粗粒度"该操作是否可以自由移动"查询的简单 pass 带来过多负担。

MLIR 有两个操作接口来表示这些隐式行为：

1. [`MemoryEffectsOpInterface` 操作接口](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Interfaces/SideEffectInterfaces.td#L26)用于追踪内存效应。
1. [`ConditionallySpeculatable` 操作接口](https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/Interfaces/SideEffectInterfaces.td#L105)用于追踪未定义行为和无限循环。

这两者都是操作接口，这意味着操作可以动态地对自身进行内省（例如通过检查输入类型或属性）来推断其具有的内存效应以及它们是否可推测执行。

我们目前尚无适当的建模来完整捕获非局部控制流语义。

### 资源层次结构与作用域

每个效应都作用于一个 `Resource`。资源形成层次结构：每个资源可以有一个父资源（`getParent()`），API 提供了 `isSubresourceOf()` 和 `isDisjointFrom()`，使 pass 能够确定两个效应是否可能冲突（例如 CSE 和别名分析使用不相交性来允许更多优化）。资源可以是*可寻址的*（`isAddressable()`），这意味着其上的效应可以与基于指针的内存发生别名；不可寻址资源（例如运行时状态）不与任何基于值的内存位置发生别名。规范定义和 API 位于 `mlir/Interfaces/SideEffectInterfaces.h` 中。

**作用域与限制。** 该机制*不*旨在用于具有特定地址或大小的细粒度区域，也不用于别名类/基于偏移量的消歧。这些关注点超出了资源层次结构的范围，应由别名分析或其他机制处理。

添加新操作时，请问：

1. 它是否从堆或栈读取或写入？它可能应该实现 `MemoryEffectsOpInterface`。
1. 这些副作用是否有序？该操作可能应该设置副作用的阶段以使分析更准确。
1. 这些副作用是否作用于资源的每个单独值？它可能应该在效应上设置 FullEffect。
1. 它是否具有必须保留的副作用，例如 volatile 存储或系统调用？它可能应该实现 `MemoryEffectsOpInterface` 并将效应建模为对抽象 `Resource` 的读取或写入。如果您的操作具有无法被 `MemoryEffectsOpInterface` 充分捕获的新型副作用，请发起 RFC。
1. 它在所有输入上是否都有良好定义，还是假设其输入具有某些运行时限制，例如指针操作数必须指向有效内存？它可能应该实现 `ConditionallySpeculatable`。
1. 它是否在某些输入上可能无限循环？它可能应该实现 `ConditionallySpeculatable`。
1. 它是否具有非局部控制流（例如 `longjmp`）？我们目前尚无适当的建模，欢迎提交补丁！
1. 您的操作是否没有副作用，可以自由提升、引入和消除？它可能应该被标记为 `Pure`。（待办事项：重新审视这个名称，因为它在 C++ 中有重载含义。）

## 示例

本节描述了一些非常简单的示例，帮助理解如何正确添加副作用。

### SIMD 计算操作

考虑一个 SIMD 后端方言中的"simd.abs"操作，它从源 memref 读取所有值，计算其绝对值，然后将其写入目标 memref：

```mlir
  func.func @abs(%source : memref<10xf32>, %target : memref<10xf32>) {
    simd.abs(%source, %target) : memref<10xf32> to memref<10xf32>
    return
  }
```

abs 操作从源资源读取每个单独的值，然后将这些值写入目标资源中每个对应的值。因此，我们需要为源指定读副作用，为目标指定写副作用。读副作用发生在写副作用之前，因此我们需要将读阶段标记为早于写阶段。此外，我们需要指示这些副作用适用于资源中的每个单独值。

典型做法如下：
``` mlir
  def AbsOp : SIMD_Op<"abs", [...] {
    ...

    let arguments = (ins Arg<AnyRankedOrUnrankedMemRef, "the source memref",
                             [MemReadAt<0, FullEffect>]>:$source,
                         Arg<AnyRankedOrUnrankedMemRef, "the target memref",
                             [MemWriteAt<1, FullEffect>]>:$target);

    ...
  }
```

在上面的例子中，我们将副作用 `[MemReadAt<0, FullEffect>]` 附加到源，表示 abs 操作在阶段 0 从源读取每个单独的值。类似地，我们将副作用 `[MemWriteAt<1, FullEffect>]` 附加到目标，表示 abs 操作在阶段 1（在从源读取之后）向目标内的每个单独值写入。

### 类加载操作

Memref.load 是一种典型的类加载操作：
```mlir
  func.func @foo(%input : memref<10xf32>, %index : index) -> f32 {
    %result = memref.load  %input[index] : memref<10xf32>
    return %result : f32
  }
```

类加载操作从输入 memref 中读取单个值并返回它。因此，我们需要为输入 memref 指定部分读副作用，表明并非每个单独的值都被使用。

典型做法如下：
``` mlir
  def LoadOp : MemRef_Op<"load", [...] {
    ...

    let arguments = (ins Arg<AnyMemRef, "the reference to load from",
                             [MemReadAt<0, PartialEffect>]>:$memref,
                         Variadic<Index>:$indices,
                         DefaultValuedOptionalAttr<BoolAttr, "false">:$nontemporal);

    ...
  }
```

在上面的例子中，我们将副作用 `[MemReadAt<0, PartialEffect>]` 附加到源，表示加载操作在阶段 0 从 memref 读取部分值。由于副作用通常在阶段 0 发生且默认为部分效应，我们可以将其简写为 `[MemRead]`。
