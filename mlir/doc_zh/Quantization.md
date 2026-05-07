# 量化

本文档概述了 MLIR 量化系统的设计。虽然"量化"一词高度重载，但在本文中，它指的是一套相当狭窄的技术，用于将浮点计算转换为以整数数学表示的对应可行变体，以用于推理，这一技术历史上一直受到低位深度推理引擎（如 TFLite、各种加速器硬件和许多 DSP）的支持。

本文大量借鉴了[这篇论文](https://arxiv.org/abs/1712.05877)中的方法，并融入了许多扩展和改进。它具体记录了 MLIR 在该主题上所采取的立场，并非通用参考资料。

[TOC]

## 均匀量化

MLIR 支持的主要量化机制是一种方案，它可以通过[实数](https://en.wikipedia.org/wiki/Real_number)数轴上均匀间隔的点来表达定点和仿射变换。

此外，该方案可以按以下方式应用：

*   *逐层（per-layer）*：应用于目标类型中的每个值。
*   *逐轴（per-axis）*（也称为*逐通道（per-channel）*）：沿张量类型的特定轴对每个索引单独应用。

### 定点值

[定点](https://en.wikipedia.org/wiki/Fixed-point_arithmetic)值是一个[实数](https://en.wikipedia.org/wiki/Real_number)除以*比例（scale）*的结果。我们将除后的实数结果称为*比例值（scaled value）*。

$$ real\\_value = scaled\\_value * scale $$

比例可以理解为相邻比例值之间的距离（以实数单位表示）。例如，如果比例为 $ \pi $，则具有此比例的定点值只能表示 $ \pi $ 的倍数，而不能表示其间的任何值。将任意实数转换为具有给定 $ scale $ 的定点值的最大舍入误差为 $ \frac{scale}{2} $。继续前面的例子，当 $ scale = \pi $ 时，最大舍入误差将为 $ \frac{\pi}{2} $。

可以对具有不同比例的比例值执行乘法，使用与实数值乘法相同的算法（注意乘积比例值的 $ scale_{product} = scale_{left \mbox{ } operand} * scale_{right \mbox{ } operand} $）。只要比例值具有相同的比例，就可以使用与实数值加法相同的算法对它们执行加法。这使得将比例值表示为有符号整数并对这些有符号整数执行算术运算变得方便，因为结果将是正确的比例值。

### 仿射值

从数学角度来说，仿射值是[将实数值*零点（zero point）*加到比例值](https://en.wikipedia.org/wiki/Affine_transformation#Representation)的结果。或者（等价地），从仿射值中减去零点得到比例值：

$$ real\\_value = scaled\\_value * scale = (affine\\_value - zero\\_point) * scale $$

本质上，仿射值是比例值按某个常量偏移的结果。一般来说，不能直接对仿射值执行算术运算（即加法、减法、乘法、除法）；必须首先将它们[转换](#affine-to-fixed-point)为等价的比例值。

如上所述，使用仿射值的动机是更有效地表示在计算过程中实际遇到的实数值。通常，在计算过程中遇到的实数值在实数零点附近并不对称。我们还假设在计算过程中会遇到实数零，因此应该能够表示它。

在这种情况下，将比例值存储为有符号整数是低效的，因为某些有符号整数将永远不会被使用。实际上，对应于这些有符号整数的位模式被浪费了。

为了用整数值的仿射值精确表示实数零，零点必须是介于最小和最大仿射值之间（含端点）的整数。例如，给定由 8 位无符号整数表示的仿射值，我们有：$ 0 \leq zero\\_point \leq 255 $。这很重要，因为在深度神经网络的卷积类操作中，我们经常需要对输入和输出进行零填充，所以零必须是精确可表示的，否则结果将有偏差。

### 关系

实数值、定点值和仿射值通过以下方程相互关联，该方程演示了如何将一种类型的数字转换为另一种类型：

$$ real\\_value = scaled\\_value * scale = (affine\\_value - zero\\_point) * scale $$

注意，计算机通常使用有限数量的位来存储数学值。因此，虽然上述转换是精确的，但为了将结果存储在有限数量的位中，通常必须对转换结果进行舍入（这适用于两种情况：使用浮点存储和使用定点存储）。注意，对舍入行为的完整讨论超出了本文档的范围，除非另有说明，否则可以安全地假设舍入应按照 IEEE754 默认的 RNE 方式进行（在硬件允许的情况下）。

### 实数与定点或仿射之间的转换

要将实数值转换为定点值，必须知道比例。要将实数值转换为仿射值，必须知道比例和零点。

#### 实数转仿射

要将实数元素的输入张量（通常由浮点格式表示，通常是[单精度](https://en.wikipedia.org/wiki/Single-precision_floating-point_format)）转换为由整数类型（例如 8 位无符号整数）表示的仿射元素张量，可以执行以下转换（注意，不要求使用整数类型的所有可表示值）：

$$
\begin{align*}
af&fine\\\_value \\\\
  &= clampToTargetSize(roundToNearestInteger( \frac{real\\\_value}{scale}) + zero\\\_point \\\\
\end{align*}
$$

其中我们假设以下类型：

- `real_value`: 单精度浮点
- `scale`: 单精度浮点
- `roundToNearestInteger`: 返回 32 位整数
- `zero_point`: 8 位或 16 位整数
- `affine_value`: 8 位或 16 位整数

注意，位深度和定点值的数量是指典型硬件上常见类型的代表，并不局限于特定的位深度，也不要求使用 N 位整数的整个范围。

#### 仿射转实数

要将由 uint8 或 uint16 表示的仿射元素输出张量转换为实数元素张量（通常由浮点格式表示，通常是单精度），可以执行以下转换：

$$
\begin{align*}
re&al\\\_value \\\\
      &= roundToNearestFloat(affine\\\_value - zero\\\_point) * scale
\end{align*}
$$

其中我们假设以下类型：

- `real_value`: 单精度浮点
- `scale`: 单精度浮点
- `affine_value`: 8 位或 16 位整数
- `zero_point`: 8 位或 16 位整数
- `roundToNearestFloat`: 返回单精度浮点
- `-`（减法）：返回 32 位有符号整数

#### 仿射转定点

当仿射和定点比例相同时，从仿射值中减去零点以获得等价的定点值。

$$
\begin{align*}
  scaled\\\_value = affine\\\_value_{non\mbox{-}negative} - zero\\\_point_{non\mbox{-}negative}
\end{align*}
$$

#### 定点转仿射

当仿射和定点比例相同时，将零点加到定点值以获得等价的仿射值。

$$
\begin{align*}
  affine\\\_value_{non\mbox{-}negative} = scaled\\\_value + zero\\\_point_{non\mbox{-}negative}
\end{align*}
$$

## 在 MLIR 中的使用

MLIR 中正在开发的量化系统有几个组成部分：

*   *量化（Quantization）* dialect 包含：

    *   一系列[量化类型（QuantizedTypes）](#quantized-type)，表示*表达*值（通常是浮点计算机类型）和*存储*值（通常是整数计算机类型）之间的映射。
    *   用于在基于 QuantizedType 的类型及其*表达*和*存储*子类型之间进行转换的[类型转换操作](#quantized-type-conversion-operations)。
    *   用于在计算中分配检测点的[检测操作](#instrumentation-and-constraint-operations)，运行时统计数据可以在这些点处帮助指导量化过程。

*   [与训练时模拟量化的集成](#integration-with-simulated-quantization-at-training-time)

*   [TFLite 原生量化](#tflite-native-quantization)

    *   TFLite 操作集原生支持均匀量化变体。
    *   存在用于直接从 *TensorFlow* dialect 转换到 TFLite 量化操作集的 Pass 和工具。

并非每种量化应用都会使用所有这些设施。具体来说，TensorFlow 到 TensorFlow Lite 的转换使用了 QuantizedTypes，但有自己的操作用于类型转换和支持数学的表达。

## 量化 Dialect

### 量化类型

TODO: 充实本节。

*   QuantizedType 基类
*   UniformQuantizedType

### 量化类型转换操作

*   qcast：从表达类型转换为 QuantizedType
*   dcast：从 QuantizedType 转换为其表达类型
*   scast：在 QuantizedType 及其存储类型之间转换

### 检测和约束操作

*   const_fake_quant：模拟历史 TensorFlow fake_quant_with_min_max_args 操作的逻辑。
*   stats_ref：声明应在此点以唯一键收集统计信息，并使其可用于求解器的未来 pass。
*   stats：声明计算中某点的内联统计信息（逐层和逐轴）。stats_ref 操作通常在执行试运行后转换为统计操作。
*   coupled_ref：基于唯一键，从类型推断的角度声明计算中要耦合的点。

## 与训练时模拟量化的集成

TensorFlow 历史上使用
[tf.quantization.fake_quant_\*](https://www.tensorflow.org/api_docs/python/tf/quantization/fake_quant_with_min_max_args)
系列操作来模拟训练时量化的效果。

最初实现时，TensorFlow Lite 是此类操作在推理时的主要用户。启用量化推理时，如果每个符合条件的张量都通过适当的 fake_quant 节点（哪些张量可以应用 fake_quant 的规则有些复杂），那么 TensorFlow Lite 将使用 fake_quant 操作的属性来判断如何转换以使用其量化操作子集中的内核。

在基于 MLIR 的量化中，fake_quant_\* 操作通过将它们转换为 *qcast*（量化）后跟 *dcast*（反量化）的序列来处理，以适当的 *UniformQuantizedType* 作为 qcast 操作的目标。

这允许后续的编译器 pass 保留量化以某种方式被模拟的知识，同时给编译器灵活性，在简化计算并将其转换为基于整数算术的形式时移动转换。

这种方案也自然地允许*部分量化*的计算，其中无法简化为整数操作的部分仍然在浮点中执行，并在边界处进行适当的转换。

## TFLite 原生量化

TODO: 充实本节

### 通用算法

1.  获取输入的最小/最大信息并设置 ArrayInfo（实际上是 InputOrOutputArrayInfo）。
1.  在 LegalizeTF 中，将 ArrayInfo 的最小/最大转换为 tf.Quantize 和 tf.Dequantize 节点（或 tf.FakeQuant）。将所有常量 FakeQuants 转换为（tf.FQ -> tfl.Q -> tfl.DQ）。
1.  硬编码逻辑/传播需要在此处进行。
1.  运行 TF 常量折叠。
1.  在 PrepareTFL 中，将所有 tf.FQ 转换为（tfl.Q -> tfl.DQ）。
1.  运行量化 pass，将（tfl.DQ（用于输入和权重）-> op -> tfl.Q）替换为（op）。同时将（constant_float -> tfl.Q）替换为（constant_quant）。
