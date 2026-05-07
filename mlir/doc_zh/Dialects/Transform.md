# Transform 方言（Dialect）

细粒度变换控制方言。更多入门信息请参阅[教程](../Tutorials/transform)。

[TOC]

## 概述

本方言提供了一组操作，可用于通过 IR 的另一部分来控制 IR（中间表示）的变换。它将被变换的 IR 称为载荷 IR（payload IR），将指导变换的 IR 称为变换 IR（transform IR）。

本方言的主要用途是对单个 IR 对象（操作或值）或其集合进行细粒度变换的编排。例如，可以在载荷 IR 中查找具有特定属性（如较大尺寸）的类循环操作，对这些操作（仅对这些操作）应用循环分块，然后对前次变换生成的内层循环应用循环展开。因此，它并非用来替代通道（pass）基础设施，也不是用来替代模式重写基础设施。在最常见的情况下，变换 IR 将由一个通道来处理并应用到载荷 IR 上。Transform 方言表达的变换可以使用模式基础设施或任何其他相关 MLIR 组件来实现。

以下 IR 大致展示了本方言中操作的外观（不使用实际存在的操作）：

```mlir
%0 = transform.loop.find { size > 42 } : !transform.interface<tileable>
%1 = transform.compute_trailing_tile_size %0 : !transform.param<index>
%2:2 = transform.loop.tile %0 tile_sizes(1, 4, %1)
      : (!transform.interface<tileable>)
     -> (!transform.op<loop>, !transform.op<loop>)
%3 = transform.get_op_result [0] %2#0 : !transform.any_value
transform.assign_to_fast_memory %3
transform.loop.unroll %1#1 : !transform.op<loop>
```

Transform 方言中使用的值可以对应于：

  * 载荷 IR 中的操作集合；

  * 载荷 IR 中的值集合；

  * 在变换方言执行时已知的参数（属性）集合。

前两类值也分别称为操作和值的*句柄（handle）*。在上例中，`%0` 对应于载荷 IR 中满足条件的循环集合，`%2` 分别对应于分块变换产生的外层和内层循环组。`%3` 对应于分块后外层循环产生的值集合。`%1` 对应于为 `%0` 所对应的每个操作选取的分块大小列表。

一个操作句柄（如 `%0`）可以与多个载荷操作相关联。这在概念上是一个操作集合，除非操作另有说明，否则不应对操作的顺序做任何假设。类似地，值句柄（如 `%3`）可以与一组载荷 IR 值相关联。Transform 方言操作可以将表示句柄和参数的值的任意组合作为操作数和结果。大多数 Transform IR 算子支持映射到多个载荷对象的操作数值，它们通常对每个映射对象依次应用相应变换（"批量执行"）。与此约定的偏差在 Transform IR 算子的文档中有所描述。

参数（如上例中的 `%1`）在变换 IR 中有两个逻辑角色。在基于参数的控制中，它们携带执行变换所定义的显式控制所需的值，例如：

```mlir
%0 = transform.match.structured.rank %linalg_op_handle : !transform.param<index>
%1 = transform.param.constant 3 : i32 -> !transform.param<index>
transform.execute_if_cmpi eq %0, %1 : !transform.param<index>, !transform.param<index>
// Some nested body of transform ops
```

另一方面，参数也可以与载荷 IR 相关联，此时执行时的具体值与变换 IR 的执行无关。换言之，参数既可以与变换 IR 关联，也可以与载荷 IR 关联。注意，通常不建议在变换控制中使用包含任意属性（attribute）的参数。基于参数的控制应尽量使用显式类型。

