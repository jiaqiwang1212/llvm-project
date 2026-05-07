# 'shape' 方言（Dialect）

Shape 方言（Dialect）中操作（Operation）和类型（Type）的描述，以及其
[使用方式](#shape-方言的不同降级阶段)。

[include "Dialects/ShapeDialectOps.md"]

## Shape 方言的不同降级阶段

本节将简要介绍 shape 方言的不同用途以及这些用途之间的降级（lowering）过程。目前，shape 函数的降级共有 3 个阶段：

1.  _错误单子（Error monadic）/携带错误/用户规范_：
    这种"输入"形式将形状（shape）和是否处于错误状态都作为值（value）携带。因此，在该层级，所有操作都是纯操作（pure operation），产生和消费的值可能代表一个错误。

2.  _受约束（Constrained）_：
    该形式使用显式证据传递（explicit evidence passing）的变体，以利用现有编译器基础设施在优化过程中保留安全信息。

3.  _带副作用（Side-effecting）/断言（Asserting）_：
    这是最终降级形式，是带副作用操作（如 assert）的命令式形式，用于最终代码生成（codegen）。

下面我们通过矩阵乘法（matmul）的示例快速演示降级过程。

从如下错误单子形式的 matmul shape 函数开始[^wip_form1]：

```mlir
shape.function_library @shplib {

func.func @matmul(%lhs: !shape.value_shape, %rhs: !shape.value_shape) -> !shape.shape {
  %c1 = shape.const_size 1
  %c2 = shape.const_size 2
  // We could also allow rank etc operations directly on value_shape too, that
  // would make it nicer as "input" language, but keeping it explicit inside the
  // IR instead and then we could have helper methods in front-end language.
  %lhs_shape = shape.shape_of %lhs : !shape.value_shape -> !shape.shape
  %rhs_shape = shape.shape_of %rhs : !shape.value_shape -> !shape.shape
  %lhs_rank = shape.rank %lhs_shape : !shape.shape -> !shape.size
  %rhs_rank = shape.rank %rhs_shape : !shape.shape -> !shape.size
  // This is not minimal as one could ensure the ranks are the same below, also a
  // variadic meet would make it more concise too.
  %r = "shape.meet"(%lhs_rank, %rhs_rank) : (!shape.size, !shape.size) -> !shape.size
  %rank = shape.meet %c2, %r, error="requires rank 2 operands" :
    !shape.size, !shape.size -> !shape.size
  %l0, %l1 = "shape.split_at"(%lhs_shape, %c1) :
    (!shape.shape, !shape.size) -> (!shape.shape, !shape.shape)
  %r0, %r1 = "shape.split_at"(%rhs_shape, %c1) :
    (!shape.shape, !shape.size) -> (!shape.shape, !shape.shape)
  %c = shape.meet %l1, %r0, error="inner dimensions required to match" :
    !shape.shape, !shape.shape -> !shape.shape
  %res = shape.concat %l0, %r1
  // Should have `shape.return %res requires %c, %rank` to enable
  return %res : !shape.shape
}

} mapping {
  foo.matmul = @matmul
}
```

*   这里使用了默认的内建 func 和 return。更理想的做法是使用 `shape_func` 作为特殊的函数操作（function op），允许传回多个影响正确执行的结果（例如，作为错误合并）。
    *   这也意味着无法在普通函数内具体化（reify）它，而无需处理 shape.return——这是一个特性，因为这些更像是模板。
    *   目前我们也没有将 `meet` 标记为无副作用，以避免在拥有 `shape.return` 之前进行死代码消除（DCE），届时计算 meet 可以被视为纯计算并返回错误。
*   Meet 代表一个应当成立的约束，因此不应用来判断*是否*相等。例如，这意味着 `meet` 不能用来表示

    ```
       either(meet(x, y), meet(y,z))
    ```

*   以上内容本可以更简洁地写为

    ```
      concat(lhs[0], rhs[1]) if rank(lhs) == 2 &&
        rank(rhs) == 2 && lhs[1] == rhs[0]
    ```

    但这里不关注前端的具体形式。

我们将直接降级到"最"嵌套的形式（参见
[测试](https://github.com/tensorflow/tensorflow/blob/64062b5c51e04e370df26551d247496787d3f5c2/tensorflow/compiler/mlir/xla/tests/legalize-tf.mlir#L3088)
以获取具体化与合法化的示例）。在上面的例子中，这位于一个独立的 shape 函数库中，而在实际中我们通常会在降级过程中将其具体化，但为了简洁起见，这里作为独立的 shape 函数展示。

```mlir
func.func @matmul_shape1(%lhs: tensor<*xf32>, %rhs: tensor<*xindex>) -> tensor<?xindex> {
  %c1 = shape.const_size 1
  %c2 = shape.const_size 2
  // We allow `shape.shape_of` to return either a `!shape.shape` or
  // `tensor<?xindex>` type, in the case where the input is a tensor the most
  // refined type is a tensor of `index` but not required.
  %lhs_shape = shape.shape_of %lhs : tensor<*xf32> -> !shape.shape
  %rhs_shape = shape.shape_of %rhs : tensor<*xf32> -> !shape.shape
  %lhs_rank = shape.rank %lhs_shape : !shape.shape -> !shape.size
  %rhs_rank = shape.rank %rhs_shape : !shape.shape -> !shape.size
  %w1 = shape.cstr_eq %lhs_rank, %rhs_rank : !shape.witness
  %res = shape.assuming %w1 -> tensor<?xindex> {
    %r1 = shape.any %lhs_rank, %rhs_rank : (!shape.size, !shape.size) -> !shape.size
    // Error message needs an addition, currently only on cstr_require.
    %w2 = shape.cstr_eq %c2, %r1, error="requires rank 2 operands"
    %res_1 = shape.assuming %w2 -> tensor<?xindex> {
      // Here the lowered
      //   %rank = shape.any %c2, %r1 (!shape.size, !shape.size) -> !shape.size
      // is dead and so elided further. But if `%rank` was actually consumed,
      // then it could have been folded in `shape.any`.
      %l0, %r0 = "shape.split_at"(%lhs_shape, %c1) :
        (!shape.shape, !shape.size) -> !shape.shape
      %l1, %r1 = "shape.split_at"(%lhs_shape, %c1) :
        (!shape.shape, !shape.size) -> !shape.shape
      %c = shape.meet %l1, %r0, error="inner dimensions required to match" :
        !shape.size, !shape.size -> !shape.size
      %res = concat(%l0, %r1)
      shape.assuming_yield %res
    }
    shape.assuming_yield %res_1
  }
  return %res : tensor<?xindex>
}
```

现在我们可以在可能的情况下提升约束的计算（在下面的例子中，提升的并不多，因为我们需要先验证 rank 才能做拆分）：

```mlir
func.func @matmul_shape2(%lhs: tensor<*xf32>, %lhs: tensor<*xf32>) -> tensor<?xindex> {
  %c1 = shape.const_size 1
  %c2 = shape.const_size 2
  %lhs_shape = shape.shape_of %lhs : tensor<*xf32> -> tensor<?xindex>
  %rhs_shape = shape.shape_of %rhs : tensor<*xf32> -> tensor<?xindex>
  %lhs_rank = shape.rank %lhs_shape : tensor<?xindex> -> tensor<index>
  %rhs_rank = shape.rank %rhs_shape : tensor<?xindex> -> tensor<index>
  %w1 = shape.cstr_eq %c2, %lhs_rank, error="requires rank 2 operands"
  %w2 = shape.cstr_eq %c2, %rhs_rank, error="requires rank 2 operands"
  %w = shape.assuming_all %w1, %w2
  %res = shape.assuming %w -> tensor<?xindex> {
    %l0, %r0 = "shape.split_at"(%lhs_shape, %c1) :
      (tensor<?xindex>, !shape.size) -> tensor<?xindex>
    %l1, %r1 = "shape.split_at"(%lhs_shape, %c1) :
      (tensor<?xindex>, !shape.size) -> tensor<?xindex>
    %w3 = shape.cstr_eq %l1, %r0, error="inner dimensions required to match"
    %res_2 = shape.assuming %w3 {
      %res = concat(%l0, %r1)
      shape.assuming_yield %res
    }
    shape.assuming_yield %res_1
  }
  return %res
}
```

以上形式现在可以被降级到完全命令式的形式（参见
[测试](https://github.com/tensorflow/mlir-hlo/blob/af14e1ded33c3164d4418c5d234b5b346b6d017c/tests/rank-specialization.mlir#L22)
示例）。

```mlir
func.func @matmul_shape3(%lhs: tensor<*xf32>, %lhs: tensor<*xf32>) -> tensor<?xindex> {
  %c1 = arith.constant 1 : index
  %c2 = arith.constant 2 : index
  %lhs_shape = shape.shape_of %lhs : tensor<*xf32> -> tensor<?xindex>
  %rhs_shape = shape.shape_of %rhs : tensor<*xf32> -> tensor<?xindex>
  %lhs_rank = shape.rank %lhs_shape : tensor<?xindex> -> tensor<index>
  %rhs_rank = shape.rank %rhs_shape : tensor<?xindex> -> tensor<index>
  %w1 = shape.shape_eq %lhs_rank, %rhs_rank
  %w2 = shape.shape_eq %c2, %lhs_rank
  %w3 = and %w1, %w2
  assert %w3, "requires rank 2 operands"
  %l0, %l1 = shape.split_at(%lhs_shape, %c1) : tensor<?xindex>
  %r0, %r1 = shape.split_at(%rhs_shape, %c1) : tensor<?xindex>
  %w4 = shape.eq %l1, %r0
  assert %w4, "inner dimensions required to match"
  %res = concat(%l0, %r1)
  return %res
}
```

*   在这个例子中，形式 3 与形式 1 同样简单且更为接近（但仅因为不需要重排）。因此，这是一个很好的问题：前端编写语言是否可以更接近命令式形式（正在讨论中）。
*   以上呈现的形式是降级过程中的中间形式。如果用作输入，我们需要限制对其的优化，因为 `shape` 方言的操作不再通过生产者-消费者（producer-consumer）关系连接，无法强制执行保护检查。

以上内容可以通过使用 `tensor.dim`、`tensor.from_elements` 等进一步降级（或者也可以通过 MHLO 或 TOSA 方言来降级）。

[^wip_form1]: 该形式在当前工作流中使用最少，仍需要更多工作。特别是在示例中，我们使用了 `shape_func`，而在代码中我们改用标准 func，因为形式 1 并未被明确使用。
