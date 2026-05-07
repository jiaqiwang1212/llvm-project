# 基于所有权的缓冲区释放

[TOC]

单次缓冲化（One-Shot Bufferize）不会释放其分配的任何缓冲区。运行单次缓冲化后，生成的 IR(中间表示) 可能包含若干 `memref.alloc` 操作(op)，但没有 `memref.dealloc` 操作。缓冲区释放被委托给 `-ownership-based-buffer-deallocation` 传递(pass)。

从高层次来看，缓冲区由基本块"拥有"。所有权以 `i1` 类型的 SSA(静态单赋值) 值的形式体现，可以理解为"负责释放"。这在概念上类似于 C++ 中的 `std::unique_ptr`。

有一些额外的预处理和后处理传递应与基于所有权的缓冲区释放传递一起运行。推荐的编译流水线如下：

```
one-shot-bufferize
       |          it's recommended to perform all bufferization here at latest,
       |       <- any allocations inserted after this point have to be handled
       V          manually
expand-realloc
       V
ownership-based-buffer-deallocation
       V
  canonicalize <- mostly for scf.if simplifications
       V
buffer-deallocation-simplification
       V       <- from this point onwards no tensor values are allowed
lower-deallocations
       V
      CSE
       V
  canonicalize
```

整个释放流水线（不包括 `-one-shot-bufferize`）通过 `-buffer-deallocation-pipeline` 暴露。

