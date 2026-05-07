# 第 3 章：不止于简单的 Transform 操作

## 类型约束与 ApplyEach Trait

对每个载荷操作单独应用并要求其为特定类型的变换操作是一种常见模式。可以使用 Transform 方言类型来指定类型前提条件。具体而言，我们可以将预期的操作数类型从宽泛的 `TransformHandleTypeInterface` 改为更精确的 `Transform_ConcreteOp<"func.call">`。此外，我们使用 `TransformEachOpTrait` trait 来提供 `apply` 方法的骨架实现，该实现负责验证、遍历载荷以及拼接结果。改进后的 ODS 操作定义如下：

```tablegen
// In MyExtension.td.

// Define the new operation. By convention, prefix its name with the name of the dialect extension, "my.". The full operation name will be further prefixed with "transform.".
def ChangeCallTargetOp : Op<Transform_Dialect, "my.change_call_target",
    // Indicate that the operation implements the required TransformOpInterface and
    // MemoryEffectsOpInterface. Use the TransformEach trait to provide the
    // implementation for TransformOpInterface.
    [TransformOpInterface, TransformEachOpTrait,
     DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  // Provide a brief and a full description. It is recommended that the latter describes
  // the effects on the operands and how the operation processes various failure modes.
  let summary = "Changes the callee of a call operation to the specified one";
  let description = [{
    For each `func.call` payload operation associated with the handle, changes its
    callee to be the symbol whose name is provided as an attribute to this operation.

    Generates a silenceable failure if the operand is associated with payload operations
    that are not `func.call`.
    Only reads the operand.
  }];

  // The arguments include the handle to the payload operations and the attribute that
  // specifies the new callee. The handle must implement TransformHandleTypeInterface.
  // We use a string attribute as the symbol may not exist in the transform IR so the
  // verification may fail.
  let arguments = (ins
    Transform_ConcreteOpType<"func.call">:$call,
    StrAttr:$new_target);

  // The results are empty as the transformation does not produce any new payload.
  let results = (outs);

  // Provide nice syntax.
  let assemblyFormat = "$call `,` $new_target attr-dict `:` type($call)";

  // Declare the function implementing the interface for a single payload operation.
  let extraClassDeclaration = [{
    ::mlir::DiagnosedSilenceableFailure applyToOne(
        ::mlir::transform::TransformRewriter &rewriter,
        ::mlir::func::CallOp call,
        ::mlir::transform::ApplyToEachResultList &results,
        ::mlir::transform::TransformState &state);
  }];
}
```

现在，我们无需定义带有循环的 `apply` 方法，只需定义一个应用于单个载荷操作的函数，trait 将处理其余的事情。

```c++
::mlir::DiagnosedSilenceableFailure ChangeCallTargetOp::applyToOne(
    ::mlir::transform::TransformRewriter &rewriter,
    ::mlir::func::CallOp call,
    ::mlir::transform::ApplyToEachResultList &results,
    ::mlir::transform::TransformState &state) {
  // Call the actual transformation function.
  updateCallee(call, getNewTarget());
  // Indicate success.
  return DiagnosedSilenceableFailure::success();
}
```

## 定义 Transform 类型

除操作外，Transform 方言还允许扩展定义并注入额外的属性和类型。如上所述，变换类型用于指定对载荷操作的约束。我们当前的调用重写操作只适用于 `func.call`。我们可能希望将其推广为适用于任何实现了 `CallOpInterface` 的载荷操作，但 Transform 方言目前没有提供检查载荷操作是否实现该接口的类型。让我们在我们的扩展中定义它。

类型定义与使用 ODS 定义方言类型的方式完全相同。

```tablegen
// Transform dialect allows additional types to be defined and injected.
def CallOpInterfaceHandle
  : TypeDef<Transform_Dialect, "CallOpInterfaceHandle",
      // The type must implement `TransformHandleTypeInterface`.
      [DeclareTypeInterfaceMethods<TransformHandleTypeInterface>]> {

  // The usual components of a type such as description, mnemonic and assembly format
  // should be provided.
  let summary = "handle to payload operations implementing CallOpInterface";
  let mnemonic = "my.call_op_interface";
  let assemblyFormat = "";
}
```

