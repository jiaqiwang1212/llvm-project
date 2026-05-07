# Action：追踪与调试基于 MLIR 的编译器

[TOC]

另请参阅 MLIR 开放会议上的[幻灯片](https://mlir.llvm.org/OpenMeetings/2023-02-23-Actions.pdf)和[录像](https://youtu.be/ayQSyekVa3c)，该会议对此功能进行了演示。

## 概述

`Action` 是一种将任意粒度的转换封装起来的手段，使框架能够出于调试或追踪目的对其进行拦截，包括以编程方式跳过某个转换（类似于 LLVM 中的"编译器燃料"或"调试计数器"的概念）。因此，"执行一个 pass"是一个 Action，"尝试应用某个规范化模式"或"对此循环进行分块"也都是 Action。

在 MLIR 中，pass 和模式（pattern）是封装通用 IR 转换的主要抽象。观察转换过程的主要方式是开启 IR 的"调试打印"（例如，使用 -mlir-print-ir-after-all 在每次 pass 执行后打印 IR）。在此之上，还可以使用 -debug 选项来启用来自转换本身的更详细日志，从而获得更细粒度的追踪。然而，这种方法存在一定的扩展性问题：它仅限于单一的文本流，该文本流可能极为庞大，且需要事后费力地在日志中逐行爬取。反复进行多次日志收集和分析的迭代过程非常耗时，且对于较小的输入程序之外的情形往往不够实用。

`Action` 框架不对高层驱动器如何控制执行做任何假设，它仅提供一个将两者连接在一起的框架。关于 `Action` 执行的工作流概览如下：

- 编译器开发者定义一个 `Action` 类，用于表示他们正在开发的转换或工具。
- 根据需要，开发者确定单个转换单元，并将其分发给 `MLIRContext` 执行。
- 外部实体向 action 管理器注册一个"action 处理器"，并提供围绕转换执行的逻辑。

`外部实体`的确切定义故意保持不透明，以便支持更有趣的处理器实现。

## 将转换封装在 Action 中

在现有代码或新代码中启用 Action 追踪，需要完成两部分工作：1) 定义实际的 `Action` 类，以及 2) 将转换封装在一个 lambda 函数中。

对于"action"的粒度没有任何限制，它可以简单到"执行这个折叠"，也可以复杂到"运行这个 pass 流水线"。一个 action 由以下部分组成：

```c++
/// 自定义 Action 可以通过派生自 `tracing::ActionImpl` 来最简化地定义。
class MyCustomAction : public tracing::ActionImpl<MyCustomAction> {
public:
  using Base = tracing::ActionImpl<MyCustomAction>;
  /// Action 使用一个 IRUnit 数组（即 Operation、Block 或 Region）初始化，
  /// 这些 IRUnit 为受转换影响的 IR 提供上下文。
  MyCustomAction(ArrayRef<IRUnit> irUnits)
      : Base(irUnits) {}
  /// 此标签应唯一标识此 action，可在处理过程中用于过滤匹配。
  static constexpr StringLiteral tag = "unique-tag-for-my-action";
  static constexpr StringLiteral desc =
      "This action will encapsulate a some very specific transformation";
};
```

任何转换都可以通过 `MLIRContext` 使用此 `Action` 进行分发：

```c++
context->executeAction<ApplyPatternAction>(
    [&]() {
      rewriter.setInsertionPoint(op);

      ...
    },
    /*IRUnits=*/{op, region});
```

一个 action 还可以携带任意的有效载荷（payload），例如，我们可以用以下成员扩展上面的 `MyCustomAction` 类：