基于所有权的缓冲区释放传递逐一处理实现了 `FunctionOpInterface` 的操作，而不分析调用图。这意味着在函数之间传递 MemRef 时必须遵守[一些规则](#函数边界-abi)。该传递的其余部分主要围绕 `bufferization.dealloc` 操作展开，该操作被无条件地插入到每个基本块的末尾（紧接终止符之前），并带有适当的操作数，应使用缓冲区释放简化传递（`--buffer-deallocation-simplification`）和常规规范化器（`--canonicalize`）进行优化。不建议直接使用 `--convert-bufferization-to-memref` 来降级 `-ownership-based-buffer-deallocation` 传递的结果而不进行事先优化，因为这会导致非常低效的代码（`bufferization.dealloc` 的运行时开销为 `O(|memrefs|^2+|memref|*|retained|)`）。

## 函数边界 ABI

缓冲区释放传递在实现了 `FunctionOpInterface` 的操作层面上运行。这类操作可以接收 MemRef 作为参数，也可以返回 MemRef。为了确保所有函数（包括外部函数）之间的兼容性，必须强制执行以下规则：
*   当 MemRef 作为函数参数传入时，所有权永远不会被获取。释放此类 MemRef 始终是调用方的责任。
*   从函数中返回 MemRef 总是将所有权转移给调用方，即调用方也有责任释放被调用函数返回的 MemRef。
*   函数不得返回与其某个参数具有相同已分配基础缓冲区的 MemRef（在这种情况下必须创建副本）。注意，在此上下文中，同一缓冲区的两个不重叠子视图也被认为存在别名。

对于外部函数（例如，用 C 语言在外部编写的库函数），外部提供的实现必须遵守这些规则，缓冲区释放传递只是假定它们遵守。对于应用了释放传递且实现可访问的函数，传递会修改这些函数使其遵守 ABI（即在必要时插入缓冲区副本）。

## 插入 `bufferization.dealloc` 操作

`bufferization.dealloc` 和所有权指示符是基于所有权的缓冲区释放传递中的主要抽象。`bufferization.dealloc` 在相应所有权指示符被设置且保留列表中没有别名缓冲区时，释放所有给定的缓冲区。

![branch_example_pre_move](/includes/img/bufferization_dealloc_op.svg)

`bufferization.dealloc` 操作被无条件地插入到每个基本块的末尾（紧接终止符之前）。该传递的大部分工作是为此操作找到正确的操作数。需要填充三个可变操作数列表：第一个包含所有可能需要释放的 MemRef 值，第二个包含其关联的所有权值（`i1` 类型），第三个包含在后续仍需使用因此不应被释放的 MemRef 值（例如，yielded 或返回的缓冲区）。

`bufferization.dealloc` 允许我们处理任何类型的别名行为：当无法静态收集到足够的信息时，它会降级为运行时别名检查。当足够的别名信息静态可用时，操作数或整个操作可能会被折叠消除。

**所有权**

为此，我们使用 memref 所有权指示符的概念，它对 `memref` 类型的任何 SSA 值物化为一个 `i1` 值，指示该值物化所在的基本块是否拥有该 MemRef 的所有权。理想情况下，这是一个常量 `true` 或 `false`，但也可能是非常量的 SSA 值。为了在不立即物化的情况下跟踪这些所有权值（立即物化可能需要在实际不需要物化值的位置插入 `bufferization.clone` 操作或在运行时检查别名的操作），我们使用 `Ownership` 类。该类在偏序上的格中以三种状态表示所有权：
```
forall X in SSA values. uninitialized < unique(X) < unknown
forall X, Y in SSA values.
  unique(X) == unique(Y) iff X and Y always evaluate to the same value
  unique(X) != unique(Y) otherwise
```
直观地说，这些状态具有以下含义：
*   未初始化（Uninitialized）：所有权尚未初始化，这是默认状态；一旦操作完成处理，所有具有 MemRef 类型的操作结果的所有权就不应再是未初始化状态。
*   唯一（Unique）：存在一个特定的 SSA 值，可以查询该值来检查所有权，而无需物化任何额外的 IR。
*   未知（Unknown）：没有特定的 SSA 值可用，而不物化额外的 IR，通常这是因为两个处于"唯一"状态的所有权必须手动合并（例如，`arith.select` 的结果根据条件值具有 then 或 else 情况的所有权，为所有权值插入另一个 `arith.select` 可以执行合并并为结果提供"唯一"所有权），然而，在一般情况下必须分配此"未知"状态。

由上述偏序所蕴含，该传递以如下方式合并两个所有权：

| 所有权 1      | 所有权 2      | 合并后所有权       |
|:--------------|:--------------|:-------------------|
| uninitialized | uninitialized | uninitialized      |
| unique(X)     | uninitialized | unique(X)          |
| unique(X)     | unique(X)     | unique(X)          |
| unique(X)     | unique(Y)     | unknown            |
| unknown       | unique        | unknown            |
| unknown       | uninitialized | unknown            |
| <td colspan=3> + symmetric cases                   |

**收集可能需要释放的 MemRef 列表**

对于给定的基本块，可能需要在该块末尾释放的 MemRef 列表通过跟踪该块可能接管所有权的所有值来计算。这包括作为基本块参数提供的 MemRef、`memref.alloc` 和 `func.call` 等操作的接口处理程序，以及具有多个基本块的区域中的活跃度信息。更具体地说，它通过以下方式计算：取当前基本块 B 的活跃度分析的"in"集中的 MemRef，追加 MemRef 基本块参数以及 B 本身分配的 MemRef 集合（由接口处理程序确定），然后减去（同样由接口处理程序确定）B 中释放的 MemRef 集合。

注意，我们不必取活跃度"in"集与前驱块的"out"集的交集，因为处于"in"集中的值必须定义在支配所有直接前驱的祖先块中，因此该块的"in"集是每个前驱的"out"集的子集。

```
memrefs = filter((liveIn(block) U
  allocated(block) U arguments(block)) \ deallocated(block), isMemRef)
```

`bufferization.dealloc` 第二个可变操作数列表的条件列表通过查询上述收集的每个 MemRef 的已存储所有权值来计算。所有权状态由接口处理程序在处理基本块时更新。

**收集要保留的 MemRef 列表**

给定基本块 B，对于不同的后继块 S，必须保留的 MemRef 列表可能不同。对于两个基本块 B 和 S，以及通过块参数传递给目标块 S 的值，我们通过取终止符的后继操作数列表中的 MemRef，以及 B 的活跃度分析的"out"集与目标块 S 的"in"集的交集中的 MemRef，来计算 B 中必须保留的 MemRef 列表。

这个保留值列表确保了即使在编译时没有可用的别名信息，也不会遇到释放后使用的情况。

```
toRetain = filter(successorOperands + (liveOut(fromBlock) insersect
  liveIn(toBlock)), isMemRef)
```

## 支持的接口

该传递使用活跃度分析和一些接口：
*   `FunctionOpInterface`
*   `CallOpInterface`
*   `MemoryEffectOpInterface`
*   `RegionBranchOpInterface`
*   `RegionBranchTerminatorOpInterface`

由于接口提供的信息不足，它目前还对 `cf.cond_br` 操作进行了特殊处理，并对实现了 `RegionBranchOpInterface` 的操作做了一些假设，但改进接口将允许我们在未来消除这些依赖。

## 限制

缓冲区释放传递对输入 IR 有一些要求和限制。这些在传递开始时进行检查，并相应地发出错误：
*   传递所操作的接口集必须被正确实现。例如，如果存在一个带有嵌套区域的操作但没有实现 `RegionBranchOpInterface`，则会发出错误，因为传递无法得知嵌套区域的语义（也不对其做任何默认假设）。
*   不存在显式的控制流循环。目前，只支持使用结构化控制流的循环。然而，这个限制将来可能会被解除。
*   不应该已经存在释放操作。传递应该已经能正确处理它们（至少在大多数情况下），但由于测试不足，目前尚不支持。
*   终止符必须实现 `RegionBranchTerminatorOpInterface` 或 `BranchOpInterface` 之一，但不能同时实现两者。不支持具有多个后继的终止符（`cf.cond_br` 除外）。这不是根本性限制，只是目前没有用例来证明更复杂实现的必要性。

## 示例

以下示例包含一些有趣的情况：
*   基本块参数被修改为同时传递所有权指示符，但入口块除外，入口块应用函数边界 ABI。
*   `arith.select` 的结果最初被分配了"未知"所有权，但一旦插入 `bufferization.dealloc` 操作，它就被放入"保留"列表（因为它在后续基本块中有使用），因此"未知"所有权可以使用 dealloc 操作的相应结果替换为"唯一"所有权。
*   `cf.cond_br` 操作有多个后继，因此必须插入两个 `bufferization.dealloc` 操作（每个后继一个）。虽然它们具有相同的要释放的 MemRef 列表（因为它们为同一个块执行释放），但必须考虑到某些 MemRef 对于一个分支保持*存活*而对另一个分支不是（因此对当前块的*live-out*和目标块的*live-in*执行集合交集）。此外，`cf.cond_br` 支持为每个后继分别转发操作数。为了确保没有 MemRef 被释放两次（因为有两个具有相同要释放的 MemRef 的 `bufferization.dealloc` 操作），条件操作数被调整以考虑分支条件。虽然可以为此类终止符操作实现通用降级，但专门化的实现可以利用此特定操作的所有语义，从而生成更高效的降级。

```mlir
func.func @example(%memref: memref<?xi8>, %select_cond: i1, %br_cond: i1) {
  %alloc = memref.alloc() : memref<?xi8>
  %alloca = memref.alloca() : memref<?xi8>
  %select = arith.select %select_cond, %alloc, %alloca : memref<?xi8>
  cf.cond_br %br_cond, ^bb1(%alloc : memref<?xi8>), ^bb1(%memref : memref<?xi8>)
^bb1(%bbarg: memref<?xi8>):
  test.copy(%bbarg, %select) : (memref<?xi8>, memref<?xi8>)
  return
}
```

运行 `--ownership-based-buffer-deallocation` 后，结果如下：

```mlir
// Function boundary ABI: ownership of `%memref` will never be acquired.
func.func @example(%memref: memref<?xi8>, %select_cond: i1, %br_cond: i1) {
  %false = arith.constant false
  %true = arith.constant true

  // The ownership of a MemRef defined by the `memref.alloc` operation is always
  // assigned to be 'true'.
  %alloc = memref.alloc() : memref<?xi8>

  // The ownership of a MemRef defined by the `memref.alloca` operation is
  // always assigned to be 'false'.
  %alloca = memref.alloca() : memref<?xi8>

  // The ownership of %select will be the join of the ownership of %alloc and
  // the ownership of %alloca, i.e., of %true and %false. Because the pass does
  // not know about the semantics of the `arith.select` operation (unless a
  // custom handler is implemented), the ownership join will be 'Unknown'. If
  // the materialized ownership indicator of %select is needed, either a clone
  // has to be created for which %true is assigned as ownership or the result
  // of a `bufferization.dealloc` where %select is in the retain list has to be
  // used.
  %select = arith.select %select_cond, %alloc, %alloca : memref<?xi8>

  // We use `memref.extract_strided_metadata` to get the base memref since it is
  // not allowed to pass arbitrary memrefs to `memref.dealloc`. This property is
  // already enforced for `bufferization.dealloc`
  %base_buffer_memref, ... = memref.extract_strided_metadata %memref
    : memref<?xi8> -> memref<i8>, index, index, index
  %base_buffer_alloc, ... = memref.extract_strided_metadata %alloc
    : memref<?xi8> -> memref<i8>, index, index, index
  %base_buffer_alloca, ... = memref.extract_strided_metadata %alloca
    : memref<?xi8> -> memref<i8>, index, index, index

  // The deallocation conditions need to be adjusted to incorporate the branch
  // condition. In this example, this requires only a single negation, but might
  // also require multiple arith.andi operations.
  %not_br_cond = arith.xori %true, %br_cond : i1

  // There are two dealloc operations inserted in this basic block, one per
  // successor. Both have the same list of MemRefs to deallocate and the
  // conditions only differ by the branch condition conjunct.
  // Note, however, that the retained list differs. Here, both contain the
  // %select value because it is used in both successors (since it's the same
  // block), but the value passed via block argument differs (%memref vs.
  // %alloc).
  %10:2 = bufferization.dealloc
           (%base_buffer_memref, %base_buffer_alloc, %base_buffer_alloca
             : memref<i8>, memref<i8>, memref<i8>)
        if (%false, %br_cond, %false)
    retain (%alloc, %select : memref<?xi8>, memref<?xi8>)

  %11:2 = bufferization.dealloc
           (%base_buffer_memref, %base_buffer_alloc, %base_buffer_alloca
             : memref<i8>, memref<i8>, memref<i8>)
        if (%false, %not_br_cond, %false)
    retain (%memref, %select : memref<?xi8>, memref<?xi8>)

  // Because %select is used in ^bb1 without passing it via block argument, we
  // need to update it's ownership value here by merging the ownership values
  // returned by the dealloc operations
  %new_ownership = arith.select %br_cond, %10#1, %11#1 : i1

  // The terminator is modified to pass along the ownership indicator values
  // with each MemRef value.
  cf.cond_br %br_cond, ^bb1(%alloc, %10#0 : memref<?xi8>, i1),
                       ^bb1(%memref, %11#0 : memref<?xi8>, i1)

// All non-entry basic blocks are modified to have an additional i1 argument for
// each MemRef value in the argument list.
^bb1(%13: memref<?xi8>, %14: i1):  // 2 preds: ^bb0, ^bb0
  test.copy(%13, %select) : (memref<?xi8>, memref<?xi8>)

  %base_buffer_13, ... = memref.extract_strided_metadata %13
    : memref<?xi8> -> memref<i8>, index, index, index
  %base_buffer_select, ... = memref.extract_strided_metadata %select
    : memref<?xi8> -> memref<i8>, index, index, index

  // Here, we don't have a retained list, because the block has no successors
  // and the return has no operands.
  bufferization.dealloc (%base_buffer_13, %base_buffer_select
                          : memref<i8>, memref<i8>)
                     if (%14, %new_ownership)
  return
}
```

## 缓冲区释放简化传递

[`bufferization.dealloc` 操作的语义](#bufferizationdealloc-bufferizationdeallocop)
提供了大量优化机会，这些优化可以方便地拆分为使用贪心模式重写器的模式。其中一些模式需要访问额外的分析，例如可以确定两个 MemRef 值是否必然、可能或绝不来自同一缓冲区分配的分析。这些模式被收集在缓冲区释放简化传递中，而不需要额外分析的模式则作为常规规范化器传递的一部分注册。此传递最好在 `--ownership-based-buffer-deallocation` 之后紧跟 `--canonicalize` 运行。

该传递应用以下简化模式：
*   当保证与"memref"操作数列表中的任何值不存在别名时，从保留列表中移除 MemRef。这避免了与已移除值的额外别名检查。
*   当"memref"列表中的某个值保证与列表中的任何其他值不存在别名时，将其拆分到仅包含该值在"memref"列表中的新 `bufferization.dealloc` 操作中。这至少避免了运行时的一次别名检查，并为这个新的 `bufferization.dealloc` 操作启用了更高效的降级。
*   当"memref"操作数列表中的某个值保证与"保留"列表中至少一个值存在别名，且可能与"保留"列表中没有其他值存在别名时，将其从"memref"操作数列表中移除。

## 降低释放传递

`-lower-deallocations` 传递将所有 `bufferization.dealloc` 操作转换为 `memref.dealloc` 操作，并可能插入来自 `scf`、`func` 和 `arith` 方言的操作，以使释放有条件，并在运行时检查两个 MemRef 值是否来自同一分配（当 `buffer-deallocation-simplification` 传递无法静态确定时）。

`bufferization.dealloc` 操作的相同降级也是 `-convert-bufferization-to-memref` 转换传递的一部分，该传递还会降级 bufferization 方言的所有其他操作。

我们在此降级传递中区分多种情况，以提供整体上更高效的降级。在一般情况下，会创建一个库函数以避免代码大小的二次方爆炸（相对于 dealloc 操作的操作数数量）。专门化降级旨在避免使用这个库函数，因为它需要分配索引值的辅助 MemRef。

### 通用降级

生成一个库函数以避免代码大小爆炸。从高层次来看，提取所有操作数的 base-memref 作为索引值，存储到专门分配的 MemRef 中，并传递给库函数，由库函数确定它们是否来自同一个原始分配。这些信息是为了避免双重释放情况以及正确保留 `retained` 列表中的 MemRef 值所必需的。

**Dealloc 操作降级**

此降级支持 dealloc 操作提供的所有功能。它计算每个 memref 的基础指针（作为索引），将其存储在新的 memref 辅助结构中，并将其传递给 `buildDeallocationLibraryFunction` 中生成的辅助函数。结果存储在作为参数传入的两个布尔列表（表示为 MemRef）中。第一个列表存储相应条件是否应该被释放，第二个列表存储保留值的所有权，可用于替换 `bufferization.dealloc` 操作的结果值。

示例：
```mlir
%0:2 = bufferization.dealloc (%m0, %m1 : memref<2xf32>, memref<5xf32>)
                          if (%cond0, %cond1)
                      retain (%r0, %r1 : memref<1xf32>, memref<2xf32>)
```
降级为（简化版）：
```mlir
%c0 = arith.constant 0 : index
%c1 = arith.constant 1 : index
%dealloc_base_pointer_list = memref.alloc() : memref<2xindex>
%cond_list = memref.alloc() : memref<2xi1>
%retain_base_pointer_list = memref.alloc() : memref<2xindex>
%m0_base_pointer = memref.extract_aligned_pointer_as_index %m0
memref.store %m0_base_pointer, %dealloc_base_pointer_list[%c0]
%m1_base_pointer = memref.extract_aligned_pointer_as_index %m1
memref.store %m1_base_pointer, %dealloc_base_pointer_list[%c1]
memref.store %cond0, %cond_list[%c0]
memref.store %cond1, %cond_list[%c1]
%r0_base_pointer = memref.extract_aligned_pointer_as_index %r0
memref.store %r0_base_pointer, %retain_base_pointer_list[%c0]
%r1_base_pointer = memref.extract_aligned_pointer_as_index %r1
memref.store %r1_base_pointer, %retain_base_pointer_list[%c1]
%dyn_dealloc_base_pointer_list = memref.cast %dealloc_base_pointer_list :
   memref<2xindex> to memref<?xindex>
%dyn_cond_list = memref.cast %cond_list : memref<2xi1> to memref<?xi1>
%dyn_retain_base_pointer_list = memref.cast %retain_base_pointer_list :
   memref<2xindex> to memref<?xindex>
%dealloc_cond_out = memref.alloc() : memref<2xi1>
%ownership_out = memref.alloc() : memref<2xi1>
%dyn_dealloc_cond_out = memref.cast %dealloc_cond_out :
   memref<2xi1> to memref<?xi1>
%dyn_ownership_out = memref.cast %ownership_out :
   memref<2xi1> to memref<?xi1>
call @dealloc_helper(%dyn_dealloc_base_pointer_list,
                     %dyn_retain_base_pointer_list,
                     %dyn_cond_list,
                     %dyn_dealloc_cond_out,
                     %dyn_ownership_out) : (...)
%m0_dealloc_cond = memref.load %dyn_dealloc_cond_out[%c0] : memref<2xi1>
scf.if %m0_dealloc_cond {
  memref.dealloc %m0 : memref<2xf32>
}
%m1_dealloc_cond = memref.load %dyn_dealloc_cond_out[%c1] : memref<2xi1>
scf.if %m1_dealloc_cond {
  memref.dealloc %m1 : memref<5xf32>
}
%r0_ownership = memref.load %dyn_ownership_out[%c0] : memref<2xi1>
%r1_ownership = memref.load %dyn_ownership_out[%c1] : memref<2xi1>
memref.dealloc %dealloc_base_pointer_list : memref<2xindex>
memref.dealloc %retain_base_pointer_list : memref<2xindex>
memref.dealloc %cond_list : memref<2xi1>
memref.dealloc %dealloc_cond_out : memref<2xi1>
memref.dealloc %ownership_out : memref<2xi1>
// replace %0#0 with %r0_ownership
// replace %0#1 with %r1_ownership
```

**库函数**

每个编译单元生成一个库函数，可在 bufferization dealloc 位置调用，以确定两个 MemRef 是否来自同一分配以及其新的所有权。

生成的函数接受两个索引 MemRef 和三个布尔 MemRef 作为参数：
  * 第一个参数 A 应包含对要释放的 MemRef 应用 extract_aligned_pointer_as_index 操作的结果
  * 第二个参数 B 应包含对要保留的 MemRef 应用 extract_aligned_pointer_as_index 操作的结果
  * 第三个参数 C 应包含直接传递给释放操作的条件
  * 第四个参数 D 用于将结果传递给调用方。这些结果表示 A 中相应位置的 MemRef 应被释放的条件。
  * 第五个参数 E 用于将结果传递给调用方。它提供 B 中相同位置的 MemRef 对应的所有权值。

这个辅助函数应该为每个 `bufferization.dealloc` 操作调用一次，以确定保留值的释放需求和新的所有权指示符，但不执行释放本身。

生成的代码：
```mlir
func.func @dealloc_helper(
    %dyn_dealloc_base_pointer_list: memref<?xindex>,
    %dyn_retain_base_pointer_list: memref<?xindex>,
    %dyn_cond_list: memref<?xi1>,
    %dyn_dealloc_cond_out: memref<?xi1>,
    %dyn_ownership_out: memref<?xi1>) {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %true = arith.constant true
  %false = arith.constant false
  %num_dealloc_memrefs = memref.dim %dyn_dealloc_base_pointer_list, %c0
  %num_retain_memrefs = memref.dim %dyn_retain_base_pointer_list, %c0
  // Zero initialize result buffer.
  scf.for %i = %c0 to %num_retain_memrefs step %c1 {
    memref.store %false, %dyn_ownership_out[%i] : memref<?xi1>
  }
  scf.for %i = %c0 to %num_dealloc_memrefs step %c1 {
    %dealloc_bp = memref.load %dyn_dealloc_base_pointer_list[%i]
    %cond = memref.load %dyn_cond_list[%i]
    // Check for aliasing with retained memrefs.
    %does_not_alias_retained = scf.for %j = %c0 to %num_retain_memrefs
        step %c1 iter_args(%does_not_alias_aggregated = %true) -> (i1) {
      %retain_bp = memref.load %dyn_retain_base_pointer_list[%j]
      %does_alias = arith.cmpi eq, %retain_bp, %dealloc_bp : index
      scf.if %does_alias {
        %curr_ownership = memref.load %dyn_ownership_out[%j]
        %updated_ownership = arith.ori %curr_ownership, %cond : i1
        memref.store %updated_ownership, %dyn_ownership_out[%j]
      }
      %does_not_alias = arith.cmpi ne, %retain_bp, %dealloc_bp : index
      %updated_aggregate = arith.andi %does_not_alias_aggregated,
                                      %does_not_alias : i1
      scf.yield %updated_aggregate : i1
    }
    // Check for aliasing with dealloc memrefs in the list before the
    // current one, i.e.,
    // `fix i, forall j < i: check_aliasing(%dyn_dealloc_base_pointer[j],
    // %dyn_dealloc_base_pointer[i])`
    %does_not_alias_any = scf.for %j = %c0 to %i step %c1
       iter_args(%does_not_alias_agg = %does_not_alias_retained) -> (i1) {
      %prev_dealloc_bp = memref.load %dyn_dealloc_base_pointer_list[%j]
      %does_not_alias = arith.cmpi ne, %prev_dealloc_bp, %dealloc_bp
      %updated_alias_agg = arith.andi %does_not_alias_agg, %does_not_alias
      scf.yield %updated_alias_agg : i1
    }
    %dealloc_cond = arith.andi %does_not_alias_any, %cond : i1
    memref.store %dealloc_cond, %dyn_dealloc_cond_out[%i] : memref<?xi1>
  }
  return
}
```

### 专门化降级

目前，有两种针对常见情况的特殊降级，以避免使用库函数，从而避免不必要的内存加载和存储操作以及函数调用：

**单个 memref，无保留值**

降级一种简单情况：没有任何保留值且只有单个 MemRef。理想情况下，静态分析可以提供足够的信息，使 `buffer-deallocation-simplification` 传递能够在运行此传递之前尽可能多地将 dealloc 操作拆分为这种简单情况。

示例：
```mlir
bufferization.dealloc (%arg0 : memref<2xf32>) if (%arg1)
```
降级为
```mlir
scf.if %arg1 {
  memref.dealloc %arg0 : memref<2xf32>
}
```

在大多数情况下，分支条件是常量 'true' 或 'false'，因此可以通过规范化器传递完全优化掉。

**单个 memref，任意数量的保留值**

对于恰好有一个 MemRef 但任意数量保留值的释放操作的特殊情况降级。此降级产生的代码大小与保留值的数量成线性关系。

示例：
```mlir
%0:2 = bufferization.dealloc (%m : memref<2xf32>) if (%cond)
                      retain (%r0, %r1 : memref<1xf32>, memref<2xf32>)
return %0#0, %0#1 : i1, i1
```
降级为
```mlir
%m_base_pointer = memref.extract_aligned_pointer_as_index %m
%r0_base_pointer = memref.extract_aligned_pointer_as_index %r0
%r0_does_not_alias = arith.cmpi ne, %m_base_pointer, %r0_base_pointer
%r1_base_pointer = memref.extract_aligned_pointer_as_index %r1
%r1_does_not_alias = arith.cmpi ne, %m_base_pointer, %r1_base_pointer
%not_retained = arith.andi %r0_does_not_alias, %r1_does_not_alias : i1
%should_dealloc = arith.andi %not_retained, %cond : i1
scf.if %should_dealloc {
  memref.dealloc %m : memref<2xf32>
}
%true = arith.constant true
%r0_does_alias = arith.xori %r0_does_not_alias, %true : i1
%r0_ownership = arith.andi %r0_does_alias, %cond : i1
%r1_does_alias = arith.xori %r1_does_not_alias, %true : i1
%r1_ownership = arith.andi %r1_does_alias, %cond : i1
return %r0_ownership, %r1_ownership : i1, i1
```
