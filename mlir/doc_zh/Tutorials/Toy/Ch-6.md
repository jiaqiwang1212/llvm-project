# 第六章：下降到 LLVM 并生成代码

[TOC]

在[上一章](Ch-5.md)中，我们介绍了[方言转换](../../DialectConversion.md)框架，并将许多 `Toy` 操作部分下降到仿射循环嵌套以进行优化。在本章中，我们将最终下降到 LLVM 以生成代码。

## 下降到 LLVM

对于此次下降，我们将再次使用方言转换框架来完成繁重的工作。但这一次，我们将执行到 [LLVM 方言](../../Dialects/LLVM.md)的完全转换。幸运的是，我们已经下降了除一个之外的所有 `toy` 操作，最后剩下的是 `toy.print`。在介绍向 LLVM 的转换之前，让我们先下降 `toy.print` 操作。我们将把这个操作下降为一个调用 `printf` 打印每个元素的非仿射循环嵌套。注意，由于方言转换框架支持[传递式下降](../../../getting_started/Glossary.md/#transitive-lowering)（transitive lowering），我们不需要直接在 LLVM 方言中生成操作。所谓传递式下降，是指转换框架可以应用多个模式来完全合法化一个操作。在此示例中，我们生成的是结构化循环嵌套，而不是 LLVM 方言中的分支形式。只要我们随后有从循环操作到 LLVM 的下降，整体下降仍然会成功。

在下降期间，我们可以如下获取或构建 printf 的声明：

```c++
/// Return a symbol reference to the printf function, inserting it into the
/// module if necessary.
static FlatSymbolRefAttr getOrInsertPrintf(PatternRewriter &rewriter,
                                           ModuleOp module,
                                           LLVM::LLVMDialect *llvmDialect) {
  auto *context = module.getContext();
  if (module.lookupSymbol<LLVM::LLVMFuncOp>("printf"))
    return SymbolRefAttr::get(context, "printf");

  // Create a function declaration for printf, the signature is:
  //   * `i32 (i8*, ...)`
  auto llvmI32Ty = IntegerType::get(context, 32);
  auto llvmI8PtrTy =
      LLVM::LLVMPointerType::get(IntegerType::get(context, 8));
  auto llvmFnType = LLVM::LLVMFunctionType::get(llvmI32Ty, llvmI8PtrTy,
                                                /*isVarArg=*/true);

  // Insert the printf function into the body of the parent module.
  PatternRewriter::InsertionGuard insertGuard(rewriter);
  rewriter.setInsertionPointToStart(module.getBody());
  LLVM::LLVMFuncOp::create(rewriter, module.getLoc(), "printf", llvmFnType);
  return SymbolRefAttr::get(context, "printf");
}
```

现在 printf 操作的下降已定义好，我们可以指定下降所需的组件。这些组件与[上一章](Ch-5.md)中定义的组件大体相同。

### 转换目标

对于此次转换，除了顶层模块之外，我们将把所有内容下降到 LLVM 方言。

```c++
  mlir::ConversionTarget target(getContext());
  target.addLegalDialect<mlir::LLVMDialect>();
  target.addLegalOp<mlir::ModuleOp>();
```

### 类型转换器

此次下降还将把当前正在操作的 MemRef 类型转换为 LLVM 中的表示形式。为了执行此转换，我们在下降中使用 TypeConverter。该转换器指定一种类型如何映射到另一种类型。由于现在我们正在执行涉及块参数（block argument）的更复杂的下降，这一步变得必要。由于我们没有任何需要下降的 Toy 方言特有类型，默认转换器对我们的用例已经足够。

```c++
  LLVMTypeConverter typeConverter(&getContext());
```

### 转换模式

现在转换目标已定义好，我们需要提供用于下降的模式。在编译过程的此阶段，我们有 `toy`、`affine`、`arith` 和 `std` 操作的组合。幸运的是，`affine`、`arith` 和 `std` 方言已经提供了将它们转换为 LLVM 方言所需的模式集合。这些模式通过依赖[传递式下降](../../../getting_started/Glossary.md/#transitive-lowering)，允许在多个阶段下降中间表示（IR）。

```c++
  mlir::RewritePatternSet patterns(&getContext());
  mlir::populateAffineToStdConversionPatterns(patterns, &getContext());
  mlir::cf::populateSCFToControlFlowConversionPatterns(patterns, &getContext());
  mlir::arith::populateArithToLLVMConversionPatterns(typeConverter,
                                                          patterns);
  mlir::populateFuncToLLVMConversionPatterns(typeConverter, patterns);
  mlir::cf::populateControlFlowToLLVMConversionPatterns(patterns, &getContext());

  // The only remaining operation, to lower from the `toy` dialect, is the
  // PrintOp.
  patterns.add<PrintOpLowering>(&getContext());
```

### 完全下降

我们希望完全下降到 LLVM，因此使用 `FullConversion`。这确保了转换后只有合法操作会保留。

```c++
  mlir::ModuleOp module = getOperation();
  if (mlir::failed(mlir::applyFullConversion(module, target, patterns)))
    signalPassFailure();
```

回顾我们当前的工作示例：

```mlir
toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %2 = toy.transpose(%0 : tensor<2x3xf64>) to tensor<3x2xf64>
  %3 = toy.mul %2, %2 : tensor<3x2xf64>
  toy.print %3 : tensor<3x2xf64>
  toy.return
}
```

我们现在可以下降到 LLVM 方言，生成以下代码：

```mlir
llvm.func @free(!llvm<"i8*">)
llvm.func @printf(!llvm<"i8*">, ...) -> i32
llvm.func @malloc(i64) -> !llvm<"i8*">
llvm.func @main() {
  %0 = llvm.mlir.constant(1.000000e+00 : f64) : f64
  %1 = llvm.mlir.constant(2.000000e+00 : f64) : f64

  ...

^bb16:
  %221 = llvm.extractvalue %25[0] : !llvm<"{ double*, i64, [2 x i64], [2 x i64] }">
  %222 = llvm.mlir.constant(0 : index) : i64
  %223 = llvm.mlir.constant(2 : index) : i64
  %224 = llvm.mul %214, %223 : i64
  %225 = llvm.add %222, %224 : i64
  %226 = llvm.mlir.constant(1 : index) : i64
  %227 = llvm.mul %219, %226 : i64
  %228 = llvm.add %225, %227 : i64
  %229 = llvm.getelementptr %221[%228] : (!llvm."double*">, i64) -> !llvm<"f64*">
  %230 = llvm.load %229 : !llvm<"double*">
  %231 = llvm.call @printf(%207, %230) : (!llvm<"i8*">, f64) -> i32
  %232 = llvm.add %219, %218 : i64
  llvm.br ^bb15(%232 : i64)

  ...

^bb18:
  %235 = llvm.extractvalue %65[0] : !llvm<"{ double*, i64, [2 x i64], [2 x i64] }">
  %236 = llvm.bitcast %235 : !llvm<"double*"> to !llvm<"i8*">
  llvm.call @free(%236) : (!llvm<"i8*">) -> ()
  %237 = llvm.extractvalue %45[0] : !llvm<"{ double*, i64, [2 x i64], [2 x i64] }">
  %238 = llvm.bitcast %237 : !llvm<"double*"> to !llvm<"i8*">
  llvm.call @free(%238) : (!llvm<"i8*">) -> ()
  %239 = llvm.extractvalue %25[0] : !llvm<"{ double*, i64, [2 x i64], [2 x i64] }">
  %240 = llvm.bitcast %239 : !llvm<"double*"> to !llvm<"i8*">
  llvm.call @free(%240) : (!llvm<"i8*">) -> ()
  llvm.return
}
```

有关下降到 LLVM 方言的更深入细节，请参阅 [LLVM IR 目标](../../TargetLLVMIR.md)。

## 代码生成：走出 MLIR

此时我们正处于代码生成的边缘。我们可以用 LLVM 方言生成代码，所以现在只需要导出到 LLVM IR 并设置 JIT 来运行它。

### 生成 LLVM IR

现在我们的模块只包含 LLVM 方言中的操作，我们可以将其导出到 LLVM IR。若要以编程方式完成此操作，可以调用以下实用工具：

```c++
  std::unique_ptr<llvm::Module> llvmModule = mlir::translateModuleToLLVMIR(module);
  if (!llvmModule)
    /* ... an error was encountered ... */
```

将我们的模块导出到 LLVM IR 会生成：

```llvm
define void @main() {
  ...

102:
  %103 = extractvalue { double*, i64, [2 x i64], [2 x i64] } %8, 0
  %104 = mul i64 %96, 2
  %105 = add i64 0, %104
  %106 = mul i64 %100, 1
  %107 = add i64 %105, %106
  %108 = getelementptr double, double* %103, i64 %107
  %109 = memref.load double, double* %108
  %110 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double %109)
  %111 = add i64 %100, 1
  cf.br label %99

  ...

115:
  %116 = extractvalue { double*, i64, [2 x i64], [2 x i64] } %24, 0
  %117 = bitcast double* %116 to i8*
  call void @free(i8* %117)
  %118 = extractvalue { double*, i64, [2 x i64], [2 x i64] } %16, 0
  %119 = bitcast double* %118 to i8*
  call void @free(i8* %119)
  %120 = extractvalue { double*, i64, [2 x i64], [2 x i64] } %8, 0
  %121 = bitcast double* %120 to i8*
  call void @free(i8* %121)
  ret void
}
```

如果我们对生成的 LLVM IR 启用优化，可以大幅精简：

```llvm
define void @main()
  %0 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 1.000000e+00)
  %1 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 1.600000e+01)
  %putchar = tail call i32 @putchar(i32 10)
  %2 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 4.000000e+00)
  %3 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 2.500000e+01)
  %putchar.1 = tail call i32 @putchar(i32 10)
  %4 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 9.000000e+00)
  %5 = tail call i32 (i8*, ...) @printf(i8* nonnull dereferenceable(1) getelementptr inbounds ([4 x i8], [4 x i8]* @frmt_spec, i64 0, i64 0), double 3.600000e+01)
  %putchar.2 = tail call i32 @putchar(i32 10)
  ret void
}
```

转储 LLVM IR 的完整代码清单可以在 `examples/toy/Ch6/toyc.cpp` 中的 `dumpLLVMIR()` 函数中找到：

```c++

int dumpLLVMIR(mlir::ModuleOp module) {
  // Translate the module, that contains the LLVM dialect, to LLVM IR. Use a
  // fresh LLVM IR context. (Note that LLVM is not thread-safe and any
  // concurrent use of a context requires external locking.)
  llvm::LLVMContext llvmContext;
  auto llvmModule = mlir::translateModuleToLLVMIR(module, llvmContext);
  if (!llvmModule) {
    llvm::errs() << "Failed to emit LLVM IR\n";
    return -1;
  }

  // Initialize LLVM targets.
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  mlir::ExecutionEngine::setupTargetTriple(llvmModule.get());

  /// Optionally run an optimization pipeline over the llvm module.
  auto optPipeline = mlir::makeOptimizingTransformer(
      /*optLevel=*/EnableOpt ? 3 : 0, /*sizeLevel=*/0,
      /*targetMachine=*/nullptr);
  if (auto err = optPipeline(llvmModule.get())) {
    llvm::errs() << "Failed to optimize LLVM IR " << err << "\n";
    return -1;
  }
  llvm::errs() << *llvmModule << "\n";
  return 0;
}
```

### 设置 JIT

使用 `mlir::ExecutionEngine` 基础设施可以设置 JIT 来运行包含 LLVM 方言的模块。这是围绕 LLVM JIT 的一个实用封装，接受 `.mlir` 作为输入。设置 JIT 的完整代码清单可以在 `Ch6/toyc.cpp` 的 `runJit()` 函数中找到：

```c++
int runJit(mlir::ModuleOp module) {
  // Initialize LLVM targets.
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  // An optimization pipeline to use within the execution engine.
  auto optPipeline = mlir::makeOptimizingTransformer(
      /*optLevel=*/EnableOpt ? 3 : 0, /*sizeLevel=*/0,
      /*targetMachine=*/nullptr);

  // Create an MLIR execution engine. The execution engine eagerly JIT-compiles
  // the module.
  auto maybeEngine = mlir::ExecutionEngine::create(module,
      /*llvmModuleBuilder=*/nullptr, optPipeline);
  assert(maybeEngine && "failed to construct an execution engine");
  auto &engine = maybeEngine.get();

  // Invoke the JIT-compiled function.
  auto invocationResult = engine->invoke("main");
  if (invocationResult) {
    llvm::errs() << "JIT invocation failed\n";
    return -1;
  }

  return 0;
}
```

你可以在构建目录下尝试运行：

```shell
$ echo 'def main() { print([[1, 2], [3, 4]]); }' | ./bin/toyc-ch6 -emit=jit
1.000000 2.000000
3.000000 4.000000
```

你也可以使用 `-emit=mlir`、`-emit=mlir-affine`、`-emit=mlir-llvm` 和 `-emit=llvm` 来比较所涉及的各个 IR 层级。还可以尝试像 [`--mlir-print-ir-after-all`](../../PassManagement.md/#ir-printing) 这样的选项，以追踪 IR 在整个流水线中的演变。

本节使用的示例代码可以在 test/Examples/Toy/Ch6/llvm-lowering.mlir 中找到。

到目前为止，我们使用的是基本数据类型。在[下一章](Ch-7.md)中，我们将添加一个复合的 `struct` 类型。
