# 第 4 章：用 Transform 操作匹配载荷

**请参阅 [mlir/test/Examples/transform/Ch4](https://github.com/llvm/llvm-project/tree/main/mlir/test/Examples/transform/Ch4) 下持续测试的 MLIR 文件版本。**

到目前为止，我们在应用 transform 方言脚本时假设特定的载荷操作是在调用 transform 方言解释器时由调用者标识的。这可能被视为与从方言驱动变换的理念相悖，因为变换目标必须通过 transform 方言解释器外部的机制来标识，例如在 C++ 中以编程方式调用解释器时，或通过前面章节中看到的 pass 参数。这也会增加实际开销（由于与 C++ 中解释器的交互增加），以及同时操作两个接口的认知负担。为了解决这个问题，Transform 方言提出了一套用于_匹配_需要被变换的载荷操作的操作子集。

_匹配（Match）_操作只是带有一些额外保证的变换操作。特别地，它们不被期望修改载荷 IR，并且当其操作数（通常是载荷操作句柄）未与具有所需属性的载荷 IR 对象关联时（如操作名称或参数类型），它们应当失败。通过使用简单的组合子操作，可以直接在 transform 方言内部建立更高层次的匹配与重写基础设施。


## 简单匹配

让我们重新考虑[第 1 章](Ch1.md/#chaining-transformations-with-handles)中的"全连接层"示例，为方便起见在此复现。


```mlir
// Original function to optimize.
func.func @fc_relu(%lhs: tensor<512x512xf32>, %rhs: tensor<512x512xf32>,
                   %bias: tensor<512x512xf32>, %output: tensor<512x512xf32>)
                   -> tensor<512x512xf32> {
  // Matrix-matrix multiplication.
  %matmul = linalg.matmul
            ins(%lhs, %rhs: tensor<512x512xf32>, tensor<512x512xf32>)
            outs(%output: tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise addition.
  %biased = linalg.elementwise kind=#linalg.elementwise_kind<add>
    ins(%matmul, %bias : tensor<512x512xf32>, tensor<512x512xf32>)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise max with 0 (ReLU).
  %c0f = arith.constant 0.0 : f32
  %relued = linalg.elementwise kind=#linalg.elementwise_kind<max_signed>
    ins(%biased, %c0f : tensor<512x512xf32>, f32)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>
  func.return %relued : tensor<512x512xf32>
}

```


在第 1 章中，我们调用测试变换解释器 pass 时附加了额外参数 `bind-first-extra-to-ops=linalg.matmul bind-second-extra-to-ops=linalg.elementwise`，以提供操作句柄的初始关联。现在，我们可以使用匹配操作在载荷 IR 中发现相关操作。匹配操作可以与"常规"变换操作结合使用，例如使用 `transform.collect_matching` 组合子操作，它利用具名序列的概念来组织匹配器。


```mlir
// The module containing named sequences must have an attribute allowing them
// to enable verification.
module @transforms attributes { transform.with_named_sequence } {
  // Entry point. This takes as the only argument the root operation (typically
  // pass root) given to the transform interpreter.
  transform.named_sequence @__transform_main(
      %root: !transform.any_op {transform.readonly}) {
    // Collect operations that match the criteria specified in named sequence.
    // If the named sequence fails with a silenceable failure, silences it (the
    // message is forwarded to the debug stream). If the named sequence
    // succeeds, appends its results to the results of this operation.
    %elemwise = transform.collect_matching @match_elemwise in %root
      : (!transform.any_op) -> !transform.any_op
    %matmul = transform.collect_matching @match_matmul in %root
      : (!transform.any_op) -> !transform.any_op
    transform.include @print_elemwise failures(propagate)  (%elemwise)
      : (!transform.any_op) -> ()
    transform.include @print_matmul failures(propagate)  (%matmul)
      : (!transform.any_op) -> ()

    transform.yield
  }

  // This is a matcher sequence. It is given an operation to match and the
  // match is considered successful unless any nested operation produces a
  // failure. The values yielded by this operation will be forwarded to the
  // rewriter sequence on success.
  transform.named_sequence @match_elemwise(
      %entry: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.match.operation_name %entry ["linalg.elementwise"]
      : !transform.any_op
    transform.yield %entry : !transform.any_op
  }
  transform.named_sequence @match_matmul(
      %entry: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.match.operation_name %entry ["linalg.matmul"] : !transform.any_op
    transform.yield %entry : !transform.any_op
  }

  // This is a rewriter sequence.
  transform.named_sequence @print_elemwise(
      %elemwise_binary: !transform.any_op {transform.readonly}) {
    transform.debug.emit_remark_at
      %elemwise_binary, "elementwise binary" : !transform.any_op
    transform.yield
  }
  transform.named_sequence @print_matmul(
      %matmul: !transform.any_op {transform.readonly}) {
    transform.debug.emit_remark_at %matmul, "matmul" : !transform.any_op
    transform.yield
  }
}

```


该脚本可以使用非测试解释器 pass 在翻译单元的根操作上执行，无需额外标志：`mlir-opt --transform-interpreter`。它将在 `linalg.elementwise` 和 `linalg.matmul` 操作处输出相应的备注。在调试构建中，基础设施提供了一种方便的方法来理解匹配过程：向 `mlir-opt` 或派生工具传递 `-debug-only=transform-matcher`。它会将匹配操作产生的可抑制失败消息打印到调试流中，例如：


```
<...>
[transform-matcher] matching %0 = linalg.matmul ins(%arg0, %arg1 : tensor<512x512xf32>, tensor<512x512xf32>) outs(%arg3 : tensor<512x512xf32>) -> tensor<512x512xf32> @0x5622eee08410
[transform-matcher] matcher match_elemwise failed: wrong operation name
<...>
```


这现在足以运行第 1 章中其余的变换脚本，将 `%arg1` 替换为 `%matmul`，将 `%arg2` 替换为 `%elemwise`。


## 匹配操作链

上面的匹配器仍然比较朴素，因为它匹配载荷根下_所有_特定类型的操作。这些操作可能互不相关，例如可能属于不同的函数。即使它们在同一个函数中，如果存在多组这样的操作，我们也无法用这种方法区分它们。实际上，我们希望匹配一组特定的操作，其中 `matmul` 操作产生的结果被一个逐元素操作使用，该逐元素操作又以相似的方式馈送给另一个逐元素操作。

这可以使用以下匹配器序列来实现。


```mlir
// This is also a matcher sequence. It is similarly given an operation to
// match and nested operations must succeed in order for a match to be deemed
// successful. It starts matching from the last operation in the use-def chain
// and goes back because each operand (use) has exactly one definition.
transform.named_sequence @match_matmul_elemwise(
    %last: !transform.any_op {transform.readonly})
    -> (!transform.any_op, !transform.any_op, !transform.any_op) {
  // The last operation must be an elementwise binary.
  transform.match.operation_name %last ["linalg.elementwise"]
    : !transform.any_op
  // Its first operand must be defined by another operation, to which we
  // will get a handle here. We are guaranteed that the first operand exists
  // because we know the operation is binary, but even in absence of such a
  // guarantee, this operation would have produced a silenceable failure when
  // `%last` does not have enough operands.
  %middle = transform.get_producer_of_operand %last[0]
    : (!transform.any_op) -> !transform.any_op
  // The defining operation must itself be an elementwise binary.
  transform.match.operation_name %middle ["linalg.elementwise"]
    : !transform.any_op
  // And the first operand of that operation must be defined by yet another
  // operation.
  %matmul = transform.get_producer_of_operand %middle[0]
    : (!transform.any_op) -> !transform.any_op
  // And that operation is a matmul.
  transform.match.operation_name %matmul ["linalg.matmul"] : !transform.any_op
  // We will yield the handles to the matmul and the two elementwise
  // operations separately.
  transform.yield %matmul, %middle, %last
    : !transform.any_op, !transform.any_op, !transform.any_op
}
```

该匹配器在存在其他 `elemwise` 和 `matmul` 操作时同样适用，它返回的是_相关联_的操作三元组，而非按发现顺序排列的操作。它的使用方式与之前类似：

```mlir
// Alternative entry point.
transform.named_sequence @__transform_main(
    %root: !transform.any_op {transform.readonly}) {
  // Collect groups of operations that match the criteria specified in the
  // named sequence.
  %matmul, %el1, %el2 = transform.collect_matching @match_matmul_elemwise in %root
    : (!transform.any_op) -> (!transform.any_op, !transform.any_op, !transform.any_op)
  %elemwise = transform.merge_handles %el1, %el2 : !transform.any_op

  transform.include @print_elemwise failures(propagate)  (%elemwise)
    : (!transform.any_op) -> ()
  transform.include @print_matmul failures(propagate)  (%matmul)
    : (!transform.any_op) -> ()

  transform.yield
}
```


## 定义匹配操作

上面的操作链匹配器在存在其他操作时是正确的，但对许多感兴趣的情况仍不够健壮。特别是，使用 `transform.get_producer_of_operand %last[0]` 要求逐元素操作的_第一个_操作数由另一个操作产生。然而，同一变换策略可能无论操作数位置如何都适用：许多二元操作是满足结合律的。让我们借此机会引入一个新的匹配操作。具体而言，我们希望该操作在_任意_一个操作数满足某些条件（可以用其他匹配操作来表达）时成功。我们还希望它返回一些状态以及匹配到的操作数在操作数列表中的位置。

匹配操作的定义方式与其他变换操作类似，唯一的区别是还需额外实现 `MatchOpInterface`。注意，该接口_没有额外的方法_（尽管将来可能会添加），仅用作验证契约，表明该操作旨在用于匹配，且不会尝试变换载荷。我们操作的最简定义如下：


```tablegen
// Define the new operation. By convention, prefix its name with `match`
// followed by the name of the dialect extension.
def HasOperandSatisfyingOp : TransformDialectOp<"match.my.has_operand_satisfying",
    [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>,
     DeclareOpInterfaceMethods<TransformOpInterface>,
     // Indicate that the operation implements MatchOpInterface in addition to
     // the TransformOpInterface. This interface is only used as a tag at this
     // point and has no methods that are mandatory to implement.
     MatchOpInterface,
     SingleBlockImplicitTerminator<"::mlir::transform::YieldOp">]> {
  let summary = "Succeed if any of the operands matches all nested criteria";
  let arguments = (ins TransformHandleTypeInterface:$op);
  let results = (outs TransformParamTypeInterface:$position,
                      Variadic<Transform_AnyHandleOrParamType>:$results);

  // Match operations can be arbitrarily complex, e.g., containing regions.
  let regions = (region SizedRegion<1>:$body);
  let hasVerifier = 1;
  let assemblyFormat = [{
    $op `:` functional-type($op, results) attr-dict-with-keyword $body
  }];
}
```


它以与载荷操作（其操作数将被匹配）关联的句柄为参数，有一个包含匹配条件的单块 region，并在成功匹配时返回被匹配操作数的位置以及从主体 yield 出的任何其他变换值。

匹配逻辑在 `TransformOpInterface` 的 `apply` 方法中实现，可以轻松与其他变换操作组合。管理解释器状态和递归进入块的所有设施均以与"常规"变换操作相同的方式提供。匹配操作应返回可抑制失败以指示匹配失败，并立即传播确定性失败。如果它们有嵌套操作，则应处理（并且在大多数情况下抑制）嵌套操作应用时产生的可抑制失败。对于我们的操作，匹配本质上是一个循环，遍历（单个）载荷操作的所有操作数，并应用嵌套的变换操作，直到所有操作数之一全部成功。


```cpp
// Matcher ops implement `apply` similarly to other transform ops. They are not
// expected to modify payload, but use the tri-state result to signal failure or
// success to match, as well as potential irrecoverable errors.
mlir::DiagnosedSilenceableFailure
mlir::transform::HasOperandSatisfyingOp::apply(
    mlir::transform::TransformRewriter &rewriter,
    mlir::transform::TransformResults &results,
    mlir::transform::TransformState &state) {
  // For simplicity, only handle a single payload op. Actual implementations
  // can use `SingleOpMatcher` trait to simplify implementation and document
  // this expectation.
  auto payloadOps = state.getPayloadOps(getOp());
  if (!llvm::hasSingleElement(payloadOps))
    return emitSilenceableError() << "expected single payload";

  // Iterate over all operands of the payload op to see if they can be matched
  // using the body of this op.
  Operation *payload = *payloadOps.begin();
  for (OpOperand &operand : payload->getOpOperands()) {
    // Create a scope for transform values defined in the body. This corresponds
    // to the syntactic scope of the region attached to this op. Any values
    // associated with payloads from now on will be automatically dissociated
    // when this object is destroyed, i.e. at the end of the iteration.
    // Associate the block argument handle with the operand.
    auto matchScope = state.make_region_scope(getBody());
    if (failed(state.mapBlockArgument(getBody().getArgument(0),
                                      {operand.get()}))) {
      return DiagnosedSilenceableFailure::definiteFailure();
    }

    // Iterate over all nested matchers with the current mapping and see if they
    // succeed.
    bool matchSucceeded = true;
    for (Operation &matcher : getBody().front().without_terminator()) {
      // Matcher ops are applied similarly to any other transform op.
      DiagnosedSilenceableFailure diag =
          state.applyTransform(cast<TransformOpInterface>(matcher));

      // Definite failures are immediately propagated as they are irrecoverable.
      if (diag.isDefiniteFailure())
        return diag;

      // On success, keep checking the remaining conditions.
      if (diag.succeeded())
        continue;

      // Report failure-to-match for debugging purposes and stop matching this
      // operand.
      assert(diag.isSilenceableFailure());
      DEBUG_MATCHER(DBGS_MATCHER()
                    << "failed to match operand #" << operand.getOperandNumber()
                    << ": " << diag.getMessage());
      (void)diag.silence();
      matchSucceeded = false;
      break;
    }
    // If failed to match this operand, try other operands.
    if (!matchSucceeded)
      continue;

    // If we reached this point, the matching succeeded for the current operand.
    // Remap the values associated with terminator operands to be associated
    // with op results, and also map the parameter result to the operand's
    // position. Note that it is safe to do here despite the end of the scope
    // as `results` are integrated into `state` by the interpreter after `apply`
    // returns rather than immediately.
    SmallVector<SmallVector<MappedValue>> yieldedMappings;
    transform::detail::prepareValueMappings(
        yieldedMappings, getBody().front().getTerminator()->getOperands(),
        state);
    results.setParams(cast<OpResult>(getPosition()),
                      {rewriter.getI32IntegerAttr(operand.getOperandNumber())});
    for (auto &&[result, mapping] : llvm::zip(getResults(), yieldedMappings))
      results.setMappedValues(result, mapping);
    return DiagnosedSilenceableFailure::success();
  }

  // If we reached this point, none of the operands succeeded the match.
  return emitSilenceableError()
         << "none of the operands satisfied the conditions";
}

```


按照约定，实现 `MatchOpInterface` 的操作不得修改载荷 IR，因此必须在其副作用中指定它们只读取操作数句柄和载荷。


```cpp
void transform::CollectMatchingOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  onlyReadsHandle(getRoot(), effects);
  producesHandle(getResults(), effects);
  onlyReadsPayload(effects);
}
```


现在，该操作可以被包含在 transform 方言扩展中，加载后在我们的匹配器中使用。具体而言，我们将用它来表明示例中"max"逐元素操作的任意一个操作数都可以由前一个逐元素操作产生。前一个操作仍然要求矩阵乘法产生第一个操作数（为简化起见）。更新后的匹配器序列如下：


```mlir
transform.named_sequence @match_matmul_elemwise(
    %last: !transform.any_op {transform.readonly})
    -> (!transform.any_op, !transform.any_op, !transform.any_op,
        !transform.param<i32>) {
  // The last operation must be an elementwise binary.
  transform.match.operation_name %last ["linalg.elementwise"]
    : !transform.any_op

  // One of its operands must be defined by another operation, to which we
  // will get a handle here. This is achieved thanks to a newly defined
  // operation that tries to match operands one by one using the match
  // operations nested in its region.
  %pos, %middle = transform.match.my.has_operand_satisfying %last
      : (!transform.any_op) -> (!transform.param<i32>, !transform.any_op) {
  ^bb0(%operand: !transform.any_value):
    // The operand must be defined by an operation.
    %def = transform.get_defining_op %operand
      : (!transform.any_value) -> !transform.any_op
    // The defining operation must itself be an elementwise binary.
    transform.match.operation_name %def ["linalg.elementwise"]
      : !transform.any_op
    transform.yield %def : !transform.any_op
  }

  // And the first operand of that operation must be defined by yet another
  // operation.
  %matmul = transform.get_producer_of_operand %middle[0]
    : (!transform.any_op) -> !transform.any_op
  // And that operation is a matmul.
  transform.match.operation_name %matmul ["linalg.matmul"] : !transform.any_op
  // We will yield the handles to the matmul and the two elementwise
  // operations separately.
  transform.yield %matmul, %middle, %last, %pos
    : !transform.any_op, !transform.any_op, !transform.any_op,
      !transform.param<i32>
}
```


这实现了预期效果，可以匹配 `max(add(matmul(...), bias), 0)` 和 `max(0, add(matmul(...), bias))` 两种形式，并将结果放在相同的值中。`%pos` 值是一个 transform 方言_参数（parameter）_，用于存储在变换应用过程中已知为常量的实体列表。最常见的情况下，参数是数值，但通常可以是任何 MLIR 属性。

为了演示各组操作是独立匹配的，让我们使用 `transform.foreach_match` 操作，它允许在 transform 方言内部实现简单的高层模式重写方法（对于高级或底层模式重写，请考虑 PDL(L) 或 C++ 重写 API）。它将一个匹配器具名序列映射到一个动作具名序列，每当前者成功时就调用后者。


```mlir
// Traverses the payload IR associated with the operand handle, invoking
// @match_matmul_elemwise on each of the operations. If the named sequence
// succeeds, i.e., if none of the nested match (transform) operations
// produced a silenceable failure, invokes @print_matmul_elemwise and
// forwards the values yielded as arguments of the new invocation. If the
// named sequence fails with a silenceable failure, silences it (the message
// is forwarded to the debug stream). Definite failures are propagated
// immediately and unconditionally, as usual.
transform.foreach_match in %root
  @match_matmul_elemwise -> @print_matmul_elemwise
  : (!transform.any_op) -> !transform.any_op
```


`@print_matmul_elemwise` 具名序列（可在 `multiple.mlir` 中找到）将使用带有操作数位置的参数来区分两组操作。


## 推断特征的匹配器

上面描述的匹配器序列虽然可以在 transform 方言解释器内驱动变换，但相当基础，因为它们主要依赖操作名称和使用-定义链。使用 API 或各种声明式重写规则的替代实现在表达能力上几乎毫不逊色，有时甚至更简洁。Transform 方言匹配操作的真正优势在于能够定义载荷_推断属性_的匹配器，即那些不能直接作为操作属性访问或通过任何简单 IR 组件关系获取的属性。

这类匹配器的实用性可以通过对原始示例的轻微修改轻松展示。如果矩阵乘法用 `linalg.generic` 而非 `linalg.matmul` 来表达为张量收缩的特例，基于操作名称的匹配器就不再适用。然而，这种表示非常常见，在原始输入和变换过程中都可能出现，例如当高维收缩被分解为围绕矩阵乘法的循环时。

要成为一个（可能经过转置的）矩阵乘法，`linalg.generic` 操作必须具备以下特征：



*   总秩为 3。
*   两个输入以迭代维度的投影置换方式访问。
*   一个输出以迭代维度的投影置换方式访问。
*   迭代维度可以细分为 LHS 平行、RHS 平行和归约维度。
*   主体块由一个乘法和一个加法组成。

这些特征大多数可以从操作的属性中推导出来，例如总秩对应 `iterators` 属性中的条目数，但几乎没有一个可以在 IR 中直接访问，或以任何声明式形式访问（后者通常仅限于检查属性或类型的存在或精确匹配）。Transform 方言允许在匹配操作的 `apply` 方法中实现这些特征，并在多个匹配场景中复用。对于结构化线性代数载荷操作，许多此类匹配操作已在 `structured` 扩展中直接提供。利用上面列出的特征，它们足以几乎逐字地实现矩阵乘法匹配器：


```mlir
transform.named_sequence @match_generic_matmul(
    %candidate: !transform.any_op {transform.readonly}) -> !transform.any_op {
  // Match a structured linear algebra operation.
  transform.match.structured %candidate : !transform.any_op {
  ^bb0(%c: !transform.any_op):
    // With a rank equal to 3.
    %rank = transform.match.structured.rank %c
      : (!transform.any_op) -> !transform.param<i64>
    %c3 = transform.param.constant 3 : i64 -> !transform.param<i64>
    transform.match.param.cmpi eq %rank, %c3 : !transform.param<i64>

    // With 2 inputs.
    %n_ins = transform.match.structured.num_inputs %c
      : (!transform.any_op) -> !transform.param<i64>
    %c2 = transform.param.constant 2 : i64 -> !transform.param<i64>
    transform.match.param.cmpi eq %n_ins, %c2 : !transform.param<i64>

    // With 1 output (note that structured ops in destination passing style
    // has as many inits as outputs).
    %n_inits = transform.match.structured.num_inits %c
      : (!transform.any_op) -> !transform.param<i64>
    %c1 = transform.param.constant 1 : i64 -> !transform.param<i64>
    transform.match.param.cmpi eq %n_inits, %c1 : !transform.param<i64>

    // All inputs and inits are accessed with a projected permutation.
    transform.match.structured.input %c[all] {projected_permutation}
      : !transform.any_op
    transform.match.structured.init %c[0] {projected_permutation}
      : !transform.any_op

    // The body is a mulf/addf contraction with appropriate dimensions.
    transform.match.structured.body %c
      { contraction = ["arith.mulf", "arith.addf"] } : !transform.any_op
    %batch, %lhs, %rhs, %reduction =
    transform.match.structured.classify_contraction_dims %c
      : (!transform.any_op)
      -> (!transform.param<i64>, !transform.param<i64>, !transform.param<i64>,
          !transform.param<i64>)


    // There is one of lhs, rhs and reduction dimensions and zero batch
    // dimensions.
    %n_batch = transform.num_associations %batch
      : (!transform.param<i64>) -> !transform.param<i64>
    %n_lhs = transform.num_associations %lhs
      : (!transform.param<i64>) -> !transform.param<i64>
    %n_rhs = transform.num_associations %rhs
      : (!transform.param<i64>) -> !transform.param<i64>
    %n_reduction = transform.num_associations %reduction
      : (!transform.param<i64>) -> !transform.param<i64>
    %c0 = transform.param.constant 0 : i64 -> !transform.param<i64>
    transform.match.param.cmpi eq %n_batch, %c0 : !transform.param<i64>
    transform.match.param.cmpi eq %n_lhs, %c1 : !transform.param<i64>
    transform.match.param.cmpi eq %n_rhs, %c1 : !transform.param<i64>
    transform.match.param.cmpi eq %n_reduction, %c1 : !transform.param<i64>
  }
  transform.yield %candidate : !transform.any_op
}
```


虽然此示例利用了具有相当非平凡 C++ 实现的收缩特定匹配器，但 Transform 方言足够灵活，可以在需要时直接实现这种推理。例如，可以将每个输入的访问映射作为参数获取，并将被访问的维度提取为其他参数，然后相互比较，以确保在 `m,n,k` 循环表示法中，LHS 的下标为 `m,k`，RHS 的下标为 `k,n`，init/result 的下标为 `m,n`。

## 附录：自动生成的文档

[include "Tutorials/transform/MyExtensionCh4.md"]