```c++
/// 自定义 Action 可以通过派生自 `tracing::ActionImpl` 来最简化地定义。它可以拥有任意成员！
class MyCustomAction : public tracing::ActionImpl<MyCustomAction> {
public:
  using Base = tracing::ActionImpl<MyCustomAction>;
  /// Action 使用一个 IRUnit 数组（即 Operation、Block 或 Region）初始化，
  /// 这些 IRUnit 为受转换影响的 IR 提供上下文。
  /// 这里还可以要求传入其他构造函数参数。
  MyCustomAction(ArrayRef<IRUnit> irUnits, int count, PaddingStyle padding)
      : Base(irUnits), count(count), padding(padding) {}
  /// 此标签应唯一标识此 action，可在处理过程中用于过滤匹配。
  static constexpr StringLiteral tag = "unique-tag-for-my-action";
  static constexpr StringLiteral desc =
      "This action will encapsulate a some very specific transformation";
  /// Action 可以携带额外的成员
  int count;
  PaddingStyle padding;
};
```

在分发 `Action` 时，这些新成员必须作为参数传入：

```c++
context->executeAction<ApplyPatternAction>(
    [&]() {
      rewriter.setInsertionPoint(op);

      ...
    },
    /*IRUnits=*/{op, region},
    /*count=*/count,
    /*padding=*/padding);
```

## 拦截 Action

当一个转换通过 `Action` 执行时，可以通过在 `MLIRContext` 上设置的处理器直接对其进行拦截：

```c++
  /// 可注册到上下文的 action 处理器签名。
  using HandlerTy =
      std::function<void(function_ref<void()>, const tracing::Action &)>;

  /// 注册一个处理器，用于处理通过此上下文分发的 action。
  /// 可以设置 nullptr 处理器来禁用之前设置的处理器。
  void registerActionHandler(HandlerTy handler);
```

此处理器接受两个参数：第一个是封装在回调中的转换，第二个是对关联 action 对象的引用。处理器对执行拥有完全控制权，因此它也可以决定不执行回调而直接返回，从而完全跳过该转换！

## MLIR 提供的处理器

MLIR 提供了一些预定义的 action 处理器，可供大多数基于 MLIR 构建的项目直接使用。

### 调试计数器

