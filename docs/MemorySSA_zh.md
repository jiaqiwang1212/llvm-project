# MemorySSA

## 简介

MemorySSA 是一种分析框架，允许我们以较低的开销推理各种内存操作之间的交互关系。其目标是替代 MemoryDependenceAnalysis（内存依赖分析）在大多数（乃至全部）使用场景中的作用。原因在于：如果使用不当，MemoryDependenceAnalysis 很容易导致 LLVM 中出现二次方时间复杂度的算法。此外，MemorySSA 没有 MemoryDependenceAnalysis 那么多的任意限制，因此通常也能得到更好的分析结果。MemorySSA 的一个常见用途是快速判断某件事情**肯定不会发生**（例如，推断某次循环外提操作是不合法的）。

从高层来看，MemorySSA 的目标之一是为内存提供一种基于 SSA 的表示形式，包含完整的 def-use 链和 use-def 链，使用户能够快速找到内存操作的 may-def（可能定义）和 may-use（可能使用）关系。它也可以被理解为一种以低开销对整个内存状态进行版本化的方法，并将内存操作与相应的版本关联起来。

本文介绍 MemorySSA 的结构，以及对 MemorySSA 工作原理的基本直觉。

关于 MemorySSA 的论文（含在 GCC 中实现的说明）可在[此处](https://dl.acm.org/doi/10.1145/3062341.3062382)找到。不过该论文已相对过时；论文中提到了多个内存分区，但 GCC 最终切换到只使用一个分区，与 LLVM 现在的做法一致。与 GCC 的实现类似，LLVM 的 MemorySSA 也是过程内（intraprocedural）的。

---

## MemorySSA 的结构

MemorySSA 是一种虚拟 IR。构建完成后，MemorySSA 会维护一个将 `Instruction`（指令）映射到 `MemoryAccess`（内存访问）的结构，`MemoryAccess` 是 MemorySSA 对 LLVM 指令的平行抽象。

每个 `MemoryAccess` 可以是以下三种类型之一：

- **MemoryDef**
- **MemoryPhi**
- **MemoryUse**

**MemoryDef** 是可能修改内存，或引入某种顺序约束的操作。MemoryDef 的例子包括：store 指令、函数调用、带有 acquire（或更强）顺序语义的 load 指令、volatile 操作、内存屏障等。每个 MemoryDef 总是引入整个内存的一个新版本，并与单个 MemoryDef/MemoryPhi 相链接，后者是新版本内存所基于的旧版本。这意味着存在一条单一的 **Def 链**，直接或间接地将所有 Def 连接起来。例如：

```
b = MemoryDef(a)
c = MemoryDef(b)
d = MemoryDef(c)
```

`d` 直接与 `c` 相连，间接与 `b` 相连。这意味着 `d` 有可能 clobber（覆盖）`c` 或 `b`，或两者都覆盖。由此推论，在不使用 Walker 的情况下，每个 MemoryDef 初始时都会 clobber 其他所有 MemoryDef。

**MemoryPhi** 是用于内存操作的 PhiNode。如果某个基本块有两个（或更多）MemoryDef 可能流入，则该块顶部的 MemoryAccess 将是一个 MemoryPhi。与 LLVM IR 中一样，MemoryPhi 不对应任何具体操作。因此，`BasicBlock` 在 MemorySSA 中映射到 MemoryPhi，而 `Instruction` 则映射到 MemoryUse 和 MemoryDef。

还需注意：在 SSA 中，Phi 节点合并的是**必然到达**的定义（即变量的新版本必然存在）；而在 MemorySSA 中，PHI 节点合并的是**可能到达**的定义（即在消歧之前，到达 phi 节点的版本可能会也可能不会 clobber 某个变量）。

**MemoryUse** 是使用但不修改内存的操作。MemoryUse 的例子包括 load 指令或只读函数调用。

每个函数都有一个特殊的 MemoryDef，称为 **liveOnEntry**。它支配该函数中 MemorySSA 所运行的所有 MemoryAccess，并表示我们已到达函数顶部。它是唯一一个不映射到任何 LLVM IR 指令的 MemoryDef。使用 liveOnEntry 意味着被使用的内存要么是未定义的，要么是在函数开始之前就已定义的。

以下是将上述内容叠加到 LLVM IR 上的一个例子（通过在 `.ll` 文件上运行 `opt -passes='print<memoryssa>' -disable-output` 获得）。查看此示例时，从 **clobber（覆盖）**的角度来理解可能会有所帮助：给定 MemoryAccess 的操作数是该 MemoryAccess 的所有（潜在）clobber，而 MemoryAccess 产生的值可以作为其他 MemoryAccess 的 clobber。

如果一个 MemoryAccess 是另一个的 clobber，则意味着这两个 MemoryAccess 可能访问相同的内存。例如，`x = MemoryDef(y)` 表示 `x` 可能修改了 `y` 修改/约束（或已修改/约束）的内存。

类似地，`a = MemoryPhi({BB1,b},{BB2,c})` 表示使用 `a` 的人正在访问可能被 `b` 或 `c`（或两者）修改/约束的内存。最后，`MemoryUse(x)` 表示该 use 访问的内存已被 `x` 修改/约束（例如，如果 `x = MemoryDef(...)` 和 `MemoryUse(x)` 在同一个循环中，则该 use 不能单独被提升到循环外）。

另一种有用的理解角度是**内存版本化**：给定 MemoryAccess 的操作数是操作前整个内存的版本；如果该 access 产生一个值（即 MemoryDef/MemoryPhi），则该值是操作后内存的新版本。

```llvm
define void @foo() {
entry:
  %p1 = alloca i8
  %p2 = alloca i8
  %p3 = alloca i8
  ; 1 = MemoryDef(liveOnEntry)
  store i8 0, ptr %p3
  br label %while.cond

while.cond:
  ; 6 = MemoryPhi({entry,1},{if.end,4})
  br i1 undef, label %if.then, label %if.else

if.then:
  ; 2 = MemoryDef(6)
  store i8 0, ptr %p1
  br label %if.end

if.else:
  ; 3 = MemoryDef(6)
  store i8 1, ptr %p2
  br label %if.end

if.end:
  ; 5 = MemoryPhi({if.then,2},{if.else,3})
  ; MemoryUse(5)
  %1 = load i8, ptr %p1
  ; 4 = MemoryDef(5)
  store i8 2, ptr %p2
  ; MemoryUse(1)
  %2 = load i8, ptr %p3
  br label %while.cond
}
```

MemorySSA IR 以注释形式显示在其所映射的指令（如果存在）之前。例如，`1 = MemoryDef(liveOnEntry)` 是一个 MemoryAccess（具体是 MemoryDef），描述的是 LLVM 指令 `store i8 0, ptr %p3`。MemorySSA 的其他地方用数字 `1` 来引用这个特定的 MemoryDef（就像在 LLVM 中可以用 `%1` 引用 `load i8, ptr %p1` 一样）。MemoryPhi 不对应任何 LLVM 指令，因此紧跟在 MemoryPhi 之后的那一行并不特殊。

从上到下依次解释：

- **`6 = MemoryPhi({entry,1},{if.end,4})`**：表示进入 `while.cond` 时，到达的定义要么是 `1`，要么是 `4`。该 MemoryPhi 在文本 IR 中用数字 `6` 引用。
- **`2 = MemoryDef(6)`**：表示 `store i8 0, ptr %p1` 是一个定义，其前驱到达定义是 `6`，即 `while.cond` 后的 MemoryPhi。（关于为何该 MemoryDef 没有链接到独立的、已消歧的 MemoryPhi，请参阅下文的 Use 和 Def 优化及精度章节。）
- **`3 = MemoryDef(6)`**：表示 `store i8 0, ptr %p2` 是一个定义，其到达定义同样是 `6`。
- **`5 = MemoryPhi({if.then,2},{if.else,3})`**：表示该块之前的 clobber 可能是 `2` 或 `3`。
- **`MemoryUse(5)`**：表示 `load i8, ptr %p1` 是一个内存 use，被 `5` clobber。
- **`4 = MemoryDef(5)`**：表示 `store i8 2, ptr %p2` 是一个定义，其到达定义是 `5`。
- **`MemoryUse(1)`**：表示 `load i8, ptr %p3` 只是一个内存 use，最后可能 clobber 该 use 的操作在 `while.cond` 之前（即对 `%p3` 的 store）。从内存版本化的角度看，它只依赖于内存版本 `1`，不受此后生成的新内存版本的影响。

顺便一提，`MemoryAccess` 继承自 `Value` 主要是为了方便起见，它并不打算与 LLVM IR 直接交互。

---

## MemorySSA 的设计

MemorySSA 是一种可以为任意函数构建的分析。构建时，它会遍历函数的 IR 以建立 MemoryAccess 的映射。随后你可以向 MemorySSA 查询诸如 MemoryAccess 之间的支配关系，以及任意给定 `Instruction` 对应的 MemoryAccess 等信息。

MemorySSA 构建完成后，还会提供一个 `MemorySSAWalker`（Walker）供你使用（见下文）。

### Walker（遍历器）

辅助 MemorySSA 工作的关键结构是 `MemorySSAWalker`（简称 Walker）。Walker 的目标是对 clobber 查询提供超出 MemoryAccess 直接表示范围的答案。例如：

```llvm
define void @foo() {
  %a = alloca i8
  %b = alloca i8

  ; 1 = MemoryDef(liveOnEntry)
  store i8 0, ptr %a
  ; 2 = MemoryDef(1)
  store i8 0, ptr %b
}
```

对 `%b` 的 store 显然不是对 `%a` 的 store 的 clobber。Walker 的任务就是找出这一点，并在查询 MemoryAccess `2` 的 clobber 时返回 `liveOnEntry`。

默认情况下，MemorySSA 提供的 Walker 可以通过查询你所使用的任意别名分析栈来优化 MemoryDef 和 MemoryUse。Walker 被设计为可扩展的，因此完全可以（也被期望）创建更专用的 Walker（例如，专门查询 GlobalsAA 的 Walker，或总是在 MemoryPhi 节点处停止的 Walker 等）。

### 默认 Walker API

使用 Walker 检索 clobbering access 主要有两个 API：

- **`MemoryAccess *getClobberingMemoryAccess(MemoryAccess *MA)`**：返回 `MA` 的 clobbering memory access，并将沿途查询的所有中间结果作为每个被查询 access 的一部分缓存起来。
- **`MemoryAccess *getClobberingMemoryAccess(MemoryAccess *MA, const MemoryLocation &Loc)`**：从 `MA` 开始，返回 clobber 内存位置 `Loc` 的 access。由于该 API 请求的不是特定 memory access 的 clobbering access，因此没有可以缓存的结果。

### 自行定位 clobber

如果你选择自己编写 Walker，可以通过遍历支配给定 MemoryAccess 的每个 MemoryDef 来找到该 MemoryAccess 的 clobber。MemoryDef 的结构使这相对简单：它们最终形成一个链表，列出所有支配你试图优化的 MemoryAccess 的 clobber。换句话说，MemoryDef 的 `definingAccess` 始终是该 MemoryDef 最近的支配 MemoryDef 或 MemoryPhi。

---

## Use 和 Def 优化

`MemoryUse` 保存单个操作数，即其定义或优化后的 access。

传统上，MemorySSA 在构建时会对 MemoryUse 进行优化（有一定阈值限制）。具体而言，每个 MemoryUse 的操作数都会被优化为指向该 MemoryUse 的实际 clobber。这在上面的例子中可以看到：`if.end` 中第二个 MemoryUse 的操作数是 `1`，即来自 entry 块的 MemoryDef。这样做是为了使遍历、值编号等操作更快、更容易。

从[此次修订](https://reviews.llvm.org/D121381)起，默认行为已更改为在构建时**不**优化 use，以便在某个 pass 不需要遍历时减少编译时间。大多数用户调用新的 API `ensureOptimizedUses()` 来保持之前的行为，并对 MemoryUse 进行一次性优化（如果之前未做过的话）。建议新 pass 的用户调用 `ensureOptimizedUses()`。

起初无法以同样的方式优化 MemoryDef，因为 MemorySSA 限制每个 access 只有一个操作数。这一限制已被更改，**MemoryDef 现在保存两个操作数**：

1. **定义 access（defining access）**：始终是同一基本块中前一个 MemoryDef 或 MemoryPhi，若当前块没有其他写内存的 access，则是支配前驱中的最后一个。这对于遍历 Def 链是必要的。
2. **优化后的 access（optimized access）**：如果之前调用过 Walker 的 `getClobberingMemoryAccess(MA)`，则会被设置。该 API 会将信息作为 `MA` 的一部分进行缓存。

优化所有 MemoryDef 的时间复杂度为二次方，默认不执行。

遍历某个 MemoryDef 的所有 use 可以找到被优化指向它的 access。代码片段如下：

```cpp
MemoryDef *Def;  // 找出谁被优化或定义为该 MemoryDef
for (auto &U : Def->uses()) {
  MemoryAccess *MA = cast<MemoryAccess>(U.getUser());
  if (auto *DefUser = dyn_cast<MemoryDef>(MA))
    if (DefUser->isOptimized() && DefUser->getOptimized() == Def) {
      // 被优化指向 Def 的用户
    } else {
      // 定义 access 为 Def 的用户；已优化到其他地方或未优化
    }
}
```

当 MemoryUse 被优化后，对于给定的 store，可以通过遍历该 store 的直接及传递 use 来找到所有被该 store clobber 的 load：

```cpp
checkUses(MemoryAccess *Def) { // Def 可以是 MemoryDef 或 MemoryPhi
  for (auto &U : Def->uses()) {
    MemoryAccess *MA = cast<MemoryAccess>(U.getUser());
    if (auto *MU = dyn_cast<MemoryUse>(MA)) {
      // 按需处理 MemoryUse
    } else {
      // 按需处理 MemoryDef 或 MemoryPhi

      // 由于一个用户可能作为优化 access 和定义 access 各出现一次，
      // 需要维护一个 visited 列表。

      // 按需检查传递 use
      checkUses(MA); // 使用 worklist 实现迭代算法
    }
  }
}
```

类似遍历的例子可在 `DeadStoreElimination` pass 中找到。

---

## 失效与更新

由于 MemorySSA 跟踪 LLVM IR，每当 IR 被更新时，MemorySSA 也需要随之更新。这里的"更新"包括指令的增加、删除和移动。更新 API 按需构建。如果需要示例，`GVNHoist` 和 `LICM` 都是 MemorySSA 更新 API 的使用者。

请注意，添加新的 MemoryDef（通过调用 `insertDef`）可能是一个耗时的更新操作，如果新 access 触发了大量 MemoryPhi 插入和许多 MemoryAccess 的重命名（优化失效）的话。

### Phi 的放置

MemorySSA 只在**真正需要**的地方放置 MemoryPhi，即它是一种剪枝的 SSA 形式，类似于 LLVM 的 SSA 形式。例如：

```llvm
define void @foo() {
entry:
  %p1 = alloca i8
  %p2 = alloca i8
  %p3 = alloca i8
  ; 1 = MemoryDef(liveOnEntry)
  store i8 0, ptr %p3
  br label %while.cond

while.cond:
  ; 3 = MemoryPhi({%0,1},{if.end,2})
  br i1 undef, label %if.then, label %if.else

if.then:
  br label %if.end

if.else:
  br label %if.end

if.end:
  ; MemoryUse(1)
  %1 = load i8, ptr %p1
  ; 2 = MemoryDef(3)
  store i8 2, ptr %p2
  ; MemoryUse(1)
  %2 = load i8, ptr %p3
  br label %while.cond
}
```

由于我们从 `if.then` 和 `if.else` 中删除了 store，`if.end` 中的 MemoryPhi 将毫无意义，因此不放置。所以，如果你需要在 `if.then` 或 `if.else` 中放置一个 MemoryDef，则还需要为 `if.end` 创建一个 MemoryPhi。

如果这造成了较大负担，我们也可以在所有地方都放置 MemoryPhi。由于 Walker 能够优化越过这些 phi，这样做不应妨碍优化。

---

## 非目标（Non-Goals）

MemorySSA 旨在推理内存操作之间的关系并加速查询，并不打算成为所有潜在内存相关优化的唯一事实来源。具体而言，在尝试使用 MemorySSA 推理原子（atomic）或 volatile 操作时必须谨慎，例如：

```llvm
define i8 @foo(ptr %a) {
entry:
  br i1 undef, label %if.then, label %if.end

if.then:
  ; 1 = MemoryDef(liveOnEntry)
  %0 = load volatile i8, ptr %a
  br label %if.end

if.end:
  %av = phi i8 [0, %entry], [%0, %if.then]
  ret i8 %av
}
```

仅凭 MemorySSA 的分析，将 load 提升到 `entry` 看起来可能是合法的。但由于它是 volatile load，实际上是不合法的。

---

## 设计权衡

### 精度

LLVM 中的 MemorySSA 故意以精度换取速度。

将内存变量理解为内存的不相交分区（即：如果只有一个变量，如上所述，它代表整个内存；如果有多个变量，每个变量代表内存的某个不相交部分）。

**第一**，由于别名分析结果之间存在冲突，且每种结果可能是某个分析所期望的（例如，TBAA 说 no-alias，但另一种分析说 must-alias），因此不可能按每种优化所希望的方式对内存进行分区。

**第二**，某些别名分析结果不具有传递性（即 A noalias B 且 B noalias C，并不意味着 A noalias C），因此在所有情况下都无法在不引入变量来表示每对可能别名的情况下进行精确分区。精确分区可能需要引入至少 N² 个新的虚拟变量、phi 节点等。

每个这样的变量可能在多个 def 站点被 clobber。举例来说，如果将结构体字段拆分为单独的变量，所有可能 may-def 多个结构体字段的别名操作（调用、拷贝、字段 store 等）将会 may-def 其中多个字段。这种情况相当普遍。

其他编译器中内存 SSA 形式的经验表明，精确做到这一点根本不可行；实际上，精确做到也不值得，因为这样所有的优化都不得不遍历大量虚拟变量和 phi 节点。

因此，我们选择**分区**。在进行分区时，经验同样告诉我们，分区到超过一个变量是没有意义的——这只会生成更多的 IR，而优化仍然需要查询某些东西来进一步消歧。

**因此，LLVM 分区到一个变量。**

### 实践中的精度

实践中，LLVM 的一些实现细节也会影响 MemorySSA 提供的结果精度。例如，AliasAnalysis 有各种限制，或对穿透 phi 节点的查看有限制，这些都会影响 MemorySSA 能够推断出的结论。不同 pass 所做的更改可能使 MemorySSA 变得"过度优化"（能提供比从头重新计算更准确的结果），或"优化不足"（如果重新计算可以推断出更多内容）。这可能导致在用单个 pass 隔离复现结果时遇到挑战，因为该结果依赖于 MemorySSA 经由多个后续 pass 更新后所积累的状态。

使用并更新 MemorySSA 的 pass 应通过 `MemorySSAUpdater` 提供的 API，或通过 Walker 的调用来进行。**不允许直接优化 MemorySSA。**

目前有一个单一、范围狭窄的例外：DSE（DeadStoreElimination）在某次遍历（该遍历保证优化正确）之后会更新 store 的优化 access。之所以允许这一例外，是因为相关遍历和推断超出了 MemorySSA 的能力范围，且是"免费"的（即 DSE 无论如何都会执行这些操作）。该例外通过标志 `"-dse-optimize-memoryssa"` 控制，可以禁用以帮助隔离复现优化效果。

---

## LLVM 开发者大会演讲

- [2016 LLVM Developers' Meeting: G. Burgess — MemorySSA in Five Minutes（五分钟了解 MemorySSA）](https://www.youtube.com/watch?v=bdxWmryoHak)
- [2020 LLVM Developers' Meeting: S. Baziotis & S. Moll — Finding Your Way Around the LLVM Dependence Analysis Zoo（在 LLVM 依赖分析"动物园"中找到方向）](https://www.youtube.com/watch?v=1e5y6WDbev4)
