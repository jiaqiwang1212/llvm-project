# `Broadcastable` Trait

[TOC]

## 描述

`Broadcastable` trait 对操作强制执行以下属性：

- 操作至少有一个输入操作数。

- 操作恰好有一个结果。

- 所有输入操作数和结果的类型均为 `tensor` 或 `vector`。

- 形状推断机制能够仅根据输入操作数的形状计算结果形状。

- 根据下述验证规则，输入操作数具有广播兼容的形状。

- 根据下述验证规则，操作的结果形状与从其输入操作数推断出的形状兼容（但不一定相同）。


## 维度推断

给定一个具有两个输入操作数的操作，其结果第 `i` 维的大小可以根据操作数的第 `i` 维按下表推断。其中，`dim0` 和 `dim1` 以可互换的顺序表示输入操作数的第 `i` 维，`inferredDim` 表示操作结果第 `i` 维的推断大小。维度分为三类：动态（"?"）、静态等于 1（"1"）以及静态大于 1（">1"）。


| `dim0` | `dim1` | `inferredDim` | 备注 |
| -------- | -------- | ------------- | ----- |
| ? | ? | ? | 若 `RuntimeSize(dim0)` 为 1，则将 `dim0` 广播到 `RuntimeSize(dim1)`。若 `RuntimeSize(dim1)` 为 1，则将 `dim1` 广播到 `RuntimeSize(dim0)`。如果两个运行时大小均大于 1 且不相等，则操作产生未定义行为。|
| ? | 1 | ? | 将 `dim1` 广播到 `RuntimeSize(dim0)`。|
| ? | >1 | `dim1` | 若 `RuntimeSize(dim0)` 为 1，则将 `dim0` 广播到 `dim1`。若 `RuntimeSize(dim0)` 大于 1 且不等于 `dim1`，则操作产生未定义行为。|
| 1 | 1 | 1 | |
| 1 | >1 | `dim1` | 将 `dim0` 广播到 `dim1`。|
| >1 | >1 | `dim0` | 若 `dim0` != `dim1`，操作验证器会产生编译时错误。|


以下伪函数是维度推断过程的形式化表示：

```python
InferDim(dim0, dim1):
	switch (dim0, dim1):
		case (?, ?):
		case (?, 1):
		case (1, 1):
		case (>1, ?):
		case (>1, 1):
			return dim0
		case (?, >1):
		case (1, ?):
		case (1, >1):
			return dim1
		case (>1, >1):
			ERROR_IF(dim0 != dim1)
			return dim0
```

## 形状推断

形状推断过程首先修正输入操作数的秩差异。通过在左侧添加大小为 1 的额外维度来扩展形状，直到达到所需的秩，如下所示：

```python
ExpandRank(shape, rank):
	while len(shape) < rank:
		shape.prepend(1)
```
		
给定两个有秩输入操作数的形状，通过均衡输入秩并推断各个维度来推断结果形状，如下所示：

```python
InferShape(shape0, shape1):

  # Equalize ranks
  rank = max(GetRank(shape0), GetRank(shape1))
  ExpandRank(shape0, rank)
  ExpandRank(shape1, rank)
	
  # Infer shape
  inferredShape = []
  for (dim0, dim1) in zip(shape0, shape1):
    inferredDim = InferDim(dim0, dim1)
    inferredShape.append(inferredDim)
  return inferredShape
```
	
对于具有任意数量输入操作数的操作，结果形状的推断过程是：舍弃无秩操作数，对第一对有秩操作数应用形状推断，然后用每个额外的有秩操作数更新推断形状。如果操作没有有秩操作数，则无法推断结果形状。如果操作恰好有一个有秩操作数，则其形状直接作为推断的结果形状。形式化表示如下：

```python
InferResultShape(op):

	# Filter ranked operands
	rankedOperands = filter(op.operands, IsRanked)
	if len(rankedOperands) == 0:
		return None
	
	# Infer result shape
	inferredShape = GetShape(rankedOperands[0])
	for operand in rankedOperands[1:]:
		inferredShape = InferShape(inferredShape, GetShape(operand))
	return inferredShape
```

## 验证

带有 `Broadcastable` trait 的操作合法性验证首先运行形状推断过程。如果形状推断过程中发生失败，则判定输入操作数不具有广播兼容性，验证失败。如果形状推断成功，则验证继续。