在调试编译器问题时，["二分法"](https://en.wikipedia.org/wiki/Bisection_(software_engineering))是一种定位问题根本原因的有效技术。`Debug Counters`（调试计数器）通过将一个计数器值附加到特定 action 上，并根据计数器的值启用或禁用该 action 的执行，从而将此技术应用于调试 action。计数器通过"skip"（跳过）值和"count"（计数）值来控制 action 的执行。"skip"值用于跳过调试 action 的一定数量的初始执行。"count"值用于在调试 action 执行了指定次数后阻止其继续执行（不包括已被跳过的执行次数）。如果"skip"值为负数，则 action 将始终执行。如果"count"值为负数，则在达到"skip"值后 action 将始终执行。例如，一个 `skip=47`、`count=2` 的调试 action 计数器将跳过前 47 次执行，然后执行两次，最后阻止任何进一步的执行。借助一些工具，可以自动选择计数器使用的值，从而精确定位可能导致所调查 bug 的那次调试 action 执行。

注意：DebugCounter action 处理器不支持多线程执行，只应在禁用了多线程的 MLIRContext 中使用（例如通过 `-mlir-disable-threading`）。

#### 命令行配置

`DebugCounter` 处理器提供了多个用于配置计数器的选项。主要选项是 `mlir-debug-counter`，它接受一个逗号分隔的 `<count-name>=<counter-value>` 列表。`<counter-name>` 是调试 action 的标签，加上 `-skip` 或 `-count` 后缀。`-skip` 后缀设置计数器的"skip"值，`-count` 后缀设置计数器的"count"值。`<counter-value>` 是用于计数器的数值。下面是使用上面定义的 `MyCustomAction` 的示例：

```shell
$ mlir-opt foo.mlir -mlir-debug-counter=unique-tag-for-my-action-skip=47,unique-tag-for-my-action-count=2
```

上述配置将跳过 `ApplyPatternAction` 的前 47 次执行，然后执行两次，最后阻止任何进一步的执行。

注意：每个计数器目前只有一个 `skip` 值和一个 `count` 值，这意味着 `skip`/`count` 序列不会被链式组合。

可以使用 `mlir-print-debug-counter` 选项在所有计数器累积完成后打印调试计数器信息。信息以如下格式打印：

```shell
DebugCounter counters:
<action-tag>                   : {<current-count>,<skip>,<count>}
```

例如，使用上述选项可以查看 action 被执行了多少次：

```shell
$ mlir-opt foo.mlir -mlir-debug-counter=unique-tag-for-my-action-skip=-1 -mlir-print-debug-counter --pass-pipeline="builtin.module(func.func(my-pass))" --mlir-disable-threading

DebugCounter counters:
unique-tag-for-my-action         : {370,-1,-1}
```

### ExecutionContext

`ExecutionContext` 是一个组件，它以可组合的方式，统一提供大多数编译器调试工具所需的各类功能。

![IMG](/actions/ActionTracing_ExecutionContext.png)

`ExecutionContext` 本身作为处理器注册到 MLIRContext，并追踪所有已执行的 action，为每个线程维护一个 action 执行栈。它充当中间件，在允许调试器注入和控制的同时处理 action 执行流程。

- 可以向 `ExecutionContext` 注册多个 `Observer`（观察者）。当一个 action 被分发执行时，它会在执行转换的前后分别传递给每个 `Observer`。
- 可以向 `ExecutionContext` 注册多个 `BreakpointManager`（断点管理器）。当一个 action 被分发执行时，它会依次传递给每个已注册的 `BreakpointManager`，直到某个管理器匹配该 action 并返回一个有效的 `Breakpoint` 对象。在这种情况下，客户端在 `ExecutionContext` 上设置的"回调"会被调用；否则，转换将直接执行。
- 可以向 `ExecutionContext` 注册一个回调：
  `using CallbackTy = function_ref<Control(const ActionActiveStack *)>;`，当 `Action` 触发 `BreakPoint` 时调用该回调。返回值的类型为 `Control`，是一个枚举值，用于指示 `ExecutionContext` 下一步如何执行：
  ```c++
  /// 允许上下文客户端控制 action 执行的枚举。
  /// - Apply：执行该 action。
  /// - Skip：跳过该 action。
  /// - Step：执行该 action，并在下一个 action 之前暂停执行，
  ///         包括在当前 action 完成之前遇到的嵌套 action。
  /// - Next：执行该 action，并在当前 action 完成后、下一个 action 之前暂停执行。
  /// - Finish：执行该 action，仅在到达父/外层操作时暂停执行。
  ///           如果没有外层操作，则继续执行而不停止。
  enum Control { Apply = 1, Skip = 2, Step = 3, Next = 4, Finish = 5 };
  ```
  由于回调实际上控制着执行，因此在任何时候只能注册一个回调。

#### 调试器 ExecutionContext 钩子

MLIR 为 `ExecutionContext` 提供了一个回调，该回调实现了一个小型运行时，适合 `gdb` 或 `lldb` 等调试器交互式地控制执行。可以通过 `mlir::setupDebuggerExecutionContextHook(executionContext);` 进行设置，或在 `mlir-opt` 中使用 `--mlir-enable-debugger-hook` 标志。此运行时暴露了一组 C API 函数，可从调试器中调用以：

- 设置断点，匹配 action 标签或与 action 关联的 IR 的 `FileLineCol` 位置。
- 设置要返回给 `ExecutionContext` 的 `Control` 标志。
- 控制一个"光标"，允许在 IR 中导航并从与 action 关联的 IR 上下文中检查它。

此运行时的实现可以作为其他程序化控制执行实现的参考示例。

#### 日志观察者

提供了一个观察者，允许将 action 执行日志记录到指定的流中。可以通过 `mlir-opt` 中的 `--log-actions-to=<filename>` 来使用，并可选地使用 `--log-mlir-actions-filter=<FileLineCol>` 过滤输出。此观察者目前不是线程安全的。
