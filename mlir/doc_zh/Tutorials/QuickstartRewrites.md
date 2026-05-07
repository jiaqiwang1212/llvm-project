# 添加 MLIR 图重写的快速入门教程

本文档将介绍添加图重写的快速入门。我们将从定义一个操作开始，展示使用模式定义重写的多种方式，以及使用图遍历器定义重写（注意：首选使用模式和重写引擎，展示遍历器只是为了演示目的）。

有关 MLIR、IR 结构、操作等的更多信息，请参阅 [MLIR 规范](../LangRef.md)。有关以表格驱动方式定义操作和重写的所有可用机制的详细说明，请参阅[表格驱动操作定义](../DefiningDialects/Operations.md)和[声明式重写规则](../DeclarativeRewrites.md)。

## 添加操作

MLIR 中的操作使用 [TableGen](https://llvm.org/docs/TableGen/index.html) 文件中的定义来指定。TableGen 是一种建模工具，用于指定操作，并从中生成与这些操作交互的 C++ 代码。要定义一个操作，需要指定：

*   操作名称。该名称是 MLIR 中操作的唯一标识符。大多数操作位于某个方言中，例如可以用 `tfl.add` 来表示 TensorFlow Lite 方言中的加法操作。为了避免在操作定义中重复方言，通常为操作方言创建一个基类，该基类根据操作名称添加方言命名空间前缀。
*   操作的特征（trait）。这些特征允许你指定操作的特性，例如它是否有副作用，或者是否应该验证操作数和结果类型相同。这些特征由执行验证的 C++ 特征支持。
*   操作的参数。这些是输入操作数（运行时由其他操作产生的值）和属性（影响操作行为的编译时已知常量值），它们是操作的输入/定义操作行为的内容。输入操作数可以是有名称的，属性必须是有名称的。
*   操作的结果。这些同样可以是有名称的或无名称的。
*   操作的文档。这包括一行摘要以及更长的人类可读描述。
*   方言特定信息。可以向操作定义中添加仅由方言特定驱动程序使用的额外信息。这些信息被主操作和文档生成器忽略，但可以用于从一种方言到另一种表示的转换中。

```tablegen
def TFL_LeakyReluOp: TFL_Op<TFL_Dialect, "leaky_relu",
                            [NoMemoryEffect, SameValueType]>,
                     Results<(outs Tensor)> {
  let arguments = (ins
    F32Tensor:$x,
    // 激活函数在 x < 0 时的斜率。
    F32Attr:$alpha
  );

  let summary = "Leaky ReLU operator";
  let description = [{
    Element-wise Leaky ReLU operator
      x -> x >= 0 ? x : (alpha * x)
  }];

  // TFLite 特定属性，用于生成输出 flatbuffer 时使用。
  let hasOptions = 1;
}
```

注意，在上面的例子中，结果类型和输入以不同的方式指定，一种通过特征，另一种通过 let。两种方式都可以用来指定这两者。

<!-- TODO: Define a style convention. -->

操作还可以有自定义的解析器、打印器、构建器、验证器、常量折叠器或规范化器。这些需要指定额外的 C++ 方法以调用以获得额外功能。例如，如果一个操作被标记为有折叠器，则还需要添加常量折叠器，例如：

```c++
OpFoldResult SpecificOp::fold(ArrayRef<Attribute> constOperands) {
  if (unable_to_fold)
    return {};
  ....
  return val;
}
```

## 添加模式

MLIR 中可以执行多种形式的图重写。最常见的一种是 DAG 片段到 DAG 片段的重写。模式提供了一种简洁的方式来表达此变换，即一对要匹配的源模式和结果模式。既有 C++ 类来表示此变换，也有 TableGen 中的模式可以从中生成。

### TableGen 模式

让我们继续以 LeakyRelu 为例。要从 TensorFlow 的 `LeakyRelu` 映射到 TensorFlow Lite 的 `LeakyRelu`：

```tablegen
def : Pat<(TF_LeakyReluOp $arg, F32Attr:$a), (TFL_LeakyReluOp $arg, $a)>
```

该模式通过实例化一个带有源和结果 DAG 的 `Pat` 来指定。源模式中的参数被捕获，并可用于结果模式。这是一个简单的模式，因为我们有 1:1 的映射，并且属性不需要转换（例如，两者都有一个浮点属性用于 alpha）。模式中指定的属性名称用于匹配/引用，不必与操作定义中的原始属性名称匹配，但 DAG 的参数顺序必须匹配。

要指定一个模式，源和结果操作都需要使用 TableGen 定义。

如果这是一个当前框架无法表达为目标的更高级模式，可以使用通用的本地代码回退方法。这包括定义一个模式以及添加一个 C++ 函数来执行替换：

```tablegen
def createTFLLeakyRelu : NativeCodeCall<
    "createTFLLeakyRelu($_builder, $0.getDefiningOp(), $1, $2)">;

def : Pat<(TF_LeakyReluOp:$old_value, $arg, F32Attr:$a),
          (createTFLLeakyRelu $old_value, $arg, $a)>;
```

```c++
static Value createTFLLeakyRelu(PatternRewriter &rewriter, Operation *op,
                                Value operand, Attribute attr) {
  return mlir::TFL::LeakyReluOp::create(rewriter,
      op->getLoc(), operands[0].getType(), /*arg=*/operands[0],
      /*alpha=*/cast<FloatAttr>(attrs[0]));
}
```

这允许任意复杂的构建器。在输入模式方面，可以表达带有输入操作数和属性约束的多操作模式。但输入模式目前还不能表达跨多个操作数/属性的约束。

### 注册模式

包含模式的文件需要在编译时使用 `mlir-tblgen` 的 `-gen-rewriters` 进行处理。可以使用以下 CMake 配置调用它：

```cmake
set(LLVM_TARGET_DEFINITIONS <name-of-the-td-file>)
mlir_tablegen(<name-of-the-generated-inc-file> -gen-rewriters)
add_public_tablegen_target(<name-of-the-cmake-target>)
```

然后可以在任何你喜欢的 C++ 实现文件中 `#include` 生成的文件。（你还需要确保库依赖于上面 CMake 目标中定义的目标。）生成的文件将有一个 `populateWithGenerated(RewritePatternSet &patterns)` 函数，你可以用它来收集 `patterns` 中所有生成的模式，然后在任何你想要的通道中使用 `patterns`。

### 简单的 C++ `matchAndRewrite` 风格规范

许多简单的重写可以用 `matchAndRewrite` 风格的模式来表达，例如将乘以 2 的幂次转换为移位操作。对于这些情况，可以将模式定义为简单函数：

```c++
static LogicalResult
convertTFLeakyRelu(TFLeakyReluOp op, PatternRewriter &rewriter) {
  rewriter.replaceOpWithNewOp<TFL::LeakyReluOp>(
      op, op->getResult(0).getType(), op->getOperand(0),
      /*alpha=*/op->getAttrOfType<FloatAttr>("alpha"));
  return success();
}

void populateRewrites(RewritePatternSet &patternSet) {
  // 将其添加到模式集中。
  patternSet.add(convertTFLeakyRelu);
}
```

ODS 提供了一种简单的方式为操作定义函数风格的规范化。在操作的 TableGen 定义中，指定 `let hasCanonicalizeMethod = 1;`，然后在 .cpp 文件中实现 `canonicalize` 方法：

```c++
// 来自 CIRCT 项目的示例，该项目有一个可变参数整数乘法。
LogicalResult circt::MulOp::canonicalize(MulOp op, PatternRewriter &rewriter) {
  auto inputs = op.inputs();
  APInt value;

  // mul(x, c) -> shl(x, log2(c))，其中 c 是 2 的幂次。
  if (inputs.size() == 2 && matchPattern(inputs.back(), m_RConstant(value)) &&
      value.isPowerOf2()) {
    auto shift = rtl::ConstantOp::create(rewriter, op.getLoc(), op.getType(),
                                                  value.exactLogBase2());
    auto shlOp =
        comb::ShlOp::create(rewriter, op.getLoc(), inputs[0], shift);
    rewriter.replaceOpWithNewOp<MulOp>(op, op.getType(),
                                       ArrayRef<Value>(shlOp));
    return success();
  }

  return failure();
}
```

但是，你可能希望规范化模式具有完全的通用性，为此可以指定任意的 `RewritePattern` 列表。

### 完全通用的 C++ `RewritePattern` 规范

如果 ODS 模式和 `matchAndRewrite` 风格函数不够用，也可以将重写指定为一组通用的 `RewritePattern`：

```c++
struct ConvertTFLeakyRelu : public RewritePattern {
  ConvertTFLeakyRelu(MLIRContext *context)
      : RewritePattern("tf.LeakyRelu", 1, context) {}

  LogicalResult matchAndRewrite(Operation *op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<TFL::LeakyReluOp>(
        op, op->getResult(0).getType(), op->getOperand(0),
        /*alpha=*/op->getAttrOfType<FloatAttr>("alpha"));
    return success();
  }
};
```

在 C++ 重写中，重写模式的静态收益在构造时指定。而在模式生成器中，目前采用一种简单的启发式方法，该方法基于匹配和替换的操作数量。

上面的规则没有捕获匹配的操作数/属性，但一般来说，多步重写中的 `match` 函数可以填充并返回一个 `PatternState`（或从中派生的类），以将匹配期间提取的信息传递给重写步骤。使用 `matchAndRewrite` 函数的单步重写的好处是可以直接使用匹配时创建的任何值，无需 `PatternState`。

## 测试

MLIR 使用 [lit](https://llvm.org/docs/CommandGuide/lit.html)（LLVM 集成测试）工具进行测试。测试通过创建输入 IR 文件、运行变换然后验证输出 IR 来执行。C++ 单元测试是例外，IR 变换作为核心测试机制。这减少了需要构建（和链接）的二进制文件数量，并强制将表示形式作为重要的关注点。

对于上面的合法化变换，我们将有一个测试（可能作为 TensorFlow Lite 中合法化通道测试的一部分），如下所示：

```mlir
// RUN: mlir-opt -tfl-legalize-tf %s | FileCheck %s

func.func @LeakyRelu(%arg0: tensor<1xf32>) -> tensor<1xf32> {
  %2 = "tf.LeakyRelu"(%arg0) {alpha: 0.1} : (tensor<1xf32>) -> tensor<1xf32>
  return %2: tensor<1xf32>

// CHECK-LABEL: LeakyRelu
// CHECK:  %0 = "tfl.leaky_relu"(%arg0) {alpha: 1.000000e-01} : (tensor<1xf32>) -> tensor<1xf32>
}
```

顶部的 RUN 命令会运行 `mlir-opt` 二进制文件（这是编译器开发者用于测试不同注册通道的工具），对当前文件调用该变换所属的优化通道，并使用 `FileCheck` 验证其输出。`FileCheck` 是一个文本输出验证器。具体地，它使用 CHECK 表达式来验证产生了给定的输出。

可以有多个 RUN 命令，每个命令对应不同的 CHECK 前缀。此外，还可以有多个独立的测试，用 `// -----` 分隔，并以 `-split-input-file` 标志调用 `mlir-opt`。这对于错误测试特别有用。

这产生了非常简单、有针对性的测试，无需绕过常量传播或其他不相关的优化通道。

## 添加优化通道

不适合/难以在上述结构中指定的优化通道，可以指定为对模块/函数的通用迭代。有关 MLIR 中优化通道的概述和介绍，请参阅[编写通道](../PassManagement.md)。
