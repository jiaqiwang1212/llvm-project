# 第 2 章：添加一个简单的新变换操作

## 设置以添加新变换

在定义新的变换操作之前，我们需要选择其实现的位置。虽然 MLIR 鼓励上游贡献，但修改主 Transform 方言并非总是可行或值得，例如当变换特定于某个树外方言且该方言本身不在上游时。

Transform 方言使用方言扩展机制，允许在不修改方言本身的情况下注入额外的操作。方言扩展与上下文一起注册，并在方言本身被加载时随之加载。扩展定义非常简洁：

```cpp
// In MyExtension.cpp.
#include "mlir/Dialect/Transform/IR/TransformDialect.h"

// Define a new Transform dialect extension. This uses the CRTP idiom to
// identify extensions.
class MyExtension : public ::mlir::transform::TransformDialectExtension<MyExtension> {
public:
  // The extension must derive the base constructor.
  using Base::Base;

  // This function initializes the extension, similarly to `initialize` in
  // dialect  definitions. List individual operations and dependent dialects
  // here.
  void init();
};

void MyExtension::init() {
  // Similarly to dialects, an extension can declare a dependent dialect. This
  // dialect will be loaded along with the extension and, therefore, along with
  // the Transform  dialect. Only declare as dependent the dialects that contain
  // the attributes or types used by transform operations. Do NOT declare as
  // dependent the dialects produced during the transformation.
  //
  // declareDependentDialect<MyDialect>();

  // When transformations are applied, they may produce new operations from
  // previously unloaded dialects. Typically, a pass would need to declare
  // itself dependent on the dialects containing such new operations. To avoid
  // confusion with the dialects the extension itself depends on, the Transform
  // dialects differentiates between:
  //   - dependent dialects, which are used by the transform operations, and
  //   - generated dialects, which contain the entities (attributes, operations,
  //     types) that may be produced by applying the transformation even when
  //     not present in the original payload IR.
  // In the following chapter, we will be add operations that generate function
  // calls and structured control flow operations, so let's declare the
  // corresponding dialects as generated.
  declareGeneratedDialect<::mlir::scf::SCFDialect>();
  declareGeneratedDialect<::mlir::func::FuncDialect>();

  // Finally, we register the additional transform operations with the dialect.
  registerTransformOps<
    // TODO: list the operation classes.
  >();
}
```

这些操作本身可以使用 ODS 定义，方式与方言中的常规操作完全相同。

```tablegen
// In MyExtension.td
#ifndef MY_EXTENSION
#define MY_EXTENSION

include "mlir/Dialect/Transform/IR/TransformDialect.td"
include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def MyOp : Op<Transform_Dialect, "transform.my.op", [
    // TODO: interfaces and traits here.
   ]> {
  let summary = "my transform op";
  // TODO: define the operation properties.
}

#endif // MY_EXTENSION
```

与方言类似，我们必须使用 Tablegen 来生成这些操作的头文件和实现文件。可以通过以下方式指示 CMake 来完成：


```sh
# In CMakeLists.txt next to MyExtension.td.

# Tell Tablegen to use MyExtension.td as input.
set(LLVM_TARGET_DEFINITIONS MyExtension.td)

# Ask Tablegen to generate op declarations and definitions from ODS.
mlir_tablegen(MyExtension.h.inc -gen-op-decls)
mlir_tablegen(MyExtension.cpp.inc -gen-op-defs)

# Add a CMakeTarget we can depend on to ensure the generation happens before the compilation.
add_public_tablegen_target(MyExtensionIncGen)

# Don't forget to generate the documentation, this will produce a MyExtension.md under
# Dialects.
add_mlir_doc(MyExtension MyExtension Dialects/ -gen-op-doc)
```

```sh
# In CMakeLists.txt next to MyExtension.cpp
add_mlir_library(
  # Library called MyExtension.
  MyExtension

  # Built from the following source files.
  MyExtension.cpp

  # Make sure ODS declaration and definitions are generated before compiling
  # this.
  DEPENDS
  MyExtensionIncGen

  # Link in the transform dialect, and all generated dialects.
  LINK_LIBS PUBLIC
  MLIRTransformDialect
  MLIRFuncDialect
  MLIRSCFDialect
)
```

这将生成两个文件 `MyExtension.h.inc` 和 `MyExtension.cpp.inc`，它们分别应被包含到变换操作的声明和定义中。

```c++
// In MyExtension.h.
#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"

#define GET_OP_CLASSES
#include "MyExtension.h.inc"
```