如果结果无秩或所有输入操作数均无秩，则无需进一步验证步骤，过程在此成功结束。反之，如果结果和至少一个输入操作数均有秩，则验证继续，检查先前推断形状与结果之间的秩是否匹配。

一旦保证秩匹配，推断形状的每个维度将与实际结果形状对应维度按下表进行比较：


| `inferredDim` | `actualDim` | 验证结果 |
| ------------- | ----------- | -------------------- |
| ? | ? | **通过** |
| ? | static | **通过** <br> 若无法保证结果的运行时维度大小等于 `actualDim`，则会产生未定义行为。虽然不常见，但这种隐式的动态到静态转换在某些场景中很方便，例如形状推断 pass 的中间状态。最终，结果中的静态维度意味着所有输入维度大小也可在编译时已知，因此最好也将其设为静态。|
| static | ? | **通过** <br> 即使可以在编译时推断出静态大小，实际结果维度也可以是动态的。程序员可以选择放宽结果维度的具体性以实现结果类型的前向兼容。|
| static | static | **相等时通过** <br> 当推断维度和实际维度都是静态时，它们必须设置为相同的大小。|


完整的验证过程可以形式化规范如下：

```python
Verify(op):

	# Run shape inference
	inferredShape = InferResultShape(op.operands)

	# Done if result is unranked or all operands are unranked
	if not IsRanked(op.result) or inferredShape is None:
		return
	
	# Rank must match
	actualShape = GetShape(op.result):
	ERROR_IF(len(inferredShape) != len(actualShape))
	
	# Verify
	for (inferredDim, actualDim) in zip(inferredShape, actualShape):
		ERROR_IF(IsStatic(actualDim) and inferredDim != actualDim)
```
		
## 示例

以下是广播操作的正确用法：

```mlir
// Exact match of static sizes.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<1x2xi32>, tensor<1x2xi32) -> tensor<1x2xi32>

// Dynamic sizes match. The programmer must guarantee that the runtime sizes of
// %arg0 and %arg1 are equal at runtime.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<?xi32>, tensor<?xi32) -> tensor<?xi32>

// The shape of %arg0 is broadcast from tensor<1xi32> to tensor<4xi32>.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<1xi32>, tensor<4xi32) -> tensor<4xi32>

// The shape of %result is inferred as tensor<4xi32>, while the actual result
// type is tensor<?xi32>. The inferred shape is compatible with the actual shape.
%result = "test.broadcastable"(%arg0) : (tensor<4xi32) -> tensor<?xi32>

// The shape of %arg0 is first expanded to tensor<1x1x4xi32> and then broadcast
// to tensor<2x3x4xi32>.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<4xi32>, tensor<2x3x4xi32) -> tensor<2x3x4xi32>

// Input and results tensors have different element types (i1, i32, i64). The
// 'Broadcastable' trait has no restrictions on element types.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<2xi1>, tensor<2xi32) -> tensor<2xi64>

// No result shape verification is needed when the result is unranked.
%result = "test.broadcastable"(%arg0) : (tensor<2xi32>) -> tensor<*xi32>

// No result shape verification needed when all inputs are unranked.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<*xi32>, tensor<*xi32>) -> tensor<2xi32>
```


以下是广播操作的错误用法：

```mlir
// Dimension 0 of input operands is static but not equal.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<3xi32>, tensor<2xi32) -> tensor<?xi32>

// The inferred result shape is tensor<3xi32>, but the actual result shape is
// tensor<1x3xi32>. Inferred and actual shapes differ in rank.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<3xi32>, tensor<3xi32) -> tensor<1x3xi32>

// The inferred result shape is tensor<?xi32>, but the actual shape is
// tensor<4xi32>. The inferred shape is not compatible with the actual shape.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<?xi32>, tensor<?xi32) -> tensor<4xi32>

// The inferred result shape is tensor<2xi32>, but the actual result shape is
// tensor<4xi32>, which is not compatible.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<2xi32>, tensor<2xi32) -> tensor<4xi32>

// The inferred result shape is tensor<1xi32>, but the actual result shape is
// tensor<4xi32>. Broadcast semantics are not applicable for results.
%result = "test.broadcastable"(%arg0, %arg1) : (tensor<1xi32>, tensor<1xi32) -> tensor<4xi32>
```