为了简洁，我们省略使用 Tablegen 生成声明和定义的过程，因为它与常规情况相同。

要完成变换类型的定义，必须实现接口方法。

```c++
// In MyExtension.cpp.

// The interface declares this method to verify constraints this type has on
// payload operations. It returns the now familiar tri-state result.
mlir::DiagnosedSilenceableFailure
mlir::transform::CallOpInterfaceHandleType::checkPayload(
    // Location at which diagnostics should be emitted.
    mlir::Location loc,
    // List of payload operations that are about to be associated with the
    // handle that has this type.
    llvm::ArrayRef<mlir::Operation *> payload) const {

  // All payload operations are expected to implement CallOpInterface, check this.
  for (Operation *op : payload) {
    if (llvm::isa<mlir::CallOpInterface>(op))
      continue;

    // By convention, these verifiers always emit a silenceable failure since they are
    // checking a precondition.
    DiagnosedSilenceableFailure diag = emitSilenceableError(loc)
        << "expected the payload operation to implement CallOpInterface";
    diag.attachNote(op->getLoc()) << "offending operation";
    return diag;
  }

  // If everything is okay, return success.
  return DiagnosedSilenceableFailure::success();
}

```

额外的属性和类型需要在扩展中与操作一起注册。

```c++
// In MyExtension.cpp.

void MyExtension::init() {
  // ...

  registerTypes<
#define GET_TYPEDEF_LIST
#include "MyExtensionTypes.cpp.inc"
  >();
}
```

该类型现在可以直接在 Transform 方言中使用，并可以用于操作中。在之前的 tablegen 定义中，`$call` 的类型必须是 `Transform_ConcreteOp<"func.call">`。通过将 `CallOpInterfaceHandle` 添加为 `$call` 的允许类型，对应的句柄可以是实现了该接口的任意操作。

```tablegen
def ChangeCallTargetOp : ... {
    let arguments = (ins
    // Allow the handle to be to concrete `func.call` ops as well as any op implementing
    // the `CallOpInterface`.
    AnyTypeOf<[Transform_ConcreteOpType<"func.call">, CallOpInterfaceHandle]>:$call,
    StrAttr:$new_target); 
}
```

然后，我们可以将以下代码添加到 `sequence.mlir` 中，并使用解释器运行它。

```mlir
  // Cast to our new type.
  %casted = transform.cast %call : !transform.any_op to !transform.my.call_op_interface
  // Using our new operation.
  transform.my.change_call_target %casted, "microkernel" : !transform.my.call_op_interface
```

## 操作数消耗

作为练习，让我们修改重写操作以消耗操作数。例如，若变换要将 `func.call` 操作重写为自定义操作 `my.mm4`，这将是必要的。由于操作数句柄现在被消耗，该操作可以返回一个指向新产生的载荷操作的新句柄。除此之外，变换操作的 ODS 定义保持不变。

```tablegen
// In MyExtension.td.

// Define another transform operation.
def CallToOp : Op<Transform_Dialect, "my.call_to_op",
     // Indicate that the operation implements the required TransformOpInterface and
     // MemoryEffectsOpInterface. Use the TransformEach trait to provide the
     // implementation for TransformOpInterface.
    [TransformOpInterface, TransformEachOpTrait,
     DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  // Summary and description omitted for brevity.

  // The argument is the handle to the payload operations.
  let arguments = (ins CallOpInterfaceHandle:$call);

  // The result is the handle to the payload operations produced during the
  // transformation.
  let results = (outs TransformHandleTypeInterface:$transformed);

  // Provide nice syntax.
  let assemblyFormat = "$call attr-dict `:` functional-type(operands, results)";

  // Declare the function implementing the interface for a single payload operation.
  let extraClassDeclaration = [{
    ::mlir::DiagnosedSilenceableFailure applyToOne(
        ::mlir::transform::TransformRewriter &rewriter,
        ::mlir::CallOpInterface call,
        ::mlir::transform::ApplyToEachResultList &results,
        ::mlir::transform::TransformState &state);
  }];
}
```

现在让我们看看接口方法的实现。

