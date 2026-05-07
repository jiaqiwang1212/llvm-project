# 第五章：部分下降到低级方言进行优化

[TOC]

此时，我们迫切希望生成实际的代码，并看到我们的 Toy 语言焕发生机。我们将使用 LLVM 来生成代码，但仅展示 LLVM 构建器接口并不会令人兴奋。相反，我们将展示如何通过在同一函数中共存的多个方言（dialect）混合来执行渐进式下降（lowering）。

为了使其更有趣，在本章中，我们将考虑重用在优化仿射变换的方言中已实现的现有优化：`Affine` 方言。该方言专为程序的计算密集部分而设计，并有一定限制：例如，它不支持表示我们的 `toy.print` 内建操作，也不应该支持！相反，我们可以将 `Affine` 作为 Toy 中计算密集部分的目标，并在[下一章](Ch-6.md)中直接以 `LLVM IR` 方言为目标来下降 `print`。在此下降过程中，我们将从 `Toy` 所操作的 [TensorType](../../Dialects/Builtin.md/#rankedtensortype) 下降到通过仿射循环嵌套索引的 [MemRefType](../../Dialects/Builtin.md/#memreftype)。张量（Tensor）表示数据的抽象值类型序列，这意味着它们不存在于任何内存中。而 MemRef 则表示较低层次的缓冲区访问，因为它们是对内存某个区域的具体引用。

# 方言转换

MLIR 有许多不同的方言（dialect），因此拥有一个统一的框架来[转换](../../../getting_started/Glossary.md/#conversion)它们非常重要。这就是 `DialectConversion` 框架发挥作用的地方。该框架允许将一组*非法*操作（operation）转换为一组*合法*操作。要使用此框架，我们需要提供两样东西（以及可选的第三样）：

*   [转换目标](../../DialectConversion.md/#conversion-target)（Conversion Target）

    -   这是对哪些操作或方言对于转换是合法的正式规范。不合法的操作将需要重写模式（rewrite pattern）来执行[合法化](../../../getting_started/Glossary.md/#legalization)。

*   一组[重写模式](../../DialectConversion.md/#rewrite-pattern-specification)（Rewrite Patterns）

    -   这是用于将*非法*操作转换为零个或多个*合法*操作的[模式](../QuickstartRewrites.md)集合。

*   可选的[类型转换器](../../DialectConversion.md/#type-conversion)（Type Converter）。

    -   如果提供，它用于转换块参数（block argument）的类型。对于我们的转换，我们不需要此项。

## 转换目标

就我们的目的而言，我们希望将计算密集型的 `Toy` 操作转换为来自 `Affine`、`Arith`、`Func` 和 `MemRef` 方言的操作组合，以便进一步优化。作为下降的第一步，我们首先定义转换目标：

```c++
void ToyToAffineLoweringPass::runOnOperation() {
  // The first thing to define is the conversion target. This will define the
  // final target for this lowering.
  mlir::ConversionTarget target(getContext());

  // We define the specific operations, or dialects, that are legal targets for
  // this lowering. In our case, we are lowering to a combination of the
  // `Affine`, `Arith`, `Func`, and `MemRef` dialects.
  target.addLegalDialect<affine::AffineDialect, arith::ArithDialect,
                         func::FuncDialect, memref::MemRefDialect>();

  // We also define the Toy dialect as Illegal so that the conversion will fail
  // if any of these operations are *not* converted. Given that we actually want
  // a partial lowering, we explicitly mark the Toy operations that don't want
  // to lower, `toy.print`, as *legal*. `toy.print` will still need its operands
  // to be updated though (as we convert from TensorType to MemRefType), so we
  // only treat it as `legal` if its operands are legal.
  target.addIllegalDialect<ToyDialect>();
  target.addDynamicallyLegalOp<toy::PrintOp>([](toy::PrintOp op) {
    return llvm::none_of(op->getOperandTypes(), llvm::IsaPred<TensorType>);
  });
  ...
}
```

在上面，我们首先将 toy 方言设置为非法，然后将 print 操作设置为合法。我们也可以反过来做。单个操作始终优先于（更通用的）方言定义，因此顺序无关紧要。详情请参阅 `ConversionTarget::getOpInfo`。

## 转换模式

定义转换目标之后，我们可以定义如何将*非法*操作转换为*合法*操作。类似于在[第三章](Ch-3.md)中介绍的规范化框架，[`DialectConversion` 框架](../../DialectConversion.md)使用一种特殊的 `ConversionPattern` 来执行转换逻辑。`ConversionPatterns` 与传统的 `RewritePatterns` 不同，它们接受一个额外的 `operands`（或 `adaptor`）参数，该参数包含已被重映射/替换的操作数（operand）。这在处理类型转换时很有用，因为模式希望在新类型的值上操作，但匹配的是旧类型。对于我们的下降，这个不变量非常有用，因为它从当前操作的 [TensorType](../../Dialects/Builtin.md/#rankedtensortype) 转换到 [MemRefType](../../Dialects/Builtin.md/#memreftype)。让我们看一段下降 `toy.transpose` 操作的代码片段：

```c++
/// Lower the `toy.transpose` operation to an affine loop nest.
struct TransposeOpLowering : public OpConversionPattern<toy::TransposeOp> {
  using OpConversionPattern<toy::TransposeOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(toy::TransposeOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto loc = op->getLoc();
    lowerOpToLoops(op, rewriter,
                   [&](OpBuilder &builder, ValueRange loopIvs) {
                     Value input = adaptor.getInput();

                     // Transpose the elements by generating a load from the
                     // reverse indices.
                     SmallVector<Value, 2> reverseIvs(llvm::reverse(loopIvs));
                     return affine::AffineLoadOp::create(builder, loc, input,
                                                         reverseIvs);
                   });
    return success();
  }
};
```

现在我们可以准备在下降过程中使用的模式列表：

```c++
void ToyToAffineLoweringPass::runOnOperation() {
  ...

  // Now that the conversion target has been defined, we just need to provide
  // the set of patterns that will lower the Toy operations.
  mlir::RewritePatternSet patterns(&getContext());
  patterns.add<..., TransposeOpLowering>(&getContext());

  ...
```

## 部分下降

定义好模式之后，我们可以执行实际的下降。`DialectConversion` 框架提供了几种不同的下降模式，但就我们的目的而言，我们将执行部分下降（partial lowering），因为我们此时不会转换 `toy.print`。

```c++
void ToyToAffineLoweringPass::runOnOperation() {
  ...

  // With the target and rewrite patterns defined, we can now attempt the
  // conversion. The conversion will signal failure if any of our *illegal*
  // operations were not converted successfully.
  if (mlir::failed(mlir::applyPartialConversion(getOperation(), target, patterns)))
    signalPassFailure();
}
```

### 部分下降的设计考量

在深入了解下降结果之前，现在是讨论部分下降设计考量的好时机。在我们的下降中，我们从值类型 TensorType 转换为已分配（类缓冲区）类型 MemRefType。然而，由于我们没有下降 `toy.print` 操作，我们需要临时桥接这两个世界。有很多方式可以实现这一点，每种方式都有其自身的权衡：

*   从缓冲区生成 `load` 操作

    一种选择是从缓冲区类型生成 `load` 操作，以实例化值类型的实例。这允许 `toy.print` 操作的定义保持不变。这种方式的缺点是 `affine` 方言上的优化是有限的，因为 `load` 实际上涉及一次完整的拷贝，而该拷贝仅在我们的优化执行*之后*才可见。

*   生成一个新版本的 `toy.print`，在下降后的类型上操作

    另一种选择是拥有另一个已下降的 `toy.print` 变体，该变体在下降后的类型上操作。这种选择的好处是对优化器没有隐藏的、不必要的拷贝。缺点是需要另一个操作定义，它可能会重复第一个定义的许多方面。在 [ODS](../../DefiningDialects/Operations.md) 中定义一个基类可能会简化这一点，但你仍然需要分别处理这些操作。

*   更新 `toy.print` 以允许在下降后的类型上操作

    第三种选择是更新 `toy.print` 的当前定义，以允许在下降后的类型上操作。这种方式的好处是简单，不会引入额外的隐藏拷贝，也不需要另一个操作定义。这种选择的缺点是它需要在 `Toy` 方言中混合抽象层级。

为了简单起见，我们将在此下降中使用第三种选择。这涉及更新操作定义文件中 PrintOp 的类型约束：

```tablegen
def PrintOp : Toy_Op<"print"> {
  ...

  // The print operation takes an input tensor to print.
  // We also allow a F64MemRef to enable interop during partial lowering.
  let arguments = (ins AnyTypeOf<[F64Tensor, F64MemRef]>:$input);
}
```

## 完整的 Toy 示例

让我们看一个具体的例子：

```mlir
toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %2 = toy.transpose(%0 : tensor<2x3xf64>) to tensor<3x2xf64>
  %3 = toy.mul %2, %2 : tensor<3x2xf64>
  toy.print %3 : tensor<3x2xf64>
  toy.return
}
```

将仿射下降添加到我们的流水线后，我们现在可以生成：

```mlir
func.func @main() {
  %cst = arith.constant 1.000000e+00 : f64
  %cst_0 = arith.constant 2.000000e+00 : f64
  %cst_1 = arith.constant 3.000000e+00 : f64
  %cst_2 = arith.constant 4.000000e+00 : f64
  %cst_3 = arith.constant 5.000000e+00 : f64
  %cst_4 = arith.constant 6.000000e+00 : f64

  // Allocating buffers for the inputs and outputs.
  %0 = memref.alloc() : memref<3x2xf64>
  %1 = memref.alloc() : memref<3x2xf64>
  %2 = memref.alloc() : memref<2x3xf64>

  // Initialize the input buffer with the constant values.
  affine.store %cst, %2[0, 0] : memref<2x3xf64>
  affine.store %cst_0, %2[0, 1] : memref<2x3xf64>
  affine.store %cst_1, %2[0, 2] : memref<2x3xf64>
  affine.store %cst_2, %2[1, 0] : memref<2x3xf64>
  affine.store %cst_3, %2[1, 1] : memref<2x3xf64>
  affine.store %cst_4, %2[1, 2] : memref<2x3xf64>

  // Load the transpose value from the input buffer and store it into the
  // next input buffer.
  affine.for %arg0 = 0 to 3 {
    affine.for %arg1 = 0 to 2 {
      %3 = affine.load %2[%arg1, %arg0] : memref<2x3xf64>
      affine.store %3, %1[%arg0, %arg1] : memref<3x2xf64>
    }
  }

  // Multiply and store into the output buffer.
  affine.for %arg0 = 0 to 3 {
    affine.for %arg1 = 0 to 2 {
      %3 = affine.load %1[%arg0, %arg1] : memref<3x2xf64>
      %4 = affine.load %1[%arg0, %arg1] : memref<3x2xf64>
      %5 = arith.mulf %3, %4 : f64
      affine.store %5, %0[%arg0, %arg1] : memref<3x2xf64>
    }
  }

  // Print the value held by the buffer.
  toy.print %0 : memref<3x2xf64>
  memref.dealloc %2 : memref<2x3xf64>
  memref.dealloc %1 : memref<3x2xf64>
  memref.dealloc %0 : memref<3x2xf64>
  return
}
```

## 利用仿射优化

我们朴素的下降是正确的，但在效率方面还有很多不足之处。例如，`toy.mul` 的下降生成了一些冗余的 load 操作。让我们看看如何通过向流水线添加一些现有的优化来清理这些问题。向流水线添加 `LoopFusion` 和 `AffineScalarReplacement` 通道（pass）会给出以下结果：

```mlir
func.func @main() {
  %cst = arith.constant 1.000000e+00 : f64
  %cst_0 = arith.constant 2.000000e+00 : f64
  %cst_1 = arith.constant 3.000000e+00 : f64
  %cst_2 = arith.constant 4.000000e+00 : f64
  %cst_3 = arith.constant 5.000000e+00 : f64
  %cst_4 = arith.constant 6.000000e+00 : f64

  // Allocating buffers for the inputs and outputs.
  %0 = memref.alloc() : memref<3x2xf64>
  %1 = memref.alloc() : memref<2x3xf64>

  // Initialize the input buffer with the constant values.
  affine.store %cst, %1[0, 0] : memref<2x3xf64>
  affine.store %cst_0, %1[0, 1] : memref<2x3xf64>
  affine.store %cst_1, %1[0, 2] : memref<2x3xf64>
  affine.store %cst_2, %1[1, 0] : memref<2x3xf64>
  affine.store %cst_3, %1[1, 1] : memref<2x3xf64>
  affine.store %cst_4, %1[1, 2] : memref<2x3xf64>

  affine.for %arg0 = 0 to 3 {
    affine.for %arg1 = 0 to 2 {
      // Load the transpose value from the input buffer.
      %2 = affine.load %1[%arg1, %arg0] : memref<2x3xf64>

      // Multiply and store into the output buffer.
      %3 = arith.mulf %2, %2 : f64
      affine.store %3, %0[%arg0, %arg1] : memref<3x2xf64>
    }
  }

  // Print the value held by the buffer.
  toy.print %0 : memref<3x2xf64>
  memref.dealloc %1 : memref<2x3xf64>
  memref.dealloc %0 : memref<3x2xf64>
  return
}
```

在这里，我们可以看到一个冗余的内存分配被移除，两个循环嵌套被融合，一些不必要的 `load` 被移除。你可以构建 `toyc-ch5` 并亲自尝试：`toyc-ch5 test/Examples/Toy/Ch5/affine-lowering.mlir -emit=mlir-affine`。我们也可以通过添加 `-opt` 来检查我们的优化效果。

在本章中，我们探讨了部分下降的一些方面，目的是进行优化。在[下一章](Ch-6.md)中，我们将继续讨论以 LLVM 为目标进行代码生成的方言转换。