```c++
// In MyExtension.cpp.

#include "MyExtension.h"

#define GET_OP_CLASSES
#include "MyExtension.cpp.inc"

// …
void MyExtension::init() {
  // …

  // Finally, we register the additional transform operations with the dialect.
  // List all  operations generated from ODS. This call will perform additional
  // checks that the  operations implement the transform and memory effect
  // interfaces required by the dialect interpreter and assert if they do not.
  registerTransformOps<
#define GET_OP_LIST
#include "MyExtension.cpp.inc"
  >();
}
```

## 定义变换操作

完成上述设置后，我们现在可以定义新的变换操作来重写函数调用了。这与在方言中定义常规操作完全相同。注意，Transform 方言要求操作实现 `TransformOpInterface` 以及 `MemoryEffectsOpInterface`，以指示操作数是被消耗还是只读。我们的操作可以按如下方式定义：

```tablegen
// In MyExtension.td.

// Define the new operation. By convention, prefix its name with the name of the
// dialect  extension, "my.". The full operation name will be further prefixed
// with "transform.".
def ChangeCallTargetOp : Op<Transform_Dialect, "my.change_call_target",
    // Indicate that the operation implements the required TransformOpInterface
    // and MemoryEffectsOpInterface.
    [DeclareOpInterfaceMethods<TransformOpInterface>,
     DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
  // Provide a brief and a full description. It is recommended that the latter
  // describes the effects on the operands and how the operation processes
  // various failure modes.
  let summary = "Changes the callee of a call operation to the specified one";
  let description = [{
    For each `func.call` payload operation associated with the handle, changes
    its callee to be the symbol whose name is provided as an attribute to this operation.

    Generates a silenceable failure if the operand is associated with payload operations that are not `func.call`. Only reads the operand.
  }];

  // The arguments include the handle to the payload operations and the
  // attribute that specifies the new callee. The handle must implement
  // TransformHandleTypeInterface.
  // We use a string attribute as the symbol may not exist in the transform IR
  // so the verification may fail.
  let arguments = (ins
    TransformHandleTypeInterface:$call,
    StrAttr:$new_target);

  // The results are empty as the transformation does not produce any new
  // payload.
  let results = (outs);

  // Provide nice syntax.
  let assemblyFormat = "$call `,` $new_target attr-dict `:` type($call)";
}
```

为完成变换操作的定义，我们需要实现接口方法。`TransformOpInterface` 目前只要求实现一个方法——`apply`——它执行实际的变换。一种好的实践是将方法主体限制为对 Transform 方言构造的操作，并将实际变换实现为独立函数，以便在代码的其他地方使用。与重写模式类似，所有 IR 修改都必须通过提供的重写器进行。

```c++
// In MyExtension.cpp

// Implementation of our Transform dialect operation.
// This operation returns a tri-state result that can be one of:
// - success when the transformation succeeded;
// - definite failure when the transformation failed in such a way that
//   following transformations are impossible or undesirable, typically it could
//   have left payload IR in an invalid state; it is expected that a diagnostic
//   is emitted immediately before returning the definite error;
// - silenceable failure when the transformation failed but following
//   transformations are still applicable, typically this means a precondition
//   for the transformation is not satisfied and the payload IR has not been
//   modified. The silenceable failure additionally carries a Diagnostic that
//   can be emitted to the user.
::mlir::DiagnosedSilenceableFailure mlir::transform::ChangeCallTargetOp::apply(
    // The rewriter that should be used when modifying IR.
    ::mlir::transform::TransformRewriter &rewriter,
    // The list of payload IR entities that will be associated with the
    // transform IR values defined by this transform operation. In this case, it
    // can remain empty as there are no results.
    ::mlir::transform::TransformResults &results,
    // The transform application state. This object can be used to query the
    // current associations between transform IR values and payload IR entities.
    // It can also carry additional user-defined state.
    ::mlir::transform::TransformState &state) {

  // First, we need to obtain the list of payload operations that are associated
  // with the operand handle.
  auto payload = state.getPayloadOps(getCall());

  // Then, we iterate over the list of operands and call the actual IR-mutating
  // function. We also check the preconditions here.
  for (Operation *payloadOp : payload) {
    auto call = dyn_cast<::mlir::func::CallOp>(payloadOp);
    if (!call) {
      DiagnosedSilenceableFailure diag = emitSilenceableError()
          << "only applies to func.call payloads";
      diag.attachNote(payloadOp->getLoc()) << "offending payload";
      return diag;
    }

    updateCallee(call, getNewTarget());
  }

  // If everything went well, return success.
  return DiagnosedSilenceableFailure::success();
}
```

