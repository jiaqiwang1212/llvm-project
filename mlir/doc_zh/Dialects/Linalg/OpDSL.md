# Linalg OpDSL

**_警告：Linalg 的 OpDSL 目前正在被[弃用](https://discourse.llvm.org/t/how-to-add-custom-linalg-named-ops-using-opdsl/83200/2)，其操作正在缓慢地[迁移](https://github.com/llvm/llvm-project/pull/115319)到 TableGen 的 ODS 格式中。请参阅 [MLIR 重组讨论](https://discourse.llvm.org/t/rfc-mlir-project-charter-and-restructuring/82896)以获取更多深入信息。_**

基于 Python 的 DSL，用于编写 Linalg 操作定义并基于它们为样本生成 `linalg.generic` IR。

Linalg OpDSL 是一种高层 DSL，用于以可导出到内置命名结构化操作（通过 [YAML 定义](_index.md/#yaml-gen)）或以交互方式发射相应 `linalg.generic` IR 来构建结构化操作定义。

## 基本用法

该工具与 MLIR Python 绑定捆绑在一起。要从 CMake 构建树中使用，必须在启用 Python 绑定（`-DMLIR_ENABLE_BINDINGS_PYTHON=ON`）的情况下构建 MLIR。然后将构建树中的 `python` 目录添加到 `PYTHONPATH` 环境变量中（即 `export PYTHONPATH=$PWD/build/tools/mlir/python_packages/mlir_core`）。或者，如果有已安装的 MLIR 包，也可使用它以避免构建。

```shell
# Dump the `core_named_ops.py` module as YAML.
python -m mlir.dialects.linalg.opdsl.dump_oplib.ops.core_named_ops
```

或者，运行 `$PWD/build/bin/update_core_linalg_named_ops.sh` 脚本，该脚本在构建 `mlir-linalg-ods-yaml-gen` 目标后可用。该工具适合在开发和运行时使用，但不作为核心编译器的构建工具：为了将静态命名操作定义导出并作为编译器的一部分构建，必须更新并审查相应的 Linalg 方言 YAML 文件。TODO：开发一个脚本来自动化这些文件的操作更新。

## 语言指南

这里介绍的语言受 [Tensor Comprehensions](https://arxiv.org/pdf/1802.04730.pdf) 工作的粗略启发，并进行了调整以表示 linalg 结构化操作。

该工具是新的，正在快速演进中。关于语言示例，请参阅 `mlir.tools.linalg_opdsl.ops` 包中的内置操作（仓库中的 `lib/Bindings/Python/mlir/tools/linalg_opdsl/ops`）。

以矩阵乘法为例，我们来分解这门语言：

```python
T1 = TV.T1
T2 = TV.T2

@linalg_structured_op
def matmul(A=TensorDef(T1, S.M, S.K),
           B=TensorDef(T2, S.K, S.N),
           C=TensorDef(U, S.M, S.N, output=True)):
  """Performs a matrix multiplication of two 2D inputs.

  Numeric casting is performed on the operands to the inner multiply, promoting
  them to the same data type as the accumulator/output.
  """
  domain(D.m, D.n, D.k)
  defines(Canonicalizer)
  implements(ContractionOpInterface)
  C[D.m, D.n] += TypeFn.cast_signed(
      U, A[D.m, D.k]) * TypeFn.cast_signed(U, B[D.k, D.n])
```

这里有一个简单的类型多态收缩，接受参数 `A` 和 `B` 并输出 `C`。每个参数都绑定到一个 `TensorDef`，它指定了：

*   符号元素类型（上面的 `T1`、`T2`、`U`）。
*   带有全局绑定到操作的符号的符号形状表达式（注意在这个简单示例中，形状表达式只是符号引用，但它们被允许是受约束的仿射表达式集合）。
*   用途（`output=True`）。

文档字符串将被原样传递到操作定义中。

可以通过 `domain(D.d0[, D.d1...])` 为操作声明显式的迭代域维度顺序。

可以通过 `implements(interface1[, interface2...])` 为操作声明特殊的标识操作接口。

可以通过 `defines(definition1[, definition2...])` 为操作声明额外的方法定义。

## 参数

结构化操作接受两种类型的运行时参数，即标量和张量。标量仅作为输入，而张量可以被标记为输出。赋值表达式对张量参数进行索引以访问各个元素，而标量可以直接访问。

以下示例演示了两种参数类型的使用：

```python
@linalg_structured_op
def copy_and_scale(val=ScalarDef(T),
                   I=TensorDef(T, S.M, S.K),
                   O=TensorDef(T, S.M, S.K, output=True)):
  """Scale the input by the scalar value and store the result"""
  O[D.m, D.n] = I[D.m, D.n] * val
```

该操作将输入张量 `I` 的元素按标量值 `val` 缩放，并将结果写入输出张量 `out`。标量 `val` 绑定到一个 `ScalarDef`，它指定了标量操作数的类型。张量绑定到如矩阵乘法示例所示的 `TensorDef`。所有参数出现在操作的参数列表中：

```python
copy_and_scale(val, in_tensor, outs=[out_tensor])
```

## 索引属性

索引属性是仅在索引表达式中可访问的编译期常量参数。它们可以用来参数化结构化操作的访问模式，例如通过设置其步幅。它们不能参与实际计算。

以下示例演示了索引属性的使用：

```python
@linalg_structured_op
def strided_copy(I=TensorDef(T, S.IH, S.IW),
                 O=TensorDef(T, S.OH, S.OW, output=True),
                 strides=IndexAttrDef(S.SH, S.SW, default=[1, 1])):
  """Copy a subset of the input tensor elements to the output tensor"""
  O[D.oh, D.ow] = I[D.oh * S.SH, D.ow * S.SW]
```

该操作实现从输入张量 `I` 到输出张量 `O` 的步幅拷贝。`strides` 属性绑定到一个 `IndexAttrDef`。它定义了符号 `S.SH` 和 `S.SW`，这些符号用于对输入张量 `I` 进行索引。在实例化操作时，该属性使用命名参数设置：

```python
strided_copy(in_tensor, outs=[out_tensor], strides=[1, 2])
```

`strides` 向量元素替换操作实例索引表达式中的符号 `S.SH` 和 `S.SW`。如果未提供步幅，则使用 `default` 向量元素代替。

索引属性目前仅限于整数向量，只能在索引表达式中访问。一个操作可以有多个属性，所有属性都放在输出张量之后的参数列表末尾。

## 仅形状张量

结构化操作根据输入和输出张量的大小推导迭代空间。某些操作需要仅形状张量，这些张量不被访问，纯粹为了指定迭代域而存在。一个例子是池化操作，它接受一个仅形状张量来定义归约的迭代空间。由于仅形状张量没有使用，`TensorDef` 接受一个额外的可选 `index_dims` 参数来将形状映射到索引维度。

以下示例演示了索引维度注解：

```python
@linalg_structured_op
def pooling_poly(
    I=TensorDef(T1, S.N, S.H, S.W, S.C),
    K=TensorDef(T2, S.KH, S.KW, index_dims=[D.kh, D.kw]),
    O=TensorDef(U, S.N, S.OH, S.OW, S.C, output=True),
    strides=IndexAttrDef(S.SH, S.SW, default=[1, 1]),
    dilations=IndexAttrDef(S.DH, S.DW, default=[1, 1])):
  O[D.n, D.oh, D.ow, D.c] += TypeFn.cast_signed(U,
          I[D.n, D.oh * S.SH + D.kh * S.DH, D.ow * S.SW + D.kw * S.DW, D.c])
```

池化操作不访问仅形状张量 `K`。相反，形状 `S.KH` 和 `S.KW` 指定了归约维度 `D.kh` 和 `D.kw` 的迭代域。

## 赋值

语言的主体由上述形式的赋值表达式组成。迭代维度顺序根据在表达式中遇到的顺序按词法确定（如果使用数学运算符，则遵循运算符优先级）。TODO：引入一个指令来固定维度绑定。

归约维度被推断为 RHS 上不在 LHS 上的任何维度。

支持多种一元和二元算术函数：

*   `BinaryFn.add(a, b)`（也可通过重载二元 `+` 运算符）
*   `BinaryFn.mul(a, b)`（也可通过重载二元 `*` 运算符）
*   `BinaryFn.max_signed(a, b)`
*   `BinaryFn.min_signed(a, b)`
*   `BinaryFn.sub(a, b)`（也可通过重载二元 `-` 运算符）
*   `BinaryFn.max_unsigned(a, b)`
*   `BinaryFn.min_unsigned(a, b)`
*   `UnaryFn.exp(a)`
*   `UnaryFn.log(a)`

由于整数类型是无符号性的，符号性通过将整数视为有符号或无符号值的不同函数来实现。

归约中支持算术函数的一个子集。这些归约函数可以作为 RHS 上最外层的函数出现：

*   `ReduceFn.add`（也可重载 LHS 上的原地 `+=`）
*   `ReduceFn.mul`
*   `ReduceFn.max_signed`
*   `ReduceFn.min_signed`
*   `ReduceFn.max_unsigned`
*   `ReduceFn.min_unsigned`

由于整数类型是无符号性的，符号性通过将整数视为有符号或无符号值的不同函数来实现。

此外，类型转换函数将操作数转换为目标类型：

*   `TypeFn.cast_signed(TypeVar, operand)`
*   `TypeFn.cast_unsigned(TypeVar, operand)`

由于整数类型是无符号性的，符号性通过将整数视为有符号（`TypeFn.cast_signed`）或无符号（`TypeFn.cast_unsigned`）值的不同函数来实现。

还有一些特殊形式：

*   `const(value)` 返回一个常量值。
*   `index(dim)` 返回给定维度 `dim` 中的迭代索引。

## 函数属性

函数属性是编译期常量函数参数。它们可以用来参数化结构化操作执行的计算，例如支持有符号和无符号计算。

以下示例演示了函数属性的使用：

```python
@linalg_structured_op
def elemwise_binary(
    lhs=TensorDef(T1),
    rhs=TensorDef(T2),
    O=TensorDef(U, output=True),
    fun=BinaryFnAttrDef(default=BinaryFn.add),
    cast=TypeFnAttrDef(default=TypeFn.cast_signed)):
  O[None] = fun(cast(U, lhs[None]), cast(U, rhs[None]))
```

`fun` 和 `cast` 函数属性默认情况下分别是其默认值 `BinaryFn.add` 和 `TypeFn.cast_signed` 的别名。在实例化操作时，可以使用可选的命名参数将函数属性设置为其他函数：

```python
elemwise_binary(lhs, rhs, outs=[out_tensor],
                fun=BinaryFn.mul, cast=TypeFn.cast_unsigned)
```

在示例中，`fun` 和 `cast` 参数调整操作的主体，以实现乘法和无符号类型转换，而不是加法和有符号类型转换。

OpDSL 支持一元、二元和类型转换函数属性。一个操作可以接受多个不同类型的属性，所有属性都放在参数列表末尾。

## 类型

赋值表达式中的所有类型都根据已构造操作的实际输入和输出类型进行后期绑定。例外情况是预定义类型，例如 `I32`、`I64`、`F32` 和 `F64`。这些硬编码类型支持使用独立于输入和输出类型的类型进行中间计算。例如，浮点计算的某些部分可能需要双精度算术，尽管所有输入和输出都是单精度值。没有 `TypeFn.cast_signed` 调用的赋值表达式通常要求整个过程中类型一致，如果违反则验证失败。`TypeFn.cast_signed` 或 `TypeFn.cast_unsigned` 的存在允许有限形式的数值类型转换，转换于可以从输入和输出（以及未来的属性）推导出的元素类型之间进行。带有 `TypeVar` 第一个参数的 `TypeFn.cast_signed` 调用在 YAML 定义中作为 `type_fn` 原语发射。

类型转换将执行 `int<->float` 和 `index->int` 类型转换，并在类型族内执行必要的扩展或截断。整数类型本身是无符号性的，符号性通过函数/操作实现。`TypeFn.cast_signed` 函数将所有整数视为有符号，而 `TypeFn.cast_unsigned` 将其视为无符号。

以下示例说明了有符号和无符号函数的降低：

*   cast_signed(I32 -> I64) -> `arith.ExtSIOp`
*   cast_signed(F32 -> I32) -> `arith.FPToSIOp`
*   cast_unsigned(I32 -> I64) -> `arith.ExtUIOp`
*   cast_unsigned(F32 -> I32) -> `arith.FPToUIOp`
*   max_signed -> `arith.MaxSIOp`
*   max_unsigned -> `arith.MaxUIOp`

并非所有函数都适用于所有数值类型，若不匹配，操作验证将失败。

## 逐点计算

逐点计算可以以秩多态的形式表达，支持任意秩的操作数——所有操作数需要具有相同的秩——使用单个操作定义。

秩多态操作的一个示例是 `fill`：

```python
@linalg_structured_op
def fill(value=ScalarDef(T),
         O=TensorDef(T, output=True)):
  O[None] = value
```

该操作将输出张量 `O` 的元素设置为 `value`。值的类型必须与输出张量的元素类型匹配。所有操作数要么是标量，要么是使用索引 `None` 访问的秩为零的张量。因此，该操作执行一个标量计算，可以平凡地扩展到多维逐点计算。因此，我们可以对任意秩的输出张量使用 `fill`：

```python
tensor_2d = tensor.EmptyOp([4, 8], f32)
tensor_3d = tensor.EmptyOp([4, 8, 16], f32)
fill(value, outs=[tensor_2d])
fill(value, outs=[tensor_3d])
```
