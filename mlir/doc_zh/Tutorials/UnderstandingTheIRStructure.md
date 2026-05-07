# 理解 IR 结构

MLIR 语言参考描述了 [高层结构](../LangRef.md/#high-level-structure)，本文档通过示例说明这一结构，并同时介绍操作它所涉及的 C++ API。

我们将实现一个[通道](../PassManagement.md/#operation-pass)，该通道遍历任意 MLIR 输入并打印 IR 中的实体。通道（或通常几乎任何 IR 片段）总是以一个操作为根。大多数情况下，顶层操作是 `ModuleOp`，MLIR `PassManager` 实际上被限制在顶层 `ModuleOp` 上操作。因此，通道从一个操作开始，我们的遍历也将如此：

```
  void runOnOperation() override {
    Operation *op = getOperation();
    resetIndent();
    printOperation(op);
  }
```

## 遍历 IR 嵌套结构

IR 是递归嵌套的：一个 `Operation` 可以有一个或多个嵌套的 `Region`，每个 `Region` 实际上是一个 `Block` 的列表，每个 `Block` 本身包含一个 `Operation` 的列表。我们的遍历将用三个方法来跟踪这一结构：`printOperation()`、`printRegion()` 和 `printBlock()`。

第一个方法检查操作的属性，然后迭代嵌套 Region 并逐个打印它们：

```c++
  void printOperation(Operation *op) {
    // 打印操作本身及其一些属性
    printIndent() << "visiting op: '" << op->getName() << "' with "
                  << op->getNumOperands() << " operands and "
                  << op->getNumResults() << " results\n";
    // 打印操作属性
    if (!op->getAttrs().empty()) {
      printIndent() << op->getAttrs().size() << " attributes:\n";
      for (NamedAttribute attr : op->getAttrs())
        printIndent() << " - '" << attr.getName() << "' : '"
                      << attr.getValue() << "'\n";
    }

    // 递归进入附加到该操作的每个 Region。
    printIndent() << " " << op->getNumRegions() << " nested regions:\n";
    auto indent = pushIndent();
    for (Region &region : op->getRegions())
      printRegion(region);
  }
```

`Region` 除了 `Block` 的列表外不持有任何其他内容：

```c++
  void printRegion(Region &region) {
    // Region 本身除了块的列表外不持有任何内容。
    printIndent() << "Region with " << region.getBlocks().size()
                  << " blocks:\n";
    auto indent = pushIndent();
    for (Block &block : region.getBlocks())
      printBlock(block);
  }
```

最后，`Block` 有一个参数列表，并持有一个 `Operation` 的列表：

```c++
  void printBlock(Block &block) {
    // 打印块的内置属性（基本上是：参数列表）
    printIndent()
        << "Block with " << block.getNumArguments() << " arguments, "
        << block.getNumSuccessors()
        << " successors, and "
        // 注意，这个 `.size()` 在遍历一个链表，时间复杂度为 O(n)。
        << block.getOperations().size() << " operations\n";

    // 块的主要作用是持有操作列表：让我们递归地打印每个操作。
    auto indent = pushIndent();
    for (Operation &op : block.getOperations())
      printOperation(&op);
  }
```

该通道的代码可以在[仓库中找到](https://github.com/llvm/llvm-project/blob/main/mlir/test/lib/IR/TestPrintNesting.cpp)，并可通过 `mlir-opt -test-print-nesting` 来使用。

### 示例

上一节介绍的通道可以应用于以下 IR，使用 `mlir-opt -test-print-nesting -allow-unregistered-dialect llvm-project/mlir/test/IR/print-ir-nesting.mlir`：

```mlir
"builtin.module"() ( {
  %results:4 = "dialect.op1"() {"attribute name" = 42 : i32} : () -> (i1, i16, i32, i64)
  "dialect.op2"() ( {
    "dialect.innerop1"(%results#0, %results#1) : (i1, i16) -> ()
  },  {
    "dialect.innerop2"() : () -> ()
    "dialect.innerop3"(%results#0, %results#2, %results#3)[^bb1, ^bb2] : (i1, i32, i64) -> ()
  ^bb1(%1: i32):  // pred: ^bb0
    "dialect.innerop4"() : () -> ()
    "dialect.innerop5"() : () -> ()
  ^bb2(%2: i64):  // pred: ^bb0
    "dialect.innerop6"() : () -> ()
    "dialect.innerop7"() : () -> ()
  }) {"other attribute" = 42 : i64} : () -> ()
}) : () -> ()
```

将产生以下输出：

```
visiting op: 'builtin.module' with 0 operands and 0 results
 1 nested regions:
  Region with 1 blocks:
    Block with 0 arguments, 0 successors, and 2 operations
      visiting op: 'dialect.op1' with 0 operands and 4 results
      1 attributes:
       - 'attribute name' : '42 : i32'
       0 nested regions:
      visiting op: 'dialect.op2' with 0 operands and 0 results
      1 attributes:
       - 'other attribute' : '42 : i64'
       2 nested regions:
        Region with 1 blocks:
          Block with 0 arguments, 0 successors, and 1 operations
            visiting op: 'dialect.innerop1' with 2 operands and 0 results
             0 nested regions:
        Region with 3 blocks:
          Block with 0 arguments, 2 successors, and 2 operations
            visiting op: 'dialect.innerop2' with 0 operands and 0 results
             0 nested regions:
            visiting op: 'dialect.innerop3' with 3 operands and 0 results
             0 nested regions:
          Block with 1 arguments, 0 successors, and 2 operations
            visiting op: 'dialect.innerop4' with 0 operands and 0 results
             0 nested regions:
            visiting op: 'dialect.innerop5' with 0 operands and 0 results
             0 nested regions:
          Block with 1 arguments, 0 successors, and 2 operations
            visiting op: 'dialect.innerop6' with 0 operands and 0 results
             0 nested regions:
            visiting op: 'dialect.innerop7' with 0 operands and 0 results
             0 nested regions:
```

## 其他 IR 遍历方法

在许多情况下，展开 IR 的递归结构比较繁琐，你可能会对使用其他辅助工具感兴趣。

### 过滤迭代器：`getOps<OpTy>()`

例如，`Block` 类暴露了一个方便的模板方法 `getOps<OpTy>()`，提供了一个过滤迭代器。下面是一个示例：

```c++
  auto varOps = entryBlock.getOps<spirv::GlobalVariableOp>();
  for (spirv::GlobalVariableOp gvOp : varOps) {
     // 处理块中的每个 GlobalVariable 操作。
     ...
  }
```

类似地，`Region` 类也暴露了相同的 `getOps` 方法，该方法将迭代 Region 中的所有块。

### 遍历器（Walkers）

`getOps<OpTy>()` 适用于迭代直接列在单个块（或单个 Region）内的某些操作，但通常需要以嵌套的方式遍历 IR。为此，MLIR 在 `Operation`、`Block` 和 `Region` 上暴露了 `walk()` 辅助函数。该辅助函数接受一个参数：一个回调方法，该方法将对递归嵌套在提供的实体下的每个操作（以及这个初始操作本身）按后序调用。

```c++
  // 递归遍历嵌套在函数内的所有 Region 和块，
  // 并以后序对每个操作应用回调。
  getFunction().walk([&](mlir::Operation *op) {
    // 处理操作 `op`。
  });
```

提供的回调可以专门化以过滤特定类型的操作，例如以下代码只将回调应用于嵌套在函数内的 `LinalgOp` 操作：

```c++
  getFunction().walk([](LinalgOp linalgOp) {
    // 处理 LinalgOp `linalgOp`。
  });
```

最后，回调可以通过返回 `WalkResult::interrupt()` 值来选择性地停止遍历。例如，以下遍历将查找嵌套在函数内的所有 `AllocOp`，并在其中一个不满足条件时中断遍历：

```c++
  WalkResult result = getFunction().walk([&](AllocOp allocOp) {
    if (!isValid(allocOp))
      return WalkResult::interrupt();
    return WalkResult::advance();
  });
  if (result.wasInterrupted())
    // 其中一个 alloc 不匹配。
    ...
```

## 遍历定义-使用链

IR 中的另一个关系是将 `Value` 与其用户联系起来的关系。如[语言参考](../LangRef.md/#high-level-structure)中所定义，每个值要么是 `BlockArgument`，要么恰好是一个 `Operation` 的结果（一个 `Operation` 可以有多个结果，每个结果都是一个单独的 `Value`）。`Value` 的用户是 `Operation`，通过它们的参数：每个 `Operation` 参数引用一个单独的 `Value`。

以下是一个检查 `Operation` 操作数并打印关于它们的一些信息的代码示例：

```c++
  // 打印关于每个操作数的生产者信息。
  for (Value operand : op->getOperands()) {
    if (Operation *producer = operand.getDefiningOp()) {
      llvm::outs() << "  - Operand produced by operation '"
                   << producer->getName() << "'\n";
    } else {
      // 如果没有定义操作，该值必然是一个块参数。
      auto blockArg = cast<BlockArgument>(operand);
      llvm::outs() << "  - Operand produced by Block argument, number "
                   << blockArg.getArgNumber() << "\n";
    }
  }
```

类似地，以下代码示例迭代 `Operation` 产生的结果 `Value`，并对每个结果迭代这些结果的用户并打印关于它们的信息：

```c++
  // 打印关于每个结果的用户信息。
  llvm::outs() << "Has " << op->getNumResults() << " results:\n";
  for (auto indexedResult : llvm::enumerate(op->getResults())) {
    Value result = indexedResult.value();
    llvm::outs() << "  - Result " << indexedResult.index();
    if (result.use_empty()) {
      llvm::outs() << " has no uses\n";
      continue;
    }
    if (result.hasOneUse())
      llvm::outs() << " has a single use: ";
    else
      llvm::outs() << " has " << result.getNumUses() << " uses:\n";
    for (Operation *userOp : result.getUsers()) {
      llvm::outs() << "    - " << userOp->getName() << "\n";
    }
  }
```

该通道的示例代码可以在[仓库中找到](https://github.com/llvm/llvm-project/blob/main/mlir/test/lib/IR/TestPrintDefUse.cpp)，并可通过 `mlir-opt -test-print-defuse` 来使用。

`Value` 与其使用之间的链可以如下图所示：

![Index Map Example](/includes/img/DefUseChains.svg)

`Value` 的使用（`OpOperand` 或 `BlockOperand`）也通过双向链表连接，这在将一个 `Value` 的所有使用替换为新值（"RAUW"）时特别有用：

![Index Map Example](/includes/img/Use-list.svg)
