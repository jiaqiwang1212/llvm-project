# Pass 基础设施

[TOC]

Pass（编译遍）是变换（transformation）和优化（optimization）的基本基础设施。本文档概述了 MLIR 中的 pass 基础设施及其使用方式。

有关 MLIR 及其核心方面（如 IR（中间表示）结构和操作（operation））的更多信息，请参阅 [MLIR 规范](LangRef.md)。

有关 MLIR 中图重写的快速入门，请参阅 [MLIR 重写](Tutorials/QuickstartRewrites.md)。如果变换涉及模式匹配操作有向无环图，这是一个很好的起点。

## 操作 Pass

在 MLIR 中，抽象和变换的主要单元是[操作（operation）](LangRef.md/#operations)。因此，pass 管理器被设计为在不同嵌套层级的操作实例上工作。在以下段落中，我们将 pass 所操作的操作称为"当前操作"。

[pass 管理器](#pass-manager)的结构以及嵌套的概念将在下文中详细介绍。MLIR 中的所有 pass 都派生自 `OperationPass`，并遵守以下限制；任何不合规行为都会在多线程和其他高级场景中导致问题：

*   不得检查与当前操作同级的操作的状态，也不得访问这些同级操作下嵌套的操作。
    *   其他线程可能正在并行修改这些操作。
    *   允许检查祖先/父操作的状态。
*   不得修改除当前操作下嵌套的操作之外的操作状态。这包括从祖先/父块中添加、修改或删除其他操作。
    *   其他线程可能同时在这些操作上运行。
    *   作为例外，可以自由修改当前操作的属性（attribute）。这是修改当前操作的唯一方式。（即，不允许修改操作数等。）
*   不得在 `runOnOperation` 的多次调用之间维护可变的 pass 状态。一个 pass 可能在许多不同的操作上运行，不保证执行顺序。
    *   在多线程时，特定的 pass 实例甚至可能不会在 IR 中的所有操作上执行。因此，pass 不应依赖于在所有操作上运行。
*   不得维护任何全局可变状态，例如源文件中的静态变量。所有可变状态应由 pass 的实例维护。
*   必须可以拷贝构造
    *   pass 管理器可能会创建 pass 的多个实例以并行处理操作。

### 操作无关（Op-Agnostic）的操作 Pass

默认情况下，操作 pass 是"操作无关"的，这意味着它在被添加到的 pass 管理器的操作类型上运行。这意味着一个 pass 可能在许多不同类型的操作上运行。无关 pass 的编写方式应使其不对运行的操作做任何假设。此类 pass 的示例有[规范化（Canonicalization）](Passes.md/#-canonicalize)和[公共子表达式消除（Common Sub-Expression Elimination）](Passes.md/#-cse)。

要创建一个无关操作 pass，派生类必须遵守以下规则：

*   继承自 CRTP 类 `OperationPass`。
*   重写虚方法 `void runOnOperation()`。

一个简单的 pass 示例如下：

```c++
/// Here we utilize the CRTP `PassWrapper` utility class to provide some
/// necessary utility hooks. This is only necessary for passes defined directly
/// in C++. Passes defined declaratively use a cleaner mechanism for providing
/// these utilities.
struct MyOperationPass : public PassWrapper<MyOperationPass, OperationPass<>> {
  void runOnOperation() override {
    // Get the current operation being operated on.
    Operation *op = getOperation();
    ...
  }
};
```

### 过滤操作 Pass

如果一个 pass 需要将其执行限制到特定类型或类别的操作，可以在此基础上额外进行过滤。这将原本"无关"的 pass 转变为更特定于某个上下文的 pass。过滤 pass 执行的方式有多种，过滤可适用于不同的上下文：

### 操作 Pass：静态调度过滤

静态过滤允许对 pass 可调度到的操作类型施加额外约束。这种过滤通常允许构建更受约束的 pass，这些 pass 只能被调度到满足必要约束的操作上。例如，这允许指定只在某一类操作上运行的 pass，即那些提供特定接口（interface）、特征（trait）或其他适用于该操作类型所有实例的约束的操作。以下是一个只允许调度到实现了 `FunctionOpInterface` 的操作上的 pass 示例：

```c++
struct MyFunctionPass : ... {
  /// This method is used to provide additional static filtering, and returns if the
  /// pass may be scheduled on the given operation type.
  bool canScheduleOn(RegisteredOperationName opInfo) const override {
    return opInfo.hasInterface<FunctionOpInterface>();
  }

  void runOnOperation() {
    // Here we can freely cast to FunctionOpInterface, because our `canScheduleOn` ensures
    // that our pass is only executed on operations implementing that interface.
    FunctionOpInterface op = cast<FunctionOpInterface>(getOperation()); 
  }
};
```

当具有静态过滤的 pass 被添加到[`op-specific` pass 管理器](#oppassmanager)时，它会断言该 pass 管理器的操作类型满足 pass 的静态约束。当添加到[`op-agnostic` pass 管理器](#oppassmanager)时，该 pass 管理器及其中包含的所有 pass 都继承该 pass 的静态约束。例如，如果 pass 过滤 `FunctionOpInterface`（如上面的 `MyFunctionPass` 示例），则在执行 pass 管理器中的**任何** pass 时，只会考虑实现了 `FunctionOpInterface` 的操作。这个不变量很重要，因为添加到 `op-agnostic` pass 管理器的每个 pass 都会进一步约束可调度到其上的操作。考虑以下示例：

```mlir
func.func @foo() {
  // ...
  return
}

module @someModule {
  // ...
}
```

如果我们将 op-agnostic 流水线 `any(cse,my-function-pass)` 应用到上面的 MLIR 片段，它只会在 `foo` 函数操作上运行。这是因为 `my-function-pass` 有一个静态过滤约束，只能调度到实现了 `FunctionOpInterface` 的操作上。请记住，这个约束被整个 pass 管理器继承，所以我们从不考虑将 `someModule` 用于任何 pass，包括通常可以调度到任何操作上的 `cse`。

#### 操作 Pass：按操作类型进行静态过滤

在上一节中，我们详细介绍了一种通用机制，用于静态过滤 pass 可调度到的操作类型。在此机制之上提供了语法糖，以简化限制为在单一操作类型上调度的 pass 的定义。在这些情况下，pass 只需向 `OperationPass` 基类提供操作类型即可。这将自动在该操作类型上建立过滤：

```c++
/// Here we utilize the CRTP `PassWrapper` utility class to provide some
/// necessary utility hooks. This is only necessary for passes defined directly
/// in C++. Passes defined declaratively use a cleaner mechanism for providing
/// these utilities.
struct MyFunctionPass : public PassWrapper<MyOperationPass, OperationPass<func::FuncOp>> {
  void runOnOperation() {
    // Get the current operation being operated on.
    func::FuncOp op = getOperation();
  }
};
```

#### 操作 Pass：按接口进行静态过滤

在上一节中，我们详细介绍了一种通用机制，用于静态过滤 pass 可调度到的操作类型。在此机制之上提供了语法糖，以简化限制为在特定操作接口上调度的 pass 的定义。在这些情况下，pass 只需继承 `InterfacePass` 基类即可。该类与 `OperationPass` 类似，但需要指定要操作的接口类型。这将自动在该接口类型上建立过滤：

```c++
/// Here we utilize the CRTP `PassWrapper` utility class to provide some
/// necessary utility hooks. This is only necessary for passes defined directly
/// in C++. Passes defined declaratively use a cleaner mechanism for providing
/// these utilities.
struct MyFunctionPass : public PassWrapper<MyOperationPass, InterfacePass<FunctionOpInterface>> {
  void runOnOperation() {
    // Get the current operation being operated on.
    FunctionOpInterface op = getOperation();
  }
};
```

### 依赖方言（Dependent Dialects）

在 MLIRContext 中，必须在加载方言（dialect）之后，才能创建这些方言中的实体（操作、类型、属性等）。在启动多线程 pass 流水线之前，也必须先加载方言。为此，可能创建尚未保证加载的方言中实体的 pass 必须通过重写 `getDependentDialects()` 方法来明确声明此方言列表。
另请参阅 [TableGen 规范](#tablegen-specification)中的 `dependentDialects` 字段。

### 初始化

在某些情况下，Pass 可能包含动态构造的状态，但在 Pass 的连续运行中重新计算可能代价高昂。一个这样的例子是使用基于 [`PDL`](Dialects/PDLOps.md) 的[模式](PatternRewriter.md)，这些模式在运行时被编译为字节码。在这些情况下，pass 可以重写以下钩子来初始化这些重量级状态：

*   `LogicalResult initialize(MLIRContext *context)`

此钩子在完整 pass 流水线每次运行时执行一次，这意味着它无法访问 `runOnOperation` 调用期间可用的状态。更具体地说，所有必要的对 `MLIRContext` 的访问应通过提供的 `context` 参数驱动，而使用"每次运行"状态的方法（如 `getContext`/`getOperation`/`getAnalysis` 等）不得使用。如果初始化期间发生错误，pass 应该发出错误诊断并返回 `failure()`，这将中止 pass 流水线的执行。

## 分析管理

分析（analysis）是与变换 pass 并列的重要概念。它们在概念上与变换 pass 类似，不同之处在于它们在特定操作上计算信息而不修改它。在 MLIR 中，分析不是 pass，而是独立的类，按需惰性计算并缓存以避免不必要的重新计算。MLIR 中的分析必须遵守以下规则：

*   提供一个有效的构造函数，接受 `Operation*` 或 `Operation*` 和 `AnalysisManager &`。
    *   提供的 `AnalysisManager &` 应用于查询任何必要的分析依赖项。
*   不得修改给定的操作。

分析可以提供额外的钩子来控制各种行为：

*   `bool isInvalidated(const AnalysisManager::PreservedAnalyses &)`

给定一个已保留的分析集，如果分析应该真正被失效，则返回 true。这允许在分析未被明确标记为已保留但可能基于其他属性（如分析集）被保留（或失效）的情况下进行更精细的失效。如果分析将任何其他分析作为依赖项使用，它还必须检查依赖项是否已被失效。

### 查询分析

`OperationPass` 基类提供了用于查询和保留当前处理操作的分析的实用工具。

*   OperationPass 自动提供以下用于查询分析的实用工具：
    *   `getAnalysis<>`
        -   获取当前操作的分析，如有必要则构造它。
    *   `getCachedAnalysis<>`
        -   获取当前操作的分析（如果已存在）。
    *   `getCachedParentAnalysis<>`
        -   获取给定父操作的分析（如果存在）。
    *   `getCachedChildAnalysis<>`
        -   获取给定子操作的分析（如果存在）。
    *   `getChildAnalysis<>`
        -   获取给定子操作的分析，如有必要则构造它。

使用上面定义的示例 pass，让我们看一些例子：

```c++
/// An interesting analysis.
struct MyOperationAnalysis {
  // Compute this analysis with the provided operation.
  MyOperationAnalysis(Operation *op);
};

struct MyOperationAnalysisWithDependency {
  MyOperationAnalysisWithDependency(Operation *op, AnalysisManager &am) {
    // Request other analysis as dependency
    MyOperationAnalysis &otherAnalysis = am.getAnalysis<MyOperationAnalysis>();
    ...
  }

  bool isInvalidated(const AnalysisManager::PreservedAnalyses &pa) {
    // Check if analysis or its dependency were invalidated
    return !pa.isPreserved<MyOperationAnalysisWithDependency>() ||
           !pa.isPreserved<MyOperationAnalysis>();
  }
};

void MyOperationPass::runOnOperation() {
  // Query MyOperationAnalysis for the current operation.
  MyOperationAnalysis &myAnalysis = getAnalysis<MyOperationAnalysis>();

  // Query a cached instance of MyOperationAnalysis for the current operation.
  // It will not be computed if it doesn't exist.
  auto optionalAnalysis = getCachedAnalysis<MyOperationAnalysis>();
  if (optionalAnalysis)
    ...

  // Query a cached instance of MyOperationAnalysis for the parent operation of
  // the current operation. It will not be computed if it doesn't exist.
  auto optionalAnalysis = getCachedParentAnalysis<MyOperationAnalysis>();
  if (optionalAnalysis)
    ...
}
```

### 保留分析

在被 pass 查询后构造的分析会被缓存，以避免在后续再次请求时进行不必要的计算。为了避免使用过时的分析，所有分析都被假设为被 pass 失效。为了避免失效，pass 必须明确标记已知被保留的分析。

*   所有 Pass 类自动提供以下用于保留分析的实用工具：
    *   `markAllAnalysesPreserved`
    *   `markAnalysesPreserved<>`

```c++
void MyOperationPass::runOnOperation() {
  // Mark all analyses as preserved. This is useful if a pass can guarantee
  // that no transformation was performed.
  markAllAnalysesPreserved();

  // Mark specific analyses as preserved. This is used if some transformation
  // was performed, but some analyses were either unaffected or explicitly
  // preserved.
  markAnalysesPreserved<MyAnalysis, MyAnalyses...>();
}
```

## Pass 失败

MLIR 中的 pass 允许优雅地失败。如果 pass 的某些不变量被打破，可能会使 IR 处于某种无效状态，就可能发生这种情况。如果出现这种情况，pass 可以通过 `signalPassFailure` 方法直接向 pass 管理器发出失败信号。如果 pass 在执行时发出了失败信号，流水线中的其他 pass 将不会执行，并且对 `PassManager::run` 的顶层调用将返回 `failure`。

```c++
void MyOperationPass::runOnOperation() {
  // Signal failure on a broken invariant.
  if (some_broken_invariant)
    return signalPassFailure();
}
```

## Pass 管理器

上面几节介绍了不同类型的 pass 及其不变量。本节介绍 PassManager 的概念，以及如何使用它来配置和调度 pass 流水线。与 pass 管理相关的主要有两个类：`PassManager` 和 `OpPassManager`。`PassManager` 类充当顶层入口点，包含用于整个 pass 流水线的各种配置。`OpPassManager` 类用于将 pass 调度在特定嵌套层级运行。顶层 `PassManager` 也充当一个 `OpPassManager`。

### OpPassManager

`OpPassManager` 本质上是锚定到在给定嵌套层级的操作上执行的 pass 集合。pass 管理器可以是"操作特定"的（锚定在特定操作类型上），也可以是"操作无关"的（不限于任何特定操作，在任何可行的操作类型上执行）。锚定 pass 管理器的操作类型必须遵守以下要求：

*   必须已注册并标记为 [`IsolatedFromAbove`](Traits/#isolatedfromabove)。

    *   Pass 预期不修改当前正在处理的操作或以上级别的操作。如果操作不是隔离的，它可能会无意中修改或遍历它不应该访问的操作的 SSA 使用链表。

可以通过 `addPass` 将 pass 添加到 pass 管理器。

`OpPassManager` 通常通过 `nest<OpT>` 或 `nestAny` 方法在另一个已存在的 `OpPassManager` 中显式嵌套流水线来创建。前一个方法接受嵌套 pass 管理器将操作的操作类型。后一个方法嵌套一个"操作无关"的 pass 管理器，该管理器可以在任何可行的操作类型上运行。这里的嵌套对应于 IR 的[区域（Region）](LangRef.md/#regions)内的[结构性](Tutorials/UnderstandingTheIRStructure.md)嵌套。

例如，以下 `.mlir` 文件：

```mlir
module {
  spirv.module "Logical" "GLSL450" {
    func @foo() {
      ...
    }
  }
}
```

具有如下嵌套结构：

```
`builtin.module`
  `spirv.module`
    `spirv.func`
```

以下是构建在上述结构上操作的流水线示例：

```c++
// Create a top-level `PassManager` class.
auto pm = PassManager::on<ModuleOp>(ctx);

// Add a pass on the top-level module operation.
pm.addPass(std::make_unique<MyModulePass>());

// Nest a pass manager that operates on `spirv.module` operations nested
// directly under the top-level module.
OpPassManager &nestedModulePM = pm.nest<spirv::ModuleOp>();
nestedModulePM.addPass(std::make_unique<MySPIRVModulePass>());

// Nest a pass manager that operates on functions within the nested SPIRV
// module.
OpPassManager &nestedFunctionPM = nestedModulePM.nest<func::FuncOp>();
nestedFunctionPM.addPass(std::make_unique<MyFunctionPass>());

// Nest an op-agnostic pass manager. This will operate on any viable
// operation, e.g. func.func, spirv.func, spirv.module, builtin.module, etc.
OpPassManager &nestedAnyPM = nestedModulePM.nestAny();
nestedAnyPM.addPass(createCanonicalizePass());
nestedAnyPM.addPass(createCSEPass());

// Run the pass manager on the top-level module.
ModuleOp m = ...;
if (failed(pm.run(m)))
    ... // One of the passes signaled a failure.
```

上述 pass 管理器包含以下流水线结构：

```
OpPassManager<ModuleOp>
  MyModulePass
  OpPassManager<spirv::ModuleOp>
    MySPIRVModulePass
    OpPassManager<func::FuncOp>
      MyFunctionPass
    OpPassManager<>
      Canonicalizer
      CSE
```

这些流水线然后一次在一个操作上运行。这意味着，例如，对于 `func::FuncOp` 上的一系列连续 pass，它将先在第一个函数上全部执行，然后在第二个函数上全部执行，依此类推，直到整个程序通过所有 pass。这提供了几个好处：

*   这改善了编译器的缓存行为，因为它每次只处理一个函数，而不是遍历整个程序。
*   这通过减少需要调度的作业数量并提高每个作业的效率来改善多线程性能。整个函数流水线可以在每个函数上异步运行。

## 动态 Pass 流水线

在某些情况下，在另一个 pass 内运行 pass 流水线可能很有用，以便根据当前正在操作的操作的某些不变量进行配置或过滤。例如，[内联 Pass（Inliner Pass）](Passes.md/#-inline) 可能希望在内联时运行过程内简化 pass，以产生更好的代价模型并提供更优化的内联。为了实现这一点，pass 可以通过 `LogicalResult Pass::runPipeline(OpPassManager &, Operation *)` 方法在当前正在操作的操作或当前操作中嵌套的任何操作上运行任意 `OpPassManager`。此方法返回动态流水线是否成功或失败，类似于顶层 `PassManager::run` 方法的结果。下面是一个简单示例：

```c++
void MyModulePass::runOnOperation() {
  ModuleOp module = getOperation();
  if (hasSomeSpecificProperty(module)) {
    OpPassManager dynamicPM("builtin.module");
    ...; // Build the dynamic pipeline.
    if (failed(runPipeline(dynamicPM, module)))
      return signalPassFailure();
  }
}
```

注意：虽然上面动态流水线是在 `runOnOperation` 方法中构造的，但这不是必需的，流水线应该尽可能缓存，因为 `OpPassManager` 类可以安全地拷贝构造。

本节描述的机制应在 pass 流水线需要以嵌套方式运行时使用，即嵌套流水线不能与主 pass 流水线的其余部分一起静态调度时。更具体地说，一般不需要在 `Pass` 内构造 `PassManager`。使用 `runPipeline` 还确保所有分析、[插桩](#pass-instrumentation)以及其他 pass 管理器相关组件都与正在执行的动态流水线集成。

## 实例特定的 Pass 选项

MLIR 为 pass 提供了一种内置机制来指定配置其行为的选项。这些选项在 pass 构造时为每个 pass 实例独立解析。选项使用 `Option<>` 和 `ListOption<>` 类定义，通常遵循 [LLVM 命令行](https://llvm.org/docs/CommandLine.html)标志定义规则。与 LLVM 命令行功能的一个主要区别是所有 `ListOption` 都是逗号分隔的，列表各个元素中的分隔子范围可以包含不被视为顶层列表分隔符的逗号。

```c++
struct MyPass ... {
  /// Make sure that we have a valid default constructor and copy constructor to
  /// ensure that the options are initialized properly.
  MyPass() = default;
  MyPass(const MyPass& pass) {}

  /// Any parameters after the description are forwarded to llvm::cl::list and
  /// llvm::cl::opt respectively.
  Option<int> exampleOption{*this, "flag-name", llvm::cl::desc("...")};
  ListOption<int> exampleListOption{*this, "list-flag-name", llvm::cl::desc("...")};
};
```

对于 pass 流水线，`PassPipelineRegistration` 模板为可选的 `Option` 结构体定义接受一个额外的模板参数。此结构体应继承自 `mlir::PassPipelineOptions` 并包含所需的流水线选项。使用 `PassPipelineRegistration` 时，构造函数现在接受一个签名为 `void (OpPassManager &pm, const MyPipelineOptions&)` 的函数，该函数应从选项构造 pass 并将其传递给 pm：

```c++
struct MyPipelineOptions : public PassPipelineOptions {
  // The structure of these options is the same as those for pass options.
  Option<int> exampleOption{*this, "flag-name", llvm::cl::desc("...")};
  ListOption<int> exampleListOption{*this, "list-flag-name",
                                    llvm::cl::desc("...")};
};

void registerMyPasses() {
  PassPipelineRegistration<MyPipelineOptions>(
    "example-pipeline", "Run an example pipeline.",
    [](OpPassManager &pm, const MyPipelineOptions &pipelineOptions) {
      // Initialize the pass manager.
    });
}
```

## Pass 统计

统计是跟踪编译器正在做什么以及各种变换效果的一种方式。查看特定变换对特定输入的影响以及它们触发的频率通常很有用。Pass 统计对每个 pass 实例是特定的，这允许查看在 pass 流水线中特定位置放置某个变换的效果。例如，它们有助于回答"如果我在这里再次运行 CSE 会发生什么？"之类的问题。

可以使用 `Pass::Statistic` 类向 pass 添加统计信息。该类以父 pass、名称和描述作为构造函数参数。该类的行为类似于原子无符号整数，可以相应地递增和更新。这些统计依赖于与 [`llvm::Statistic`](http://llvm.org/docs/ProgrammersManual.html#the-statistic-class-stats-option) 相同的基础设施，因此具有类似的使用约束。收集的统计可以通过 [pass 管理器](#pass-manager)以编程方式通过 `PassManager::enableStatistics` 转储；或通过命令行上的 `-mlir-pass-statistics` 和 `-mlir-pass-statistics-display` 转储。

示例如下：

```c++
struct MyPass ... {
  /// Make sure that we have a valid default constructor and copy constructor to
  /// ensure that the options are initialized properly.
  MyPass() = default;
  MyPass(const MyPass& pass) {}
  StringRef getArgument() const final {
    // This is the argument used to refer to the pass in
    // the textual format (on the commandline for example).
    return "argument";
  }
  StringRef getDescription() const final {
    // This is a brief description of the pass.
    return  "description";
  }
  /// Define the statistic to track during the execution of MyPass.
  Statistic exampleStat{this, "exampleStat", "An example statistic"};

  void runOnOperation() {
    ...

    // Update the statistic after some invariant was hit.
    ++exampleStat;

    ...
  }
};
```

收集的统计可以以两种视图方式汇总：

一种是流水线视图，模拟 pass 管理器的结构，这是默认视图：

```shell
$ mlir-opt -pass-pipeline='any(func.func(my-pass,my-pass))' foo.mlir -mlir-pass-statistics

===-------------------------------------------------------------------------===
                         ... Pass statistics report ...
===-------------------------------------------------------------------------===
'func.func' Pipeline
  MyPass
    (S) 15 exampleStat - An example statistic
  VerifierPass
  MyPass
    (S)  6 exampleStat - An example statistic
  VerifierPass
VerifierPass
```

另一种是列表视图，将特定 pass 的所有实例的统计信息汇总在一起：

```shell
$ mlir-opt -pass-pipeline='any(func.func(my-pass,my-pass))' foo.mlir -mlir-pass-statistics -mlir-pass-statistics-display=list

===-------------------------------------------------------------------------===
                         ... Pass statistics report ...
===-------------------------------------------------------------------------===
MyPass
  (S) 21 exampleStat - An example statistic
```

## Pass 注册

在各种 pass 类型的示例定义中简要展示的是 `PassRegistration` 类。该机制允许注册 pass 类，以便可以在[文本 pass 流水线描述](#textual-pass-pipeline-specification)中创建它们。注册示例如下：

```c++
void registerMyPass() {
  PassRegistration<MyPass>();
}
```

*   `MyPass` 是派生 pass 类的名称。
*   Pass 的 `getArgument()` 方法用于获取将用于引用该 pass 的标识符。
*   Pass 的 `getDescription()` 方法提供对该 pass 的简短描述。

对于无法默认构造的 pass，`PassRegistration` 接受一个可选参数，该参数接受创建 pass 的回调：

```c++
void registerMyPass() {
  PassRegistration<MyParametricPass>(
    []() -> std::unique_ptr<Pass> {
      std::unique_ptr<Pass> p = std::make_unique<MyParametricPass>(/*options*/);
      /*... non-trivial-logic to configure the pass ...*/;
      return p;
    });
}
```

例如，可以使用此注册变体从命令行参数接受 pass 的配置并将其传递给 pass 构造函数。

注意：确保 pass 可以以不共享数据的方式拷贝构造，因为 [pass 管理器](#pass-manager)可能会创建 pass 的副本以并行运行。

### Pass 流水线注册

上面描述的是用于注册特定派生 pass 类的机制。在此基础上，MLIR 允许以类似方式注册自定义 pass 流水线。这允许自定义流水线以与 pass 相同的方式供 mlir-opt 等工具使用，这对于封装常见流水线（如"-O1"系列 pass）很有用。流水线通过类似于 pass 的机制以 `PassPipelineRegistration` 的形式注册。与 `PassRegistration` 相比，该类接受一个额外参数，即修改提供的 `OpPassManager` 的流水线构建器。

```c++
void pipelineBuilder(OpPassManager &pm) {
  pm.addPass(std::make_unique<MyPass>());
  pm.addPass(std::make_unique<MyOtherPass>());
}

void registerMyPasses() {
  // Register an existing pipeline builder function.
  PassPipelineRegistration<>(
    "argument", "description", pipelineBuilder);

  // Register an inline pipeline builder.
  PassPipelineRegistration<>(
    "argument", "description", [](OpPassManager &pm) {
      pm.addPass(std::make_unique<MyPass>());
      pm.addPass(std::make_unique<MyOtherPass>());
    });
}
```

### 文本 Pass 流水线规范

前几节详细介绍了如何使用特定参数和描述注册 pass 和 pass 流水线。一旦注册，这些可以用于从字符串描述配置 pass 管理器。这对于 `mlir-opt` 等工具特别有用，它们从命令行配置 pass 管理器，或作为使用[动态 pass 流水线](#dynamic-pass-pipelines)的 pass 的选项。

为了支持描述完整 pass 流水线结构的能力，MLIR 支持 pass 流水线的自定义文本描述。文本描述包括嵌套结构、要运行的 pass 和 pass 流水线的参数，以及这些 pass 和流水线的任何选项。文本流水线被定义为一系列名称，每个名称本身可以递归地包含嵌套的流水线描述。此规范的语法如下：

```ebnf
pipeline          ::= op-anchor `(` pipeline-element (`,` pipeline-element)* `)`
pipeline-element  ::= pipeline | (pass-name | pass-pipeline-name) options?
options           ::= '{' (key ('=' value)?)+ '}'
```

*   `op-anchor`
    *   这对应于锚定 pass 管理器执行的助记符名称。这可以是要在其上运行 pass 的操作的名称，例如 `func.func` 或 `builtin.module`，或者 `any`，用于在任何可行操作上执行的操作无关 pass 管理器（即任何可用于锚定 pass 管理器的操作）。
*   `pass-name` | `pass-pipeline-name`
    *   这对应于已注册 pass 或 pass 流水线的参数，例如 `cse` 或 `canonicalize`。
*   `options`
    *   选项是表示由 pass 或 pass 流水线定义的选项的特定键值对，如["实例特定的 Pass 选项"](#instance-specific-pass-options)部分所述。请参阅该部分了解文本流水线中的示例用法。

例如，以下流水线：

```shell
$ mlir-opt foo.mlir -cse -canonicalize -convert-func-to-llvm='use-bare-ptr-memref-call-conv=1'
```

也可以指定为（通过 `-pass-pipeline` 标志）：

```shell
# Anchor the cse and canonicalize passes on the `func.func` operation.
$ mlir-opt foo.mlir -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm{use-bare-ptr-memref-call-conv=1})'

# Anchor the cse and canonicalize passes on "any" viable root operation.
$ mlir-opt foo.mlir -pass-pipeline='builtin.module(any(cse,canonicalize),convert-func-to-llvm{use-bare-ptr-memref-call-conv=1})'
```

为了支持使用 `OpPassManager::printAsTextualPipeline(raw_ostream&)` 将 pass 往返到文本表示，请重写 `StringRef Pass::getArgument()` 以指定注册 pass 时使用的参数。

## 声明式 Pass 规范

Pass 的某些方面可以以类似于[操作](DefiningDialects/Operations.md)的形式声明式地指定。此规范简化了定义 pass 时使用的几种机制。它可用于生成 pass 注册调用、定义样板 pass 实用工具以及生成 pass 文档。

考虑以下 C++ 中指定的 pass：

```c++
struct MyPass : PassWrapper<MyPass, OperationPass<ModuleOp>> {
  MyPass() = default;
  MyPass(const MyPass &) {}

  ...

  // Specify any options.
  Option<bool> option{
      *this, "example-option",
      llvm::cl::desc("An example option"), llvm::cl::init(true)};
  ListOption<int64_t> listOption{
      *this, "example-list",
      llvm::cl::desc("An example list option")};

  // Specify any statistics.
  Statistic statistic{this, "example-statistic", "An example statistic"};
};

/// Expose this pass to the outside world.
std::unique_ptr<Pass> foo::createMyPass() {
  return std::make_unique<MyPass>();
}

/// Register this pass.
void foo::registerMyPass() {
  PassRegistration<MyPass>();
}
```

此 pass 可以声明式地指定为：

```tablegen
def MyPass : Pass<"my-pass", "ModuleOp"> {
  let summary = "My Pass Summary";
  let description = [{
    Here we can now give a much larger description of `MyPass`, including all of
    its various constraints and behavior.
  }];

  // Specify any options.
  let options = [
    Option<"option", "example-option", "bool", /*default=*/"true",
           "An example option">,
    ListOption<"listOption", "example-list", "int64_t",
               "An example list option">
  ];

  // Specify any statistics.
  let statistics = [
    Statistic<"statistic", "example-statistic", "An example statistic">
  ];
}
```

使用 `gen-pass-decls` 生成器，我们可以自动生成上面的大部分样板代码。此生成器接受一个 `-name` 参数作为输入，该参数为正在生成的 pass 组提供标签。此生成器生成具有多种用途的代码：

首先是向全局注册表注册声明的 pass。对于每个 pass，生成器生成一个 `registerPassName`，其中 `PassName` 是 tablegen 中指定的定义名称。它还生成一个 `registerGroupPasses`，其中 `Group` 是通过 `-name` 输入参数提供的标签，用于注册所有存在的 pass。

可以通过定义 `GEN_PASS_REGISTRATION` 宏为整个 pass 组启用这些声明，或者通过定义 `GEN_PASS_REGISTRATION_PASSNAME`（其中 `PASSNAME` 是 pass 名称的大写版本，类似于 pass def 和 decls）逐个 pass 启用。

```c++
// Tablegen options: -gen-pass-decls -name="Example"

// Passes.h

namespace foo {
#define GEN_PASS_REGISTRATION
#include "Passes.h.inc"
} // namespace foo

void registerMyPasses() {
  // Register all of the passes.
  foo::registerExamplePasses();
  
  // Or

  // Register `MyPass` specifically.
  foo::registerMyPass();
}
```

第二是提供一种配置 pass 选项的方式。这些类以 `MyPassOptions` 的形式命名，其中 `MyPass` 是 tablegen 中 pass 定义的名称。可配置参数反映了 tablegen 文件中声明的选项。可以通过定义 `GEN_PASS_DECL` 宏为整个 pass 组启用这些声明，或者通过定义 `GEN_PASS_DECL_PASSNAME`（其中 `PASSNAME` 是 tablegen 中指定名称的大写版本）逐个 pass 启用。

```c++
// .h.inc

#ifdef GEN_PASS_DECL_MYPASS

struct MyPassOptions {
    bool option = true;
    ::llvm::ArrayRef<int64_t> listOption;
};

#undef GEN_PASS_DECL_MYPASS
#endif // GEN_PASS_DECL_MYPASS
```

自动生成的文件还将包含默认构造函数的声明。

```c++
// .h.inc

#ifdef GEN_PASS_DECL_MYPASS
...

std::unique_ptr<::mlir::Pass> createMyPass();
std::unique_ptr<::mlir::Pass> createMyPass(const MyPassOptions &options);

#undef GEN_PASS_DECL_MYPASS
#endif // GEN_PASS_DECL_MYPASS
```

此生成器的最后一个目的是为每个 pass 发出一个基类，其中包含与 pass 定义相关的大部分样板代码。这些类以 `MyPassBase` 的形式命名并在 `impl` 命名空间内声明，其中 `MyPass` 是 tablegen 中 pass 定义的名称。我们可以按如下方式更新原始 C++ pass 定义：

```c++
// MyPass.cpp

/// Include the generated base pass class definitions.
namespace foo {
#define GEN_PASS_DEF_MYPASS
#include "Passes.h.inc"
}

/// Define the main class as deriving from the generated base class.
struct MyPass : foo::impl::MyPassBase<MyPass> {
  using MyPassBase::MyPassBase;

  /// The definitions of the options and statistics are now generated within
  /// the base class, but are accessible in the same way.
};
```

可以通过定义适当的预处理器 `GEN_PASS_DEF_PASSNAME` 宏（`PASSNAME` 等于 tablegen 中 pass 定义名称的大写版本）逐个 pass 启用这些定义。默认构造函数也被定义，并期望实际 pass 类的名称等于 tablegen 中定义的名称。

使用 `gen-pass-doc` 生成器，可以为每个 pass 生成 Markdown 文档。请参阅 [Passes.md](Passes.md) 查看真实 MLIR pass 的示例输出。

### TableGen 规范

`Pass` 类用于开始新的 pass 定义。该类以要属性化到 pass 的注册参数以及对应于 pass 操作的操作类型的可选字符串作为参数。该类包含以下字段：

*   `summary`
    -   pass 的简短单行摘要，用作注册 pass 时的描述。
*   `description`
    -   pass 的更长、更详细的描述。用于生成 pass 文档。
*   `dependentDialects`
    -   表示此 pass 可能引入实体（属性/操作/类型等）的 `Dialect` 类的字符串列表。
*   `options`
    -   pass 使用的 pass 选项列表。
*   `statistics`
    -   pass 使用的 pass 统计列表。
*   `constructor`
    -   用于创建 pass 默认实例的代码块。指定它将禁用 pass 的构造函数自动生成。这是一个遗留选项，不建议使用。

#### 选项

选项可以通过 `Option` 和 `ListOption` 类指定。`Option` 类接受以下模板参数：

*   C++ 变量名
    -   用于生成的选项变量的名称。
*   参数（argument）
    -   选项的参数名称。
*   类型（type）
    -   选项的 C++ 类型。
*   默认值（default value）
    -   默认选项值。
*   描述（description）
    -   选项的单行描述。
*   附加选项标志（additional option flags）
    -   包含构造选项所需的任何附加选项的字符串。

```tablegen
def MyPass : Pass<"my-pass"> {
  let options = [
    Option<"option", "example-option", "bool", /*default=*/"true",
           "An example option">,
  ];
}
```

`ListOption` 类接受以下字段：

*   C++ 变量名
    -   用于生成的选项变量的名称。
*   参数（argument）
    -   选项的参数名称。
*   元素类型（element type）
    -   列表元素的 C++ 类型。
*   描述（description）
    -   选项的单行描述。
*   附加选项标志（additional option flags）
    -   包含构造选项所需的任何附加选项的字符串。

```tablegen
def MyPass : Pass<"my-pass"> {
  let options = [
    ListOption<"listOption", "example-list", "int64_t",
               "An example list option">
  ];
}
```

#### 统计

统计可以通过 `Statistic` 指定，它接受以下模板参数：

*   C++ 变量名
    -   用于生成的统计变量的名称。
*   显示名称（display name）
    -   显示统计时使用的名称。
*   描述（description）
    -   统计的单行描述。

```tablegen
def MyPass : Pass<"my-pass"> {
  let statistics = [
    Statistic<"statistic", "example-statistic", "An example statistic">
  ];
}
```

## Pass 插桩

MLIR 通过 `PassInstrumentation` 类提供了一个可定制的框架，用于插桩 pass 执行和分析计算。该类提供钩子进入 PassManager，以观察各种事件：

*   `runBeforePipeline`
    *   此回调在 pass 流水线（即 pass 管理器）执行之前运行。
*   `runAfterPipeline`
    *   此回调在 pass 流水线成功或失败执行后立即运行。
*   `runBeforePass`
    *   此回调在 pass 执行之前运行。
*   `runAfterPass`
    *   此回调在 pass 成功执行后立即运行。如果此钩子被执行，`runAfterPassFailed` 将*不会*被执行。
*   `runAfterPassFailed`
    *   此回调在 pass 执行失败后立即运行。如果此钩子被执行，`runAfterPass` 将*不会*被执行。
*   `runBeforeAnalysis`
    *   此回调在计算分析之前运行。
    *   如果分析请求另一个分析作为依赖项，则依赖项的 `runBeforeAnalysis`/`runAfterAnalysis` 对可以从当前 `runBeforeAnalysis`/`runAfterAnalysis` 对内部调用。
*   `runAfterAnalysis`
    *   此回调在分析计算后立即运行。

PassInstrumentation 实例可以通过 `addInstrumentation` 方法直接向 [PassManager](#pass-manager) 实例注册。添加到 PassManager 的插桩以栈式方式运行，即最后执行 `runBefore*` 钩子的插桩将第一个执行相应的 `runAfter*` 钩子。`PassInstrumentation` 类的钩子保证以线程安全的方式执行，因此不需要额外的同步。以下是一个插桩示例，计算 `DominanceInfo` 分析被计算的次数：

```c++
struct DominanceCounterInstrumentation : public PassInstrumentation {
  /// The cumulative count of how many times dominance has been calculated.
  unsigned &count;

  DominanceCounterInstrumentation(unsigned &count) : count(count) {}
  void runAfterAnalysis(llvm::StringRef, TypeID id, Operation *) override {
    if (id == TypeID::get<DominanceInfo>())
      ++count;
  }
};

MLIRContext *ctx = ...;
PassManager pm(ctx);

// Add the instrumentation to the pass manager.
unsigned domInfoCount;
pm.addInstrumentation(
    std::make_unique<DominanceCounterInstrumentation>(domInfoCount));

// Run the pass manager on a module operation.
ModuleOp m = ...;
if (failed(pm.run(m)))
    ...

llvm::errs() << "DominanceInfo was computed " << domInfoCount << " times!\n";
```

### 标准插桩

MLIR 利用 pass 插桩框架提供了一些有用的开发者工具和实用工具。这些插桩对 MLIR pass 框架的所有用户都直接可用。

#### Pass 计时

PassTiming 插桩提供关于 pass 执行和分析计算的计时信息。这可以快速了解哪些 pass 执行时间最长，以及 pass 对流水线总执行时间的影响有多大。用户可以通过 `enableTiming` 直接在 PassManager 上启用此插桩。此插桩也可通过 `-mlir-timing` 标志在 mlir-opt 中使用。PassTiming 插桩为计时结果提供了几种不同的显示模式，下面描述了每种模式：

##### 列表显示模式

在此模式下，结果按总时间排序显示在列表中，每个 pass/分析实例汇总为一个唯一结果。此视图对于了解流水线中哪些分析/pass 花费时间最多很有用。此显示模式可通过 `-mlir-timing-display=list` 在 mlir-opt 中使用。

```shell
$ mlir-opt foo.mlir -mlir-disable-threading -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm)' -mlir-timing -mlir-timing-display=list

===-------------------------------------------------------------------------===
                         ... Execution time report ...
===-------------------------------------------------------------------------===
  Total Execution Time: 0.0135 seconds

  ----Wall Time----  ----Name----
    0.0135 (100.0%)  root
    0.0041 ( 30.1%)  Parser
    0.0018 ( 13.3%)  ConvertFuncToLLVMPass
    0.0011 (  8.2%)  Output
    0.0007 (  5.2%)  Pipeline Collection : ['func.func']
    0.0006 (  4.6%)  'func.func' Pipeline
    0.0005 (  3.5%)  Canonicalizer
    0.0001 (  0.9%)  CSE
    0.0001 (  0.5%)  (A) DataLayoutAnalysis
    0.0000 (  0.1%)  (A) DominanceInfo
    0.0058 ( 43.2%)  Rest
    0.0135 (100.0%)  Total
```

结果可以通过 `-mlir-output-format=json` 以 JSON 格式显示。

```shell
$ mlir-opt foo.mlir -mlir-disable-threading -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm)' -mlir-timing -mlir-timing-display=list -mlir-output-format=json

[
{"wall": {"duration":   0.0135, "percentage": 100.0}, "name": "root"},
{"wall": {"duration":   0.0041, "percentage":  30.1}, "name": "Parser"},
{"wall": {"duration":   0.0018, "percentage":  13.3}, "name": "ConvertFuncToLLVMPass"},
{"wall": {"duration":   0.0011, "percentage":   8.2}, "name": "Output"},
{"wall": {"duration":   0.0007, "percentage":   5.2}, "name": "Pipeline Collection : ['func.func']"},
{"wall": {"duration":   0.0006, "percentage":   4.6}, "name": "'func.func' Pipeline"},
{"wall": {"duration":   0.0005, "percentage":   3.5}, "name": "Canonicalizer"},
{"wall": {"duration":   0.0001, "percentage":   0.9}, "name": "CSE"},
{"wall": {"duration":   0.0001, "percentage":   0.5}, "name": "(A) DataLayoutAnalysis"},
{"wall": {"duration":   0.0000, "percentage":   0.1}, "name": "(A) DominanceInfo"},
{"wall": {"duration":   0.0058, "percentage":  43.2}, "name": "Rest"},
{"wall": {"duration":   0.0135, "percentage": 100.0}, "name": "Total"}
]
```

##### 树形显示模式

在此模式下，结果以嵌套流水线视图显示，镜像 pass 管理器中正在执行的内部 pass 流水线。此视图对于了解流水线的哪些部分花费时间最多很有用，也可用于识别何时分析被失效和重新计算。这是默认显示模式。

```shell
$ mlir-opt foo.mlir -mlir-disable-threading -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm)' -mlir-timing

===-------------------------------------------------------------------------===
                         ... Execution time report ...
===-------------------------------------------------------------------------===
  Total Execution Time: 0.0127 seconds

  ----Wall Time----  ----Name----
    0.0038 ( 30.2%)  Parser
    0.0006 (  4.8%)  'func.func' Pipeline
    0.0001 (  0.9%)    CSE
    0.0000 (  0.1%)      (A) DominanceInfo
    0.0005 (  3.7%)    Canonicalizer
    0.0017 ( 13.7%)  ConvertFuncToLLVMPass
    0.0001 (  0.6%)    (A) DataLayoutAnalysis
    0.0010 (  8.2%)  Output
    0.0054 ( 42.5%)  Rest
    0.0127 (100.0%)  Total
```

结果可以通过 `-mlir-output-format=json` 以 JSON 格式显示。

```shell
$ mlir-opt foo.mlir -mlir-disable-threading -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm)' -mlir-timing -mlir-output-format=json

[
{"wall": {"duration":   0.0038, "percentage":  30.2}, "name": "Parser", "passes": [
{}]},
{"wall": {"duration":   0.0006, "percentage":   4.8}, "name": "'func.func' Pipeline", "passes": [
  {"wall": {"duration":   0.0001, "percentage":   0.9}, "name": "CSE", "passes": [
    {"wall": {"duration":   0.0000, "percentage":   0.1}, "name": "(A) DominanceInfo", "passes": [
    {}]},
  {}]},
  {"wall": {"duration":   0.0005, "percentage":   3.7}, "name": "Canonicalizer", "passes": [
  {}]},
{}]},
{"wall": {"duration":   0.0017, "percentage":  13.7}, "name": "ConvertFuncToLLVMPass", "passes": [
  {"wall": {"duration":   0.0001, "percentage":   0.6}, "name": "(A) DataLayoutAnalysis", "passes": [
  {}]},
{}]},
{"wall": {"duration":   0.0010, "percentage":   8.2}, "name": "Output", "passes": [
{}]},
{"wall": {"duration":   0.0054, "percentage":  42.5}, "name": "Rest"},
{"wall": {"duration":   0.0127, "percentage": 100.0}, "name": "Total"}
]
```

##### 多线程 Pass 计时

当 pass 管理器中启用多线程时，显示的含义略有变化。首先，添加了一个新的计时列 `User Time`，显示所有线程上花费的总时间。其次，`Wall Time` 列显示所有线程中花费的最长单个时间。这意味着 `Wall Time` 列将继续给出感知时间（或时钟时间）的指示，而 `User Time` 将显示总 CPU 时间。

```shell
$ mlir-opt foo.mlir -pass-pipeline='builtin.module(func.func(cse,canonicalize),convert-func-to-llvm)'  -mlir-timing

===-------------------------------------------------------------------------===
                      ... Pass execution timing report ...
===-------------------------------------------------------------------------===
  Total Execution Time: 0.0078 seconds

   ---User Time---   ---Wall Time---  --- Name ---
   0.0177 ( 88.5%)     0.0057 ( 71.3%)  'func.func' Pipeline
   0.0044 ( 22.0%)     0.0015 ( 18.9%)    CSE
   0.0029 ( 14.5%)     0.0012 ( 15.2%)      (A) DominanceInfo
   0.0038 ( 18.9%)     0.0015 ( 18.7%)    VerifierPass
   0.0089 ( 44.6%)     0.0025 ( 31.1%)    Canonicalizer
   0.0006 (  3.0%)     0.0002 (  2.6%)    VerifierPass
   0.0004 (  2.2%)     0.0004 (  5.4%)  VerifierPass
   0.0013 (  6.5%)     0.0013 ( 16.3%)  LLVMLoweringPass
   0.0006 (  2.8%)     0.0006 (  7.0%)  VerifierPass
   0.0200 (100.0%)     0.0081 (100.0%)  Total
```

#### IR 打印

调试时，在 pass 流水线的各个阶段转储 IR 通常很有用。这就是 IR 打印插桩的用武之地。此插桩允许通过可选地过滤正在执行的 pass，在 pass 执行前后有条件地打印 IR。此插桩可以通过 `enableIRPrinting` 方法直接添加到 PassManager。`mlir-opt` 提供了一些用于此插桩的有用标志：

*   `mlir-print-ir-before=(comma-separated-pass-list)`
    *   在 pass 列表中提供的每个 pass 之前打印 IR。
*   `mlir-print-ir-before-all`
    *   在流水线中每个 pass 之前打印 IR。

```shell
$ mlir-opt foo.mlir -pass-pipeline='func.func(cse)' -mlir-print-ir-before=cse

*** IR Dump Before CSE ***
func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  %c1_i32_0 = arith.constant 1 : i32
  return %c1_i32, %c1_i32_0 : i32, i32
}
```

*   `mlir-print-ir-after=(comma-separated-pass-list)`
    *   在 pass 列表中提供的每个 pass 之后打印 IR。
*   `mlir-print-ir-after-all`
    *   在流水线中每个 pass 之后打印 IR。

```shell
$ mlir-opt foo.mlir -pass-pipeline='func.func(cse)' -mlir-print-ir-after=cse

*** IR Dump After CSE ***
func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  return %c1_i32, %c1_i32 : i32, i32
}
```

*   `mlir-print-ir-after-change`
    *   只有在 pass 修改了 IR 时才在 pass 之后打印 IR。这有助于减少"无趣"pass 的 IR 转储数量。
    *   注意：通过比较 pass 前后操作的哈希值来检测变更。这会增加额外的运行时间来计算 IR 的哈希值，在某些罕见情况下，根据所用哈希算法的碰撞率，可能会产生误报。
    *   注意：此选项应与上述其他 `mlir-print-ir-after` 选项之一配合使用，因为此选项单独不会启用打印。

```shell
$ mlir-opt foo.mlir -pass-pipeline='func.func(cse,cse)' -mlir-print-ir-after=cse -mlir-print-ir-after-change

*** IR Dump After CSE ***
func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  return %c1_i32, %c1_i32 : i32, i32
}
```

*   `mlir-print-ir-after-failure`
    *   只在 pass 失败后打印 IR。
    *   此选项*不应*与上面其他 `mlir-print-ir-after` 标志一起使用。

```shell
$ mlir-opt foo.mlir -pass-pipeline='func.func(cse,bad-pass)' -mlir-print-ir-after-failure

*** IR Dump After BadPass Failed ***
func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  return %c1_i32, %c1_i32 : i32, i32
}
```

*   `mlir-print-ir-module-scope`
    *   始终打印顶层模块操作，无论 pass 类型或操作嵌套级别如何。
    *   注意：在模块范围内打印只应在禁用多线程时使用（`-mlir-disable-threading`）

```shell
$ mlir-opt foo.mlir -mlir-disable-threading -pass-pipeline='func.func(cse)' -mlir-print-ir-after=cse -mlir-print-ir-module-scope

*** IR Dump After CSE ***  ('func.func' operation: @bar)
func.func @bar(%arg0: f32, %arg1: f32) -> f32 {
  ...
}

func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  %c1_i32_0 = arith.constant 1 : i32
  return %c1_i32, %c1_i32_0 : i32, i32
}

*** IR Dump After CSE ***  ('func.func' operation: @simple_constant)
func.func @bar(%arg0: f32, %arg1: f32) -> f32 {
  ...
}

func.func @simple_constant() -> (i32, i32) {
  %c1_i32 = arith.constant 1 : i32
  return %c1_i32, %c1_i32 : i32, i32
}
```

*   `mlir-print-ir-tree-dir=(directory path)`
    *   不设置此选项时，插桩打印的 IR 将打印到 `stderr`。如果使用此选项提供目录，每个 pass 对应的输出将打印到以 `(directory path)` 为根的目录树中的文件。为每个 pass 创建的路径反映了 IR 和 pass 流水线的嵌套结构。
    *   以下示例说明了在具有位于两个嵌套 `builtin.module` 操作中的两个 `func.func` 的 IR 上运行 pass 流水线时创建的文件树。
    *   子目录的名称反映父操作名称以及这些操作的符号名称（如果存在）。
    *   打印器维护与 pass 为目标的操作及其隔离于上方的父操作关联的计数器。每个文件名都使用目标操作的计数器值的数字前缀。然后将每个父操作的计数器值前置。这给出了一个命名方式，可以很容易地区分哪些 pass 可能是并发运行的，哪些具有明确的顺序。在下面的示例中，对于两个 `1_1_pass4.mlir` 文件，第一个 1 指的是父操作的计数器，第二个指的是相应函数的计数器。

```
$ pipeline="builtin.module(pass1,pass2,func.func(pass3,pass4),pass5)"
$ mlir-opt foo.mlir -pass-pipeline="$pipeline" -mlir-print-ir-tree-dir=/tmp/pipeline_output
$ tree /tmp/pipeline_output

/tmp/pass_output
├── builtin_module_the_symbol_name
│   ├── 0_pass1.mlir
│   ├── 1_pass2.mlir
│   ├── 2_pass5.mlir
│   ├── func_func_my_func_name
│   │   ├── 1_0_pass3.mlir
│   │   ├── 1_1_pass4.mlir
│   ├── func_func_my_other_func_name
│   │   ├── 1_0_pass3.mlir
│   │   ├── 1_1_pass4.mlir
```

*   `mlir-use-nameloc-as-prefix`
    * 如果您的源 IR 有命名位置（`loc("named_location")`），那么传递此标志将使用这些名称（`named_location`）作为相应 SSA 标识符的前缀：
    
      ```mlir
      %1 = memref.load %0[] : memref<i32> loc("alice")  
      %2 = memref.load %0[] : memref<i32> loc("bob")
      %3 = memref.load %0[] : memref<i32> loc("bob")
      ```
      
      将打印为
    
      ```mlir
      %alice = memref.load %0[] : memref<i32>
      %bob = memref.load %0[] : memref<i32>
      %bob_0 = memref.load %0[] : memref<i32>
      ```
      
      如果使用适当的位置，这些名称也将通过 pass 保留到新创建的操作中。
      

## 崩溃和失败重现

MLIR 中的 [pass 管理器](#pass-manager)包含一个内置机制，用于在崩溃或 [pass 失败](#pass-failure)时生成可重现文件。此功能可通过 `PassManager::enableCrashReproducerGeneration` 或命令行标志 `mlir-pass-pipeline-crash-reproducer` 启用。在任何情况下，都会提供一个参数，对应于应写入可重现文件的输出 `.mlir` 文件名。可重现文件包含正在执行的 pass 管理器的配置，以及任何 pass 运行之前的初始 IR。重现文件以外部资源的形式存储在汇编格式中。一个可能的可重现文件形式如下：

```mlir
module {
  func.func @foo() {
    ...
  }
}

{-#
  external_resources: {
    mlir_reproducer: {
      pipeline: "builtin.module(func.func(cse,canonicalize),inline)",
      disable_threading: true,
      verify_each: true
    }
  }
#-}
```

可以通过指定 `-run-reproducer` 标志将转储的配置传递给 `mlir-opt`。这将导致解析重现文件的配置并调整必要的 opt 状态，例如配置 pass 管理器、上下文等。

除了指定文件名外，还可以注册一个 `ReproducerStreamFactory` 函数，在崩溃时调用该函数并将重现文件写入其流。

### 本地重现生成

可以向 `PassManager::enableCrashReproducerGeneration` 传递一个额外标志，并通过命令行上的 `mlir-pass-pipeline-local-reproducer` 指定，该标志表示 pass 管理器应尝试生成"本地"重现文件。这将尝试生成一个包含失败 pass 之前的 IR 的重现文件。这对于已知崩溃在特定 pass 内的情况，或者原始输入依赖于可能并不总是可用的组件（如方言或 pass）的情况非常有用。

注意：本地重现生成需要禁用多线程（`-mlir-disable-threading`）

例如，如果上一个示例中的失败来自 `canonicalize` pass，则将生成以下重现文件：

```mlir
module {
  func.func @foo() {
    ...
  }
}

{-#
  external_resources: {
    mlir_reproducer: {
      pipeline: "builtin.module(func.func(canonicalize))",
      disable_threading: true,
      verify_each: true
    }
  }
#-}
```