```c++
// In MyExtension.cpp.

::mlir::DiagnosedSilenceableFailure CallToOp::applyToOne(
    ::mlir::transform::TransformRewriter &rewriter,
    ::mlir::CallOpInterface call,
    ::mlir::transform::ApplyToEachResultList &results,
    ::mlir::transform::TransformState &state) {
  // Call the actual rewrite.
  Operation *rewritten = rewriteToOp(call);

  // Report an error if the rewriter produced a null pointer. Note that it may have
  // irreversibly modified the payload IR, so we produce a definite failure.
  if (!rewritten) {
    return emitDefiniteError() << "failed to rewrite call to operation";
  }

  // On success, push the resulting operation into the result list. The list is expected
  // to contain exactly one entity per result and per application. The handles will be
  // associated with lists of the respective values produced by each application.
  results.push_back(rewritten);

  // If everything is fine, return success.
  return DiagnosedSilenceableFailure::success();
}

void CallToOp::getEffects(
    ::llvm::SmallVectorImpl<::mlir::MemoryEffects::EffectInstance> &effects) {
  // Indicate using side effects that the operand handle is consumed, and the
  // result handle is produced.
  consumesHandle(getCall(), effects);
  producesHandle(getTransformed(), effects);

  // Indicate that the payload IR is modified.
  modifiesPayload(effects);
}
```

这些实现的整体流程与之前类似。应用时还需要指定将与其产生的句柄相关联的结果实体。成功时，操作需要为_所有_结果指定要关联的实体，即使列表为空。如果不这样做，将触发断言。失败时，解释器会自动将所有尚未定义的结果与空列表关联。

注意，由于 `applyToOne` 始终期望每次应用时每个结果句柄与一个载荷实体相关联，它不能用于对非空操作数句柄返回与空列表关联的句柄。在这种情况下，应直接使用 `apply`。

```c++
::mlir::DiagnosedSilenceableFailure SomeOtherOp::apply(
    ::mlir::transform::TransformRewriter &rewriter,
    ::mlir::transform::TransformResults &results,
    ::mlir::transform::TransformState &state) {
  // ...

  // Associate the result `transformed` with an empty list of payload operations.
  results.set(cast<OpResult>(getTransformed()), {});
  return DiagnosedSilenceableFailure::success();
}
```

## 内存效果 Traits

一些常见的内存效果模式也以 traits 的形式提供，以最小化样板代码。

*   `FunctionalStyleTransformOpTrait` 表示所有句柄类型的操作数被消耗，所有结果被产生，且载荷 IR 被修改。
*   `NavigationTransformOpTrait` 表示所有句柄类型的操作数只被读取，所有结果被产生，且载荷 IR 只被读取。

使用这些 traits 无需声明或定义 `MemoryEffectsOpInterface` 的方法。

```tablegen
// In MyExtension.td.

// Define another transform operation.
def CallToOp : Op<Transform_Dialect, "my.call_to_op",
     // Indicate that the operation implements the required TransformOpInterface.
     // Use the TransformEach trait to provide implementation of this interface.
    [TransformOpInterface, TransformEachOpTrait,
     // Indicate that the operation implements the required MemoryEffectsOpInterface.
     // Use the FunctionalStyle trait to provide the implementation for this interface.
     MemoryEffectsOpInterface, FunctionalStyleTransformOpTrait]> {
  // Summary and description omitted for brevity.

  // The argument is the handle to the payload operations.
  let arguments = (ins CallOpInterfaceHandle:$call);

  // The result is the handle to the payload operations produced during the
  // transformation.
  let results = (outs TransformHandleTypeInterface:$transformed);

  // Provide nice syntax.
  let assemblyFormat = "$call attr-dict `:` functional-type(operands, results)";

  // Declare the function implementing the interface for a single payload operation.
  let extraClassDeclaration = [{
    ::mlir::DiagnosedSilenceableFailure applyToOne(
        ::mlir::transform::TransformRewriter &rewriter,
        ::mlir::CallOpInterface call,
        ::mlir::transform::ApplyToEachResultList &results,
        ::mlir::transform::TransformState &state);
  }];
}
```

## 附录：自动生成的文档

[include "Tutorials/transform/MyExtensionCh3.md"]