`MemoryEffectsOpInterface` 的实现必须指定该操作对其操作数（消耗或只读）以及对载荷 IR（修改或只读）的副作用。Transform 方言验证器会检查副作用是否存在，若不存在则在调试构建中断言。

```c++
// In MyExtension.cpp

void ChangeCallTargetOp::getEffects(
    ::llvm::SmallVectorImpl<::mlir::MemoryEffects::EffectInstance> &effects) {
  // Indicate that the `call` handle is only read by this operation because the
  // associated operation is not erased but rather modified in-place, so the
  // reference to it remains valid.
  onlyReadsHandle(getCall(), effects);

  // Indicate that the payload is modified by this operation.
  modifiesPayload(effects);
}
```

## 注册与使用

这已经足够定义变换操作了。唯一剩余的部分是提供可从项目 `main` 函数调用的扩展注册钩子。


```c++
// In TransformDialect.cpp (don't forget a declaration in TransformDialect.h);

void registerMyExtension(::mlir::DialectRegistry &registry) {
  registry.addExtensions<MyExtension>();
}
```

注册扩展后，就可以在 Transform 方言解释器中使用我们的新操作了。上游测试 pass 可以直接使用，它实际上存在于 `mlir/test/Examples/transform/Ch2/sequence.mlir` 中，其中包含 `microkernel` 的实现。

```mlir
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
      %arg0: !transform.any_op,
      %arg1: !transform.op<"linalg.matmul">,
      %arg2: !transform.op<"linalg.elementwise">) {
    // Since the %arg2 handle is associated with both elementwise operations,
    // we need to split it into two handles so we can target only the second
    // elementwise operation.
    %add, %max = transform.split_handle %arg2
        : (!transform.op<"linalg.elementwise">)
        -> (!transform.any_op, !transform.any_op)

    // The actual tiling transformation takes tile sizes as attributes. It
    // produces a handle to the loop generated during tiling.
    %tiled, %loop = transform.structured.tile_using_forall %max
                    tile_sizes [8, 32]
        : (!transform.any_op) -> (!transform.any_op, !transform.any_op)

    // We can now fuse the other operations into the loop. Here, we fuse
    // operations one-by-one. This requires the operation that is being fused
    // to define the value used within the loop, so the order of such fusions
    // is important. We could also use "transform.merge_handles" to obtain
    // a single handle to all operations and give it to
    // `fuse_into_containing_op` that would take care of the ordering in this
    // case.
    %add_fused, %loop2 = transform.structured.fuse_into_containing_op %add into %loop
        : (!transform.any_op, !transform.any_op) -> (!transform.any_op, !transform.any_op)
    %matmul_fused, %loop3 = transform.structured.fuse_into_containing_op %arg1
                    into %loop2
        : (!transform.op<"linalg.matmul">, !transform.any_op)
       -> (!transform.any_op, !transform.any_op)

    // Tile again to get the desired size. Note that this time this tiles the
    // "add" operation and fuses matmul into the loop, but doesn't affect the
    // "max" operation. This illustrates the precise targeting with the
    // transform dialect. Otherwise, it is difficult to differentiate "add" and
    // "max", both of which having the same kind.
    %tiled_second, %loop_second = transform.structured.tile_using_forall %add_fused
                        tile_sizes [4, 4]
        : (!transform.any_op) -> (!transform.any_op, !transform.any_op)
    %matmul_fused_2, %loop_second_2 = transform.structured.fuse_into_containing_op %matmul_fused
                      into %loop_second
        : (!transform.any_op, !transform.any_op) -> (!transform.any_op, !transform.any_op)

    // Since outlining is currently only implemented for region-holding
    // operations such as loops, use tiling to size 1 to materialize the outer
    // loop that is going to be outlined.
    %_0, %loop_third = transform.structured.tile_using_forall %tiled_second tile_sizes [1]
        : (!transform.any_op) -> (!transform.any_op, !transform.any_op)
    %_1, %outline_target = transform.structured.fuse_into_containing_op %matmul_fused_2 into %loop_third
        : (!transform.any_op, !transform.any_op) -> (!transform.any_op, !transform.any_op)
    %func, %call = transform.loop.outline %outline_target
                   {func_name = "outlined"}
        : (!transform.any_op) -> (!transform.any_op, !transform.any_op)

    // Rewrite the call target.
    transform.my.change_call_target %call, "microkernel" : !transform.any_op

    transform.yield
  }
}
```

## 附录：自动生成的文档

[include "Tutorials/transform/MyExtensionCh2.md"]
