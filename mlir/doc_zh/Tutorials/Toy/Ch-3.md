# 第 3 章：高级语言特定分析与变换

[TOC]

创建一个能够紧密表达输入语言语义的方言（dialect），可以在 MLIR 中进行需要高级语言信息的分析、变换（transformation）和优化，这些操作通常在语言的 AST 上执行。例如，`clang` 有一套相当[复杂的机制](https://clang.llvm.org/doxygen/classclang_1_1TreeTransform.html)用于在 C++ 中执行模板实例化。

我们将编译器变换分为两类：局部（local）和全局（global）。在本章中，我们重点介绍如何利用 Toy 方言及其高级语义来执行在 LLVM 中难以实现的局部模式匹配变换（local pattern-match transformations）。为此，我们使用 MLIR 的[通用 DAG 重写器](../../PatternRewriter.md)。

实现模式匹配变换有两种方法：1. 命令式（Imperative）的 C++ 模式匹配与重写；2. 声明式（Declarative）的、基于规则的模式匹配与重写，使用表驱动的[声明式重写规则](../../DeclarativeRewrites.md)（DRR）。注意，使用 DRR 要求操作（operations）使用 ODS 定义，如[第 2 章](Ch-2.md)所述。

## 使用 C++ 风格的模式匹配与重写优化转置操作

让我们从一个简单的模式开始，尝试消除相互抵消的两次连续转置：`transpose(transpose(X)) -> X`。以下是对应的 Toy 示例：

```toy
def transpose_transpose(x) {
  return transpose(transpose(x));
}
```

对应的 IR（中间表示）如下：

```mlir
toy.func @transpose_transpose(%arg0: tensor<*xf64>) -> tensor<*xf64> {
  %0 = toy.transpose(%arg0 : tensor<*xf64>) to tensor<*xf64>
  %1 = toy.transpose(%0 : tensor<*xf64>) to tensor<*xf64>
  toy.return %1 : tensor<*xf64>
}
```

这是一个在 Toy IR 上很容易匹配的变换示例，但对于 LLVM 来说却相当难以识别。例如，目前 Clang 无法优化掉临时数组，而使用朴素转置的计算需要用以下循环来表达：

```c++
#define N 100
#define M 100

void sink(void *);
void double_transpose(int A[N][M]) {
  int B[M][N];
  for(int i = 0; i < N; ++i) {
    for(int j = 0; j < M; ++j) {
       B[j][i] = A[i][j];
    }
  }
  for(int i = 0; i < N; ++i) {
    for(int j = 0; j < M; ++j) {
       A[i][j] = B[j][i];
    }
  }
  sink(A);
}
```

对于一种简单的 C++ 重写方法——在 IR 中匹配树状模式并将其替换为一组不同的操作——我们可以通过实现 `RewritePattern` 来接入 MLIR 的 `Canonicalizer`（规范化）通道（pass）：

```c++
/// Fold transpose(transpose(x)) -> x
struct SimplifyRedundantTranspose : public mlir::OpRewritePattern<TransposeOp> {
  /// We register this pattern to match every toy.transpose in the IR.
  /// The "benefit" is used by the framework to order the patterns and process
  /// them in order of profitability.
  SimplifyRedundantTranspose(mlir::MLIRContext *context)
      : OpRewritePattern<TransposeOp>(context, /*benefit=*/1) {}

  /// This method is attempting to match a pattern and rewrite it. The rewriter
  /// argument is the orchestrator of the sequence of rewrites. It is expected
  /// to interact with it to perform any changes to the IR from here.
  llvm::LogicalResult
  matchAndRewrite(TransposeOp op,
                  mlir::PatternRewriter &rewriter) const override {
    // Look through the input of the current transpose.
    mlir::Value transposeInput = op.getOperand();
    TransposeOp transposeInputOp = transposeInput.getDefiningOp<TransposeOp>();

    // Input defined by another transpose? If not, no match.
    if (!transposeInputOp)
      return failure();

    // Otherwise, we have a redundant transpose. Use the rewriter.
    rewriter.replaceOp(op, {transposeInputOp.getOperand()});
    return success();
  }
};
```

该重写器的实现位于 `ToyCombine.cpp` 中。[规范化通道](../../Canonicalization.md)以贪婪、迭代的方式应用操作定义的变换。为确保规范化通道应用我们的新变换，我们设置 [hasCanonicalizer = 1](../../DefiningDialects/Operations.md/#hascanonicalizer) 并向规范化框架注册该模式。

```c++
// Register our patterns for rewrite by the Canonicalization framework.
void TransposeOp::getCanonicalizationPatterns(
    RewritePatternSet &results, MLIRContext *context) {
  results.add<SimplifyRedundantTranspose>(context);
}
```

我们还需要更新主文件 `toyc.cpp`，添加一个优化流水线。在 MLIR 中，优化通过 `PassManager`（通道管理器）运行，方式与 LLVM 类似：

```c++
  mlir::PassManager pm(module->getName());
  pm.addNestedPass<mlir::toy::FuncOp>(mlir::createCanonicalizerPass());
```

最后，我们运行 `toyc-ch3 test/Examples/Toy/Ch3/transpose_transpose.toy -emit=mlir -opt`，观察模式的实际效果：

```mlir
toy.func @transpose_transpose(%arg0: tensor<*xf64>) -> tensor<*xf64> {
  %0 = toy.transpose(%arg0 : tensor<*xf64>) to tensor<*xf64>
  toy.return %arg0 : tensor<*xf64>
}
```

如预期所示，我们现在直接返回了函数参数，绕过了任何转置操作。然而，其中一个转置操作仍未被消除，这并不理想！发生这种情况是因为我们的模式用函数输入替换了最后一个变换，但留下了现在已成为死代码的转置输入。规范化器知道如何清理死操作；但是，MLIR 保守地假设操作可能存在副作用。我们可以通过向 `TransposeOp` 添加新的特性（trait）`Pure` 来修复这个问题：

```tablegen
def TransposeOp : Toy_Op<"transpose", [Pure]> {...}
```

现在让我们再试一次 `toyc-ch3 test/transpose_transpose.toy -emit=mlir -opt`：

```mlir
toy.func @transpose_transpose(%arg0: tensor<*xf64>) -> tensor<*xf64> {
  toy.return %arg0 : tensor<*xf64>
}
```

完美！没有 `transpose` 操作残留——代码已达到最优。

在下一节中，我们使用 DRR 对 Reshape 操作进行模式匹配优化。

## 使用 DRR 优化 Reshape 操作

声明式、基于规则的模式匹配与重写（DRR）是一种基于操作 DAG 的声明式重写器，提供表格式语法用于模式匹配和重写规则：

```tablegen
class Pattern<
    dag sourcePattern, list<dag> resultPatterns,
    list<dag> additionalConstraints = [],
    dag benefitsAdded = (addBenefit 0)>;
```

类似于 SimplifyRedundantTranspose 的冗余 Reshape 优化，使用 DRR 可以更简洁地表达如下：

```tablegen
// Reshape(Reshape(x)) = Reshape(x)
def ReshapeReshapeOptPattern : Pat<(ReshapeOp(ReshapeOp $arg)),
                                   (ReshapeOp $arg)>;
```

每个 DRR 模式对应自动生成的 C++ 代码可以在 `path/to/BUILD/tools/mlir/examples/toy/Ch3/ToyCombine.inc` 中找到。

当变换依赖于参数和结果的某些属性时，DRR 还提供了一种添加参数约束的方法。例如，当输入和输出形状相同时，消除冗余 Reshape 的变换：

```tablegen
def TypesAreIdentical : Constraint<CPred<"$0.getType() == $1.getType()">>;
def RedundantReshapeOptPattern : Pat<
  (ReshapeOp:$res $arg), (replaceWithValue $arg),
  [(TypesAreIdentical $res, $arg)]>;
```

某些优化可能需要对指令参数进行额外变换。这可以通过 NativeCodeCall 来实现，它允许通过调用 C++ 辅助函数或使用内联 C++ 来进行更复杂的变换。FoldConstantReshape 就是这样一个优化示例：我们通过原地重塑常量来优化对常量值的 Reshape 操作，并消除 Reshape 操作本身。

```tablegen
def ReshapeConstant : NativeCodeCall<"$0.reshape(($1.getType()).cast<ShapedType>())">;
def FoldConstantReshapeOptPattern : Pat<
  (ReshapeOp:$res (ConstantOp $arg)),
  (ConstantOp (ReshapeConstant $arg, $res))>;
```

我们使用以下 trivial_reshape.toy 程序来演示这些 Reshape 优化：

```c++
def main() {
  var a<2,1> = [1, 2];
  var b<2,1> = a;
  var c<2,1> = b;
  print(c);
}
```

```mlir
module {
  toy.func @main() {
    %0 = toy.constant dense<[1.000000e+00, 2.000000e+00]> : tensor<2xf64>
    %1 = toy.reshape(%0 : tensor<2xf64>) to tensor<2x1xf64>
    %2 = toy.reshape(%1 : tensor<2x1xf64>) to tensor<2x1xf64>
    %3 = toy.reshape(%2 : tensor<2x1xf64>) to tensor<2x1xf64>
    toy.print %3 : tensor<2x1xf64>
    toy.return
  }
}
```

我们尝试运行 `toyc-ch3 test/Examples/Toy/Ch3/trivial_reshape.toy -emit=mlir -opt`，观察模式的实际效果：

```mlir
module {
  toy.func @main() {
    %0 = toy.constant dense<[[1.000000e+00], [2.000000e+00]]> : tensor<2x1xf64>
    toy.print %0 : tensor<2x1xf64>
    toy.return
  }
}
```

如预期所示，规范化后没有任何 Reshape 操作残留。

有关声明式重写方法的更多详情，请参阅[表驱动声明式重写规则（DRR）](../../DeclarativeRewrites.md)。

在本章中，我们了解了如何通过始终可用的钩子（hook）来使用某些核心变换。在[下一章](Ch-4.md)中，我们将了解如何通过接口（Interfaces）使用可扩展性更强的通用解决方案。
