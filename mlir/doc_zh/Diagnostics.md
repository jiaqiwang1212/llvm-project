# 诊断基础设施

[TOC]

本文档介绍了如何使用 MLIR 诊断基础设施以及如何与之交互。

有关 MLIR、IR 结构、操作等的更多信息，请参阅 [MLIR 规范](LangRef.md)。

## 源代码位置

源代码位置信息对任何编译器来说都极为重要，因为它为可调试性和错误报告提供了基线。[内置方言](Dialects/Builtin.md)根据不同的场景需求提供了多种不同的位置属性类型。

## 诊断引擎

`DiagnosticEngine` 是 MLIR 中诊断的主要接口。它管理诊断处理器的注册，以及诊断发出的核心 API。处理器通常采用 `LogicalResult(Diagnostic &)` 的形式。如果结果为 `success`，表示诊断已被完全处理和消费。如果为 `failure`，表示诊断应传播到之前注册的处理器。可以通过 `MLIRContext` 实例与之交互。

```c++
DiagnosticEngine& engine = ctx->getDiagEngine();

/// 处理报告的诊断。
// 返回 success 表示诊断已完全处理，
// 或返回 failure 表示诊断应传播到之前的处理器。
DiagnosticEngine::HandlerID id = engine.registerHandler(
    [](Diagnostic &diag) -> LogicalResult {
  bool should_propagate_diagnostic = ...;
  return failure(should_propagate_diagnostic);
});


// 我们也可以完全省略返回值，在这种情况下引擎假定所有诊断都被消费（即 success() 结果）。
DiagnosticEngine::HandlerID id = engine.registerHandler([](Diagnostic &diag) {
  return;
});

// 完成后注销此处理器。
engine.eraseHandler(id);
```

### 构造诊断