变换 IR 的值具有变换 IR 类型，这些类型应恰好实现以下之一：

  * [TransformHandleTypeInterface](#transformhandletypeinterface-transformhandletypeinterface)，

  * [TransformValueHandleTypeInterface](#transformvaluehandletypeinterface-transformvaluehandletypeinterface)，

  * [TransformParamTypeInterface](#transformparamtypeinterface-transformparamtypeinterface)。

这些类型接口的目标，除了为所接受的类型提供公共基础外，还用于验证关联对象的属性。例如，句柄类型接口实现可以检查所有关联的载荷 IR 操作是否实现了"TileableOp"接口或具有特定的"loop"种类。类似地，值句柄类型接口实现可以检查关联的载荷 IR 值是否为块参数或具有特定类型，参数类型接口可以检查关联属性是否包含非负整数值。这些属性用于静态指示与 Transform 方言操作关联的变换的前置和后置条件。当载荷对象操作首次与变换句柄关联时，会验证这些条件。按照惯例，Transform 方言操作应通过在其定义和验证器中强制执行操作数类型约束来指明其操作数的窄前置条件。相反，操作对其结果的约束应尽可能少。然后，变换操作的特定实例可以用比操作中约束更严格的结果类型来创建（例如，"查找"操作仅将结果类型约束为变换 IR 类型，而其具体实例可以具有更严格约束的类型，如实现"tilable"接口）。验证将在变换执行时进行。这种方法允许在变换 IR 中捕获载荷 IR 操作属性，而无需过度使用类型转换或在方言扩展之间产生耦合。这是冗长度/复杂性与静态加固之间的权衡，未来可以修订。

总体而言，Transform IR 算子预期包含在单个顶层算子中。此类顶层算子指定如何应用其所包含操作描述的变换，例如，`transform.sequence` 逐一执行变换，若任何变换失败则整体失败。此类算子预期具有 `PossibleTopLevelTransformOpTrait`，并且可以不带参数使用。

使用 Transform 方言表达的程序变换可以通过调用以下函数以编程方式触发：

```c++
LogicalResult transform::applyTransforms(
    Operation *payloadRoot,
    const RaggedArray<transform::MappedValue> &extraMappings,
    TransformOpInterface transform,
    const TransformOptions &options);
```

该函数将顶层 `transform` 指定的变换应用于 `payloadRoot` 中包含的载荷 IR。载荷根操作将与顶层变换算子入口块的第一个参数相关联。此块可以有额外的参数、句柄或参数。它们将与作为 `extraMappings` 提供的值相关联。如果提供了错误数量的映射，调用将报告错误并返回。

## 方言扩展机制

本方言被设计为可扩展的，即本方言的客户端可以使用 `TransformDialectExtension` 机制向本方言注入额外的操作。这使得方言可以避免对变换实现的依赖，同时避免引入特定于方言的变换方言。在上例中，这些操作可能是由一个假想的 `loop` 方言注入的，而非定义在本方言中，因此具有共同的前缀。

建议为注入的操作加上一个或多个点分隔的单词前缀，以指示是哪个扩展添加了它们。对于特定于方言的变换，前缀自然是方言的名称，例如 `transform.affine.reschedule`。对于与方言无关的变换（通常通过接口实现），前缀可以来自接口名称或某个公共概念，例如 `transform.loop.tile` 可以应用于任何实现了 `TileableOpInterface` 的类循环操作。方言扩展的 C++ 类应在其名称中包含该前缀，例如上述情况中的 `AffineTransformDialectExtension` 或 `LoopTransformDialectExtension`。无前缀的操作名称保留给 Transform 方言中直接定义的算子。

注入到方言中的操作必须：

  * 实现 `TransformOpInterface`，以在载荷 IR 上执行相应的变换。

  * 实现 `MemoryEffectsOpInterface`，以标注变换 IR 操作对载荷 IR 以及变换 IR 值与载荷 IR 操作之间映射的影响。可用影响的描述见下文。

在方言加载时，会在运行时检查接口实现的存在，以允许这些实现由独立的方言扩展提供（如有需要）。

与操作类似，也可以使用相同的扩展机制将额外的类型注入到方言中。这些类型必须：

  * 恰好实现 `TransformHandleTypeInterface`、`TransformValueHandleTypeInterface`、`TransformParamTypeInterface` 之一。

## 副作用

Transform 方言依赖 MLIR 副作用建模来优化变换 IR。更具体地说，它提供了几个副作用资源对象，并期望操作描述其对这些资源的影响。

  * `TransformMappingResource` — 对应于变换 IR 值与载荷 IR 操作之间映射的副作用资源。

    - 来自该资源的 `Allocate` 效果意味着创建一个新的映射条目，始终伴随 `Write` 效果。

    - 来自该资源的 `Read` 效果意味着访问该映射。

    - 对该资源的 `Free` 效果表示删除映射条目，通常发生在修改了与某个变换 IR 操作数关联的载荷 IR 操作的变换之后。始终伴随 `Read` 效果。

  * `PayloadIRResource` — 对应于载荷 IR 本身的副作用资源。

    - 来自该资源的 `Read` 效果意味着访问载荷 IR。

    - 对该资源的 `Write` 效果意味着修改载荷 IR。几乎总是伴随 `Read`。

变换 IR 中值的典型流程如下。大多数操作产生新的变换 IR 值，并立即将其与载荷 IR 操作列表关联。这对应于 `TransformMappingResource` 上的 `Allocate` 和 `Write` 效果，并且通常需要至少对 `PayloadIRResource` 有 `Read` 效果。仅检查载荷 IR 以产生新句柄的变换操作通常仅限于对其操作数产生这些效果。修改载荷 IR 的变换操作被认为会*消耗*作为操作数提供的句柄，即对其具有 `Read` 和 `Free` 效果。与通常的内存效果一样，在值被释放后使用它是不正确的。在变换 IR 的情况下，该值可能与已被变换修改甚至删除的载荷 IR 操作相关联，因此引用它们是没有意义的。当需要进一步变换时，变换操作可以返回*新的*句柄，供后续操作读取或消耗。

## 执行模型

变换从用户指定的顶层变换 IR 操作开始，并应用于某个用户指定的载荷 IR 范围，该范围由包含待变换 IR 的载荷 IR 算子标识。用户有责任正确选择范围，并/或避免变换修改给定范围之外的 IR。顶层变换 IR 操作可以包含更多变换操作，并按所需顺序执行它们。

变换应用函数产生三态状态：

- 成功；
- 可恢复（可静默）失败；
- 不可恢复失败。

变换容器操作可以拦截可恢复失败并执行所需的恢复步骤，从而自身成功。另一方面，它们必须传播不可恢复失败。对于此类失败，诊断信息会立即发出，而可恢复失败的诊断信息则会推迟发出。变换容器操作也可能无法从理论上可恢复的失败中恢复，在这种情况下，它们可以将其传播给父级，或发出诊断信息并将失败转为不可恢复的。顶层变换 IR 操作应用产生的可恢复失败被视为不可恢复的。

如果某个先前操作的应用产生了失败，变换容器操作允许"跳过"一些嵌套操作。这在概念上可以理解为有一个全局"可恢复错误寄存器"，每个变换操作作为副作用对其进行读/写访问。如果寄存器已包含错误描述，则跳过变换，控制流继续到下一个操作。

注意，可静默失败（如果发出）是一个编译器*错误*而非警告。如果变换尚未修改载荷 IR（即报告前置条件失败），则预期产生可静默失败；如果以违反变换操作语义或会导致后置条件失败的方式修改了 IR，则产生不可恢复失败。一些识别后续变换的载荷 IR 目标的"导航"操作可能在概念上存在"匹配失败"，在执行模型中这被视为成功执行，但会导致句柄与空载荷 IR 操作列表相关联。

## 句柄失效

Transform 方言的执行模型允许一个载荷 IR 操作与*多个*句柄相关联，也允许嵌套的载荷 IR 操作与不同句柄相关联。类似地，一个载荷 IR 值可以与多个变换 IR 值句柄相关联。当一个变换 IR 操作消耗一个句柄时，通常表示相应的载荷 IR 对象已被销毁，不应再被引用。*可能*指向已被擦除的载荷 IR 对象的变换 IR 句柄被*失效*。变换 IR 中仅存在失效句柄不是问题，但*使用*它会导致未定义行为。失效的句柄可以类比为悬空指针。注意，即使与句柄关联的部分载荷 IR 对象仍然存活，*整个*句柄也会失效。

以下句柄失效规则适用。

  * 当一个操作句柄被消耗时，以下内容被失效：

    - 与被消耗句柄关联的某个载荷操作相关联的操作句柄；

    - 与上述载荷操作*嵌套*在其中的某个操作相关联的操作句柄；

    - 与上述任何操作的任何结果相关联的值句柄；

    - 与包含在附加于上述任何操作的区域中的块的任何参数相关联的值句柄。

  * 当一个值句柄被消耗时，以下内容被失效：

    - 与产生被消耗句柄所关联的某个值（作为操作结果时）的载荷操作相关联的操作句柄；

    - 与上述载荷操作*嵌套*在其中的载荷操作相关联的操作句柄；

    - 与（递归地）*包含*在将被消耗句柄所关联的某个值定义为参数的块中的载荷操作相关联的操作句柄（当关联值为块参数时）；注意，相邻的块不受影响；

    - 与上述任何操作的任何结果相关联的值句柄，包括将被消耗句柄所关联的值定义为结果的操作的所有结果；

    - 与包含在附加于上述任何操作的区域中的块的任何参数相关联的值句柄。

更直观地说，消耗一个句柄会使任何可能指向以最近操作或块为根的载荷 IR 子树中定义或包含的对象的句柄失效。

Transform 方言基础设施具有在应用变换之前检查变换 IR 算子操作数是否失效的能力。但是，此类检查计算开销较大，必须通过 `TransformOptions` 显式启用。此外，`transform-dialect-check-uses` 通道（pass）会在句柄可能在被消耗后使用时发出警告，但这是抽象地进行的，不处理载荷 IR。

与参数（非句柄）关联的值不能被失效。

## 预期用途与集成

本方言提供的变换控制基础设施在定位上大致介于重写模式与通道之间。由变换操作执行的变换可能足够复杂，至少需要一组模式来实现。它也预期比通道更加专注：通道通常在 IR 中的所有地方应用相同的变换，而 Transform 方言控制的变换将应用于小部分操作，这些操作例如通过模式匹配操作选择，或由先前的变换生成。尽管技术上可行，但不建议将通道流水线作为变换算子实现的一部分来运行。

使用本方言的主要场景之一是变换的细粒度链式组合。例如，类循环操作可以将其迭代域分成两部分，实现为独立的循环（称为索引集拆分的变换），然后对每部分进行不同的变换（例如，第一个循环分块，第二个展开），并在主变换周围应用必要的使能和清理模式：

```mlir
// <generate %loop, e.g., by pattern-matching>
// ...
%parts:2 = transform.loop.split %loop { upper_bound_divisible_by = 8 }
transform.loop.tile %parts#0 { tile_sizes = [8] }
transform.loop.unroll %parts#1 { full }
```

这种组合作为独立通道很难实现，因为假想的"分块"和"展开"通道需要以某种方式区分先前通道产生的循环部分（两者是相同的操作，用特定于通道的信息污染操作可能是不可取的）。将运行组合变换的通道实现会遇到由于多种可能的变换组合而导致的组合爆炸问题，或需要深度通道参数化，其最终形式是一种特定于方言的 IR 来指定通道应运行哪些变换。Transform 方言为在此类情况下控制变换提供了统一的、可扩展的机制。

Transform 方言应由驱动变换应用的"解释器"通道来消费。为确保可扩展性和可组合性，该通道实际上不需要执行算子指定的变换。相反，变换由变换算子本身通过 `TransformOpInterface` 来实现。通道作为入口点，处理变换操作的流程并负责记账。因此，Transform 方言不提供解释器通道。相反，它提供了一组工具，客户端可以用来定义自己的解释器通道，或作为更复杂通道的一部分。例如，变换 IR 中值与载荷 IR 中操作之间的映射，或按顺序应用给定块中算子指定的变换的函数。注意，变换算子可以具有包含更多变换算子的区域，算子本身指导如何将变换控制流分派到这些区域。这种方法允许客户端决定其输入中变换 IR 的相对位置（例如，嵌套模块、独立模块、某些操作的可选区域等），注册额外的变换操作，并执行特定于客户端的记账。

## 对基础设施的影响

尽管范围限于单个方言，此功能在概念上属于 MLIR 基础设施。它旨在尽量减少侵入性并采用选择加入的方式。

一些基础设施组件可能会增加额外功能以支持 Transform 方言。特别是，模式基础设施可能会添加额外的钩子，以识别变换的"主要结果"或通知外部观察者有关对某些操作所做的更改。这些预计不会影响基础设施的现有用途。

为了可重用性，变换应实现为从变换算子的接口方法调用的工具函数，而非让方法直接作用于载荷 IR。

## 类型定义

[include "Dialects/TransformTypes.md"]

## 核心操作

[include "Dialects/TransformOps.md"]

## Tune 扩展操作

[include "Dialects/TuneExtensionOps.md"]

## SMT 扩展操作

[include "Dialects/SMTExtensionOps.md"]

## Affine 变换操作

[include "Dialects/AffineLoopTransformOps.md"]

## ARM Neon 变换操作

[include "Dialects/ArmNeonVectorTransformOps.md"]

## ARM SVE 变换操作

[include "Dialects/ArmSVEVectorTransformOps.md"]

## Bufferization 变换操作

[include "Dialects/BufferizationTransformOps.md"]

## Debug 变换操作

[include "Dialects/DebugExtensionOps.md"]

## DLTI 变换操作

[include "Dialects/DLTITransformOps.md"]

## IRDL（扩展）变换操作

[include "Dialects/IRDLExtensionOps.md"]

## Func 变换操作

[include "Dialects/FuncTransformOps.md"]

## GPU 变换操作

[include "Dialects/GPUTransformOps.md"]

## Loop（扩展）变换操作

[include "Dialects/LoopExtensionOps.md"]

## Loop（SCF）变换操作

[include "Dialects/SCFLoopTransformOps.md"]

## MemRef 变换操作

[include "Dialects/MemRefTransformOps.md"]

## PDL（扩展）变换操作

[include "Dialects/PDLExtensionOps.md"]

## 结构化（Linalg）匹配操作

[include "Dialects/LinalgStructuredMatchOps.md"]

## 结构化（Linalg）变换操作

[include "Dialects/LinalgStructuredTransformOps.md"]

## Tensor 变换操作

[include "Dialects/TensorTransformOps.md"]

## Vector 变换操作

[include "Dialects/VectorTransformOps.md"]

[include "Dialects/TransformTypeInterfaces.md"]

[include "Dialects/TransformOpInterfaces.md"]