如上所述，`DiagnosticEngine` 持有诊断发出的核心 API。可以通过 `emit` 向引擎发出新诊断。此方法返回一个 [InFlightDiagnostic](#inflight-diagnostic)，可以进一步修改。

```c++
InFlightDiagnostic emit(Location loc, DiagnosticSeverity severity);
```

但是，在 MLIR 中发出诊断通常不推荐直接使用 `DiagnosticEngine`。[`operation`](LangRef.md/#operations) 提供了用于发出诊断的工具方法：

```c++
// mlir 命名空间中可用的 `emit` 方法。
InFlightDiagnostic emitError/Remark/Warning(Location);

// 这些方法使用附加到操作的位置。
InFlightDiagnostic Operation::emitError/Remark/Warning();

// 此方法创建以 "'op-name' op " 为前缀的诊断。
InFlightDiagnostic Operation::emitOpError();
```

## 诊断

MLIR 中的 `Diagnostic` 包含向用户报告消息所需的所有信息。`Diagnostic` 本质上由四个主要组件组成：

*   [源代码位置](#source-locations)
*   严重性级别
    -   Error（错误）、Note（注释）、Remark（备注）、Warning（警告）
*   诊断参数
    -   诊断参数在构建输出消息时使用。
*   元数据
    -   附加的一些额外信息，可用于识别此诊断（而非源代码位置和严重性级别），
        例如供诊断处理器进行过滤。
        元数据不是输出消息的一部分。

### 附加参数

一旦构建了诊断，用户就可以开始组合它。诊断的输出消息由一组已附加到其上的诊断参数组成。可以通过几种不同方式向诊断附加新参数：

```c++
// 组合诊断时可用的一些有趣内容。
Attribute fooAttr;
Type fooType;
SmallVector<int> fooInts;

// 诊断可以通过流式操作符组合。
op->emitError() << "Compose an interesting error: " << fooAttr << ", " << fooType
                << ", (" << fooInts << ')';

// 这可能生成类似以下的内容（FuncAttr:@foo, IntegerType:i32, {0,1,2}）：
"Compose an interesting error: @foo, i32, (0, 1, 2)"
```

附加到诊断的操作将在严重性级别为 `Error` 时以通用形式打印，否则将使用自定义操作打印器。
```c++
// `anotherOp` 将以通用形式打印，
// 例如 %3 = "arith.addf"(%arg4, %2) : (f32, f32) -> f32
op->emitError() << anotherOp;

// `anotherOp` 将使用自定义打印器打印，
// 例如 %3 = arith.addf %arg4, %2 : f32
op->emitRemark() << anotherOp;
```

要使自定义类型与 Diagnostics 兼容，必须实现以下友元函数：

```c++
friend mlir::Diagnostic &operator<<(
    mlir::Diagnostic &diagnostic, const MyType &foo);
```

### 附加注释

与许多其他编译器框架不同，MLIR 中的注释不能直接发出。它们必须显式附加到另一个非注释类型的诊断上。在发出诊断时，可以通过 `attachNote` 直接附加注释。附加注释时，如果用户没有提供明确的源代码位置，注释将继承父诊断的位置。

```c++
// 发出带有明确源代码位置的注释。
op->emitError("...").attachNote(noteLoc) << "...";

// 发出继承父位置的注释。
op->emitError("...").attachNote() << "...";
```

### 管理元数据
元数据是一个可变的 DiagnosticArguments 向量。
可以作为向量访问和修改。


## InFlight 诊断

在解释了[诊断](#diagnostic)之后，我们介绍 `InFlightDiagnostic`，它是一个围绕待报告诊断的 RAII 封装器。这允许在诊断还在传播中时对其进行修改。如果用户没有直接报告它，它将在销毁时自动报告。

```c++
{
  InFlightDiagnostic diag = op->emitError() << "...";
}  // 诊断在此处自动报告。
```

## 诊断配置选项

提供了多个选项来帮助控制和增强诊断的行为。这些选项可以通过 MLIRContext 配置，并通过 `registerMLIRContextCLOptions` 方法注册到命令行。这些选项列举如下：

### 在诊断时打印操作

命令行标志：`-mlir-print-op-on-diagnostic`

当通过 `Operation::emitError/...` 在操作上发出诊断时，该操作的文本形式会被打印并作为注释附加到诊断中。此选项有助于了解可能无效的操作的当前形式，特别是在调试验证器失败时。示例输出如下：

```shell
test.mlir:3:3: error: 'module_terminator' op expects parent op 'builtin.module'
  "module_terminator"() : () -> ()
  ^
test.mlir:3:3: note: see current operation: "module_terminator"() : () -> ()
  "module_terminator"() : () -> ()
  ^
```

### 在诊断时打印栈跟踪

命令行标志：`-mlir-print-stacktrace-on-diagnostic`

当发出诊断时，将当前栈跟踪作为注释附加到诊断中。此选项有助于了解编译器的哪个部分生成了某些诊断。示例输出如下：

```shell
test.mlir:3:3: error: 'module_terminator' op expects parent op 'builtin.module'
  "module_terminator"() : () -> ()
  ^
test.mlir:3:3: note: diagnostic emitted with trace:
 #0 0x000055dd40543805 llvm::sys::PrintStackTrace(llvm::raw_ostream&) llvm/lib/Support/Unix/Signals.inc:553:11
 #1 0x000055dd3f8ac162 emitDiag(mlir::Location, mlir::DiagnosticSeverity, llvm::Twine const&) /lib/IR/Diagnostics.cpp:292:7
 #2 0x000055dd3f8abe8e mlir::emitError(mlir::Location, llvm::Twine const&) /lib/IR/Diagnostics.cpp:304:10
 #3 0x000055dd3f998e87 mlir::Operation::emitError(llvm::Twine const&) /lib/IR/Operation.cpp:324:29
 #4 0x000055dd3f99d21c mlir::Operation::emitOpError(llvm::Twine const&) /lib/IR/Operation.cpp:652:10
 #5 0x000055dd3f96b01c mlir::OpTrait::HasParent<mlir::ModuleOp>::Impl<mlir::ModuleTerminatorOp>::verifyTrait(mlir::Operation*) /mlir/IR/OpDefinition.h:897:18
 #6 0x000055dd3f96ab38 mlir::Op<mlir::ModuleTerminatorOp, mlir::OpTrait::ZeroOperands, mlir::OpTrait::ZeroResults, mlir::OpTrait::HasParent<mlir::ModuleOp>::Impl, mlir::OpTrait::IsTerminator>::BaseVerifier<mlir::OpTrait::HasParent<mlir::ModuleOp>::Impl<mlir::ModuleTerminatorOp>, mlir::OpTrait::IsTerminator<mlir::ModuleTerminatorOp> >::verifyTrait(mlir::Operation*) /mlir/IR/OpDefinition.h:1052:29
 #  ...
  "module_terminator"() : () -> ()
  ^
```

## 常见诊断处理器

要与诊断基础设施交互，用户需要向 [`DiagnosticEngine`](#diagnostic-engine) 注册诊断处理器。认识到许多用户希望具有相同的处理器功能，MLIR 提供了几种常见的诊断处理器供立即使用。

### 作用域诊断处理器

此诊断处理器是一个简单的 RAII 类，用于注册和注销给定的诊断处理器。此类可以直接使用，也可以与派生的诊断处理器结合使用。

```c++
// 直接构造处理器。
MLIRContext context;
ScopedDiagnosticHandler scopedHandler(&context, [](Diagnostic &diag) {
  ...
});

// 与另一个处理器结合使用。
class MyDerivedHandler : public ScopedDiagnosticHandler {
  MyDerivedHandler(MLIRContext *ctx) : ScopedDiagnosticHandler(ctx) {
    // 设置应由 RAII 管理的处理器。
    setHandler([&](Diagnostic diag) {
      ...
    });
  }
};
```

### SourceMgr 诊断处理器

此诊断处理器是围绕 llvm::SourceMgr 实例的封装器。它支持在相应源文件的一行内联显示诊断消息。当尝试显示诊断的源行时，此处理器还会自动将新看到的源文件加载到 SourceMgr 中。此处理器的示例用法可以在 `mlir-opt` 工具中看到。

```shell
$ mlir-opt foo.mlir

/tmp/test.mlir:6:24: error: expected non-function type
func.func @foo() -> (index, ind) {
                       ^
```

要在你的工具中使用此处理器，请添加以下内容：

```c++
SourceMgr sourceMgr;
MLIRContext context;
SourceMgrDiagnosticHandler sourceMgrHandler(sourceMgr, &context);
```

#### 过滤位置

在某些情况下，诊断可能以非常深的调用栈中的调用位置发出，其中许多帧与用户源代码无关。当用户源代码与大型框架或库的代码交织在一起时，这种情况经常出现。在这些情况下，诊断的上下文往往被无关的框架源代码位置所混淆。为了帮助减轻这种混淆，`SourceMgrDiagnosticHandler` 支持过滤向用户显示哪些位置。要启用过滤，用户只需在构造时向 `SourceMgrDiagnosticHandler` 提供一个过滤函数，该函数指示哪些位置应该显示。下面是一个简单示例：

```c++
// 在这里我们定义控制哪些位置显示给用户的函数子对象（functor）。
// 此函数在位置应显示时返回 true，否则返回 false。
// 当过滤容器位置（例如 NameLoc）时，此函数不应递归进入子位置。
// 调用者会根据需要递归进入嵌套位置。
auto shouldShowFn = [](Location loc) -> bool {
  FileLineColLoc fileLoc = dyn_cast<FileLineColLoc>(loc);

  // 我们不对非文件位置执行任何过滤。
  // 提示：调用者将递归进入任何必要的子位置。
  if (!fileLoc)
    return true;

  // 不显示包含我们框架代码的文件位置。
  return !fileLoc.getFilename().strref().contains("my/framework/source/");
};

SourceMgr sourceMgr;
MLIRContext context;
SourceMgrDiagnosticHandler sourceMgrHandler(sourceMgr, &context, shouldShowFn);
```

注意：如果所有位置都被过滤掉，栈中的第一个位置仍然会被显示。

### SourceMgr 诊断验证处理器

此处理器是围绕 llvm::SourceMgr 的封装器，用于验证某些诊断是否已被发送到上下文。要使用此处理器，请以以下形式在源文件中注释预期诊断：

*   `expected-(error|note|remark|warning)(-re)? {{ message }}`

提供的 `message` 是一个预期包含在生成的诊断中的字符串。`-re` 后缀可用于在 `message` 中启用正则表达式匹配。当存在时，`message` 可以在 `{{` `}}` 块中定义正则表达式匹配序列。正则表达式匹配器支持扩展 POSIX 正则表达式（ERE）。下面展示了几个示例：

```mlir
// 预期在同一行出现错误。
func.func @bad_branch() {
  cf.br ^missing  // expected-error {{reference to an undefined block}}
}

// 预期在相邻行出现错误。
func.func @foo(%a : f32) {
  // expected-error@+1 {{unknown comparison predicate "foo"}}
  %result = arith.cmpf "foo", %a, %a : f32
  return
}

// 预期在下一个不含指示符的行出现错误。
// expected-remark@below {{remark on function below}}
// expected-remark@below {{another remark on function below}}
func.func @bar(%a : f32)

// 预期在上一个不含指示符的行出现错误。
func.func @baz(%a : f32)
// expected-remark@above {{remark on function above}}
// expected-remark@above {{another remark on function above}}

// 预期出现提及父函数的错误，但使用正则表达式避免硬编码名称。
func.func @foo() -> i32 {
  // expected-error-re@+1 {{'func.return' op has 0 operands, but enclosing function (@{{.*}}) returns 1}}
  return
}
```

如果出现任何意外的诊断，或者任何预期的诊断没有被产生，处理器将报告错误。

```shell
$ mlir-opt foo.mlir

/tmp/test.mlir:6:24: error: unexpected error: expected non-function type
func.func @foo() -> (index, ind) {
                       ^

/tmp/test.mlir:15:4: error: expected remark "expected some remark" was not produced
// expected-remark {{expected some remark}}
   ^~~~~~~~~~~~~~~~~~~~~~~~~~
```

与 [SourceMgr 诊断处理器](#sourcemgr-diagnostic-handler) 类似，此处理器可以通过以下方式添加到任何工具中：

```c++
SourceMgr sourceMgr;
MLIRContext context;
SourceMgrDiagnosticVerifierHandler sourceMgrHandler(sourceMgr, &context);
```

### 并行诊断处理器

MLIR 从一开始就被设计为多线程的。多线程时需要牢记的一件重要事情是确定性。这意味着在多个线程上运行时看到的行为与在单个线程上运行时相同。对于诊断，这意味着无论使用多少线程，诊断的顺序都是相同的。引入 ParallelDiagnosticHandler 就是为了解决这个问题。

创建此类型的处理器后，唯一剩余的步骤是确保每个将向处理器发出诊断的线程都设置了相应的 'orderID'。orderID 对应于同步执行时诊断的发出顺序。例如，如果我们在单线程上处理操作列表 [a, b, c]，处理操作 'a' 时发出的诊断将在 'b' 或 'c' 的诊断之前发出。这与 'orderID' 一一对应。处理 'a' 的线程应将 orderID 设置为 '0'；处理 'b' 的线程应将其设置为 '1'；以此类推。这为处理器提供了一种方式，可以根据接收诊断的线程确定性地对其进行排序。

下面是一个简单示例：

```c++
MLIRContext *context = ...;
ParallelDiagnosticHandler handler(context);

// 并行处理操作列表。
std::vector<Operation *> opsToProcess = ...;
llvm::parallelFor(0, opsToProcess.size(), [&](size_t i) {
  // 通知处理器我们正在处理第 i 个操作。
  handler.setOrderIDForThread(i);
  auto *op = opsToProcess[i];
  ...

  // 通知处理器我们已完成在此线程上处理诊断。
  handler.eraseOrderIDForThread();
});
```
