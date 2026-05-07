# 第 4 章：通过接口实现通用变换

[TOC]

## 背景：应对可扩展 IR 的挑战

通过方言（dialect），MLIR 能够表示许多不同层次的抽象；我们之前定义的 Toy 方言就是其中一个例子。尽管这些不同的方言可能表示不同的抽象，但通常存在一组我们希望执行的通用变换和分析。问题在于，为每个方言单独实现每种变换会导致大量代码重复，因为内部算法通常非常相似甚至完全相同。我们希望提供一种能力，让变换可以不透明地钩入（hook into）像 Toy 这样的方言，以获取所需的信息。

MLIR 为某些核心变换提供了一组始终可用的钩子，如[上一章](Ch-3.md)所示，我们通过操作（operation）上的钩子（`getCanonicalizationPatterns`）注册了一些规范化模式。然而，这类钩子的扩展性并不好。因此，设计了一种更通用的解决方案——[接口（interfaces）](../../Interfaces.md)，使 MLIR 基础设施与其所表示的内容一样可扩展。接口为方言和操作提供了一种通用机制，用于向变换或分析提供信息。

## 形状推断：为代码生成做准备

我们的 Toy IR 目前操作的是泛型张量，这意味着除了在常量初始化时，我们并不知道张量的形状。这给优化和代码生成带来了麻烦。幸运的是，我们可以简单地在计算中传播形状，直到所有形状都已知。问题在于如何处理对用户自定义泛型函数的调用：每个调用点可能推导出不同的形状。一种可能性是基于参数类型进行符号推断，但如果我们在语言中引入更多控制流，这将难以泛化。另一种方法是函数特化（function specialization），即为每个具有新参数形状的调用点复制被调用函数并对其进行特化。我们在 Toy 中采用的方法是内联所有函数调用，然后在过程内（intraprocedural）执行形状传播。

### 内联（Inlining）

我们可以专门为 Toy 方言编写一个内联算法，但根据所需的复杂程度，这可能会相当复杂。不考虑代价模型，仅是纯结构变换从头实现就已经很复杂了。幸运的是，MLIR 提供了一个通用内联算法，方言可以接入其中。我们在 Toy 中所需要做的，就是为内联器提供可以钩入的[接口](../../Interfaces.md)。

首先，我们需要定义 Toy 方言中内联操作的约束条件。该信息通过[方言接口（dialect interface）](../../Interfaces.md/#dialect-interfaces)提供。这本质上是一个包含一组虚钩子（virtual hooks）的类，方言可以重写这些钩子。在本例中，该接口为 `DialectInlinerInterface`。

```c++
/// This class defines the interface for handling inlining with Toy operations.
/// We simplify inherit from the base interface class and override
/// the necessary methods.
struct ToyInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  /// This hook checks to see if the given callable operation is legal to inline
  /// into the given call. For Toy this hook can simply return true, as the Toy
  /// Call operation is always inlinable.
  bool isLegalToInline(Operation *call, Operation *callable,
                       bool wouldBeCloned) const final {
    return true;
  }

  /// This hook checks to see if the given operation is legal to inline into the
  /// given region. For Toy this hook can simply return true, as all Toy
  /// operations are inlinable.
  bool isLegalToInline(Operation *, Region *, bool,
                       IRMapping &) const final {
    return true;
  }

  /// This hook cheks if the given 'src' region can be inlined into the 'dest'
  /// region. The regions here are the bodies of the callable functions. For
  /// Toy, any function can be inlined, so we simply return true.
  bool isLegalToInline(Region *dest, Region *src, bool wouldBeCloned,
                       IRMapping &valueMapping) const final {
    return true;
  }

  /// This hook is called when a terminator operation has been inlined. The only
  /// terminator that we have in the Toy dialect is the return
  /// operation(toy.return). We handle the return by replacing the values
  /// previously returned by the call operation with the operands of the
  /// return.
  void handleTerminator(Operation *op,
                        ValueRange valuesToRepl) const final {
    // Only "toy.return" needs to be handled here.
    auto returnOp = cast<ReturnOp>(op);

    // Replace the values directly with the return operands.
    assert(returnOp.getNumOperands() == valuesToRepl.size());
    for (const auto &it : llvm::enumerate(returnOp.getOperands()))
      valuesToRepl[it.index()].replaceAllUsesWith(it.value());
  }
};
```

此外，内联器只会丢弃私有可见的未使用函数定义。我们还需要在 MLIR 生成器中设置函数的可见性（除主函数外）。

```c++
/// Emit a new function and add it to the MLIR module.
mlir::toy::FuncOp mlirGen(FunctionAST &funcAST) {
  ...
  // If this function isn't main, then set the visibility to private.
  if (funcAST.getProto()->getName() != "main")
    function.setPrivate();

  return function;
}
```

然后，我们直接在 Toy 方言上注册方言接口，方式与注册操作时相同。

```c++
void ToyDialect::initialize() {
  addInterfaces<ToyInlinerInterface>();
}
```

接下来，我们需要为内联器提供一种方式来识别 `toy.generic_call` 表示调用（call），而 `toy.func` 表示函数（function）。MLIR 提供了[操作接口（operation interfaces）](../../Interfaces.md/#attributeoperationtype-interfaces)，可用于将操作标记为"类调用（call-like）"或"类可调用（callable-like）"。与方言接口不同，操作接口提供了更精细的粒度信息，这些信息特定且核心于单个操作。我们将在这里添加的接口是 `CallOpInterface` 和 `CallableOpInterface`。

要添加此接口，我们只需将定义包含到操作规范文件（`Ops.td`）中：

```tablegen
include "mlir/Interfaces/CallInterfaces.td"
```

并将其添加到 `GenericCallOp` 的特性列表中：

```tablegen
def FuncOp : Toy_Op<"func",
    [FunctionOpInterface, IsolatedFromAbove]> {
  ...
}

def GenericCallOp : Toy_Op<"generic_call",
    [DeclareOpInterfaceMethods<CallOpInterface>]> {
  ...
}
```

在上面的代码中，我们还使用了 `DeclareOpInterfaceMethods` 指令，在 `GenericCallOp` 的类声明中自动声明所有接口方法。然而，对 `CallOpInterface` 使用该指令会包含处理参数和结果属性的方法。因此，我们需要将这些特定命名的属性（attribute）添加到 `GenericCallOp` 的定义中：

```tablegen
let arguments = (ins
  ...
  OptionalAttr<DictArrayAttr>:$arg_attrs,
  OptionalAttr<DictArrayAttr>:$res_attrs
);
```

我们已经在 `FuncOp` 类的 `extraClassDeclaration` 字段中提供了定义：

```c++
/// Returns the region on the function operation that is callable.
Region *FuncOp::getCallableRegion() { return &getBody(); }

// ....

/// Return the callee of the generic call operation, this is required by the
/// call interface.
CallInterfaceCallable GenericCallOp::getCallableForCallee() {
  return (*this)->getAttrOfType<SymbolRefAttr>("callee");
}

/// Set the callee for the generic call operation, this is required by the call
/// interface.
void GenericCallOp::setCalleeFromCallable(CallInterfaceCallable callee) {
  (*this)->setAttr("callee", callee.get<SymbolRefAttr>());
}

/// Get the argument operands to the called function, this is required by the
/// call interface.
Operation::operand_range GenericCallOp::getArgOperands() { return getInputs(); }

/// Get the argument operands to the called function as a mutable range, this is
/// required by the call interface.
MutableOperandRange GenericCallOp::getArgOperandsMutable() {
  return getInputsMutable();
}
```

既然内联器已经了解了 Toy 方言，我们可以将内联通道（inliner pass）添加到 Toy 的通道管理器中：

```c++
  pm.addPass(mlir::createInlinerPass());
```

现在让我们看一个实际的例子：

```mlir
toy.func @multiply_transpose(%arg0: tensor<*xf64>, %arg1: tensor<*xf64>) -> tensor<*xf64> {
  %0 = toy.transpose(%arg0 : tensor<*xf64>) to tensor<*xf64>
  %1 = toy.transpose(%arg1 : tensor<*xf64>) to tensor<*xf64>
  %2 = toy.mul %0, %1 : tensor<*xf64>
  toy.return %2 : tensor<*xf64>
}
toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %1 = toy.reshape(%0 : tensor<2x3xf64>) to tensor<2x3xf64>
  %2 = toy.constant dense<[1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00]> : tensor<6xf64>
  %3 = toy.reshape(%2 : tensor<6xf64>) to tensor<2x3xf64>
  %4 = toy.generic_call @multiply_transpose(%1, %3) : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64>
  %5 = toy.generic_call @multiply_transpose(%3, %1) : (tensor<2x3xf64>, tensor<2x3xf64>) -> tensor<*xf64>
  toy.print %5 : tensor<*xf64>
  toy.return
}
```

我们有两个对 multiply_transpose 的调用希望内联到 main 中，但如果观察输出，没有任何变化。我们缺少最后一个微妙的部分：在调用边界上存在一个隐式类型转换。观察上面的代码，`generic_call` 的操作数类型为 `tensor<2x3xf64>`，而函数的输入期望的是 `tensor<*xf64>`。为了解决这种差异，内联器需要插入一个显式的转换操作（cast operation）。为此，我们需要在 Toy 方言中添加一个新操作 `ToyCastOp`（toy.cast），用于表示两种不同形状之间的转换。

```tablegen
def CastOp : Toy_Op<"cast", [
    DeclareOpInterfaceMethods<CastOpInterface>,
    Pure,
    SameOperandsAndResultShape]
  > {
  let summary = "shape cast operation";
  let description = [{
    The "cast" operation converts a tensor from one type to an equivalent type
    without changing any data elements. The source and destination types
    must both be tensor types with the same element type. If both are ranked,
    then shape is required to match. The operation is invalid if converting
    to a mismatching constant dimension.
  }];

  let arguments = (ins F64Tensor:$input);
  let results = (outs F64Tensor:$output);
  let assemblyFormat = "$input attr-dict `:` type($input) `to` type($output)";
}
```

注意，该转换操作的定义在特性列表中添加了 `CastOpInterface`。该接口为类转换操作提供了多种实用工具，例如折叠恒等转换（identity casts）和验证。我们通过提供 `areCastCompatible` 方法的定义来钩入此接口：

```c++
/// Returns true if the given set of input and result types are compatible with
/// this cast operation. This is required by the `CastOpInterface` to verify
/// this operation and provide other additional utilities.
bool CastOp::areCastCompatible(TypeRange inputs, TypeRange outputs) {
  if (inputs.size() != 1 || outputs.size() != 1)
    return false;
  // The inputs must be Tensors with the same element type.
  TensorType input = llvm::dyn_cast<TensorType>(inputs.front());
  TensorType output = llvm::dyn_cast<TensorType>(outputs.front());
  if (!input || !output || input.getElementType() != output.getElementType())
    return false;
  // The shape is required to match if both types are ranked.
  return !input.hasRank() || !output.hasRank() || input == output;
}

```

有了合适的转换操作，我们现在可以重写 ToyInlinerInterface 上必要的钩子，让其在需要时自动插入转换：

```c++
struct ToyInlinerInterface : public DialectInlinerInterface {
  ...

  /// Attempts to materialize a conversion for a type mismatch between a call
  /// from this dialect, and a callable region. This method should generate an
  /// operation that takes 'input' as the only operand, and produces a single
  /// result of 'resultType'. If a conversion can not be generated, nullptr
  /// should be returned.
  Operation *materializeCallConversion(OpBuilder &builder, Value input,
                                       Type resultType,
                                       Location conversionLoc) const final {
    return CastOp::create(builder, conversionLoc, resultType, input);
  }
};
```

如果我们再次通过流水线运行这个示例，将得到预期的结果：

```mlir
toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %1 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %2 = toy.cast %1 : tensor<2x3xf64> to tensor<*xf64>
  %3 = toy.cast %0 : tensor<2x3xf64> to tensor<*xf64>
  %4 = toy.transpose(%2 : tensor<*xf64>) to tensor<*xf64>
  %5 = toy.transpose(%3 : tensor<*xf64>) to tensor<*xf64>
  %6 = toy.mul %4, %5 : tensor<*xf64>
  toy.print %6 : tensor<*xf64>
  toy.return
}
```

注意：通用内联器还会执行一些化简操作，因此输出可能比预期更加整洁。

### 过程内形状推断（Intraprocedural Shape Inference）

现在我们已经内联了所有函数，剩下的是一个包含静态和动态形状操作混合的 main 函数。我们现在可以编写一个简单的形状推断通道（shape inference pass）来在过程内（单个函数内）传播形状。我们可以将其编写为一个直接对 Toy 方言中操作约束进行编码的通道，但这似乎是一个可以泛化编写的良好候选变换。作为经验法则，最好尽可能泛化地表达变换，以便将来可以扩展到其他方言。没有人能预知有多少其他方言可能有类似需求或遇到相同问题。

对于形状推断，如果我们将问题分解到核心，我们真正想要的是让操作告诉我们：给定一组静态已知的输入，期望的输出是什么。（当然可以比这更复杂，但对于我们的需求，保持简单即可。）由于这个特性是特定操作的核心，我们可以定义一个操作接口，可以在需要推断结果形状的操作上指定该接口。

与操作类似，我们也可以使用操作定义规范（ODS）框架来[定义操作接口](../../Interfaces.md/#attributeoperationtype-interfaces)。

接口通过继承 `OpInterface` 来定义，`OpInterface` 以要生成的 C++ 接口类的名称作为模板参数。出于我们的目的，我们将生成的类简单地命名为 `ShapeInference`。我们还为接口提供了描述。

```tablegen
def ShapeInferenceOpInterface : OpInterface<"ShapeInference"> {
  let description = [{
    Interface to access a registered method to infer the return types for an
    operation that can be used during type inference.
  }];
}
```

接下来，我们定义操作需要提供的接口方法。接口方法由以下部分组成：描述、字符串形式的 C++ 返回类型、字符串形式的方法名称，以及根据需要的一些可选组件。更多信息请参阅 [ODS 文档](../../Interfaces.md/#attributeoperationtype-interfaces)。

```tablegen
def ShapeInferenceOpInterface : OpInterface<"ShapeInference"> {
  ...

  let methods = [
    InterfaceMethod<"Infer and set the output shape for the current operation.",
                    "void", "inferShapes">
  ];
}
```

既然接口已定义，我们可以以类似于向 GenericCallOp 添加 `CallOpInterface` 的方式，将其添加到必要的 Toy 操作中：

```tablegen
def MulOp : Toy_Op<"mul",
    [..., DeclareOpInterfaceMethods<ShapeInferenceOpInterface>]> {
  ...
}
```

每个操作都需要为 `inferShapes()` 方法提供定义。以 mul 操作为例，结果形状推断为输入的形状：

```c++
/// Infer the output shape of the MulOp, this is required by the shape inference
/// interface.
void MulOp::inferShapes() { getResult().setType(getLhs().getType()); }
```

至此，每个必要的 Toy 操作都提供了推断输出形状的机制。ShapeInferencePass 将作用于函数：它将对每个函数独立运行。MLIR 还支持通用的 [OperationPasses](../../PassManagement.md/#operation-pass)，可运行于任何隔离操作上，但我们的模块仅包含函数，因此无需泛化到所有操作。

实现此通道的方式是创建一个继承自 `mlir::OperationPass<FuncOp>` 的类，并重写 `runOnOperation()` 方法：

```c++
class ShapeInferencePass
    : public mlir::PassWrapper<ShapeInferencePass, OperationPass<FuncOp>> {
  void runOnOperation() override {
    FuncOp function = getOperation();
    ...
  }
};
```

同时，让我们创建一个用于实例化该通道的辅助方法：

```c++
std::unique_ptr<mlir::Pass> mlir::toy::createShapeInferencePass() {
  return std::make_unique<ShapeInferencePass>();
}
```

形状推断算法的运行流程如下：

1.  构建一个工作列表（worklist），包含所有返回动态形状张量的操作：这些是需要形状推断的操作。
2.  迭代工作列表：
    -   找到一个待处理的操作：工作列表中下一个就绪的操作，其所有参数均为非泛型；
    -   如果没有找到操作，则跳出循环；
    -   从工作列表中移除该操作；
    -   根据参数类型推断其输出形状。
3.  如果工作列表为空，则算法成功。

在处理如上所述的操作时，我们使用以下代码片段查询操作是否注册了 `ShapeInference` 接口：

```c++
  // Ask the operation to infer its output shapes.
  LDBG() << "Inferring shape for: " << *op;

  /// We check if an operation has a particular interface by casting.
  if (ShapeInference shapeOp = dyn_cast<ShapeInference>(op)) {
    shapeOp.inferShapes();
  } else {
    op->emitError("unable to infer shape of operation without shape "
                  "inference interface");
    return signalPassFailure();
  }
```

然后，我们可以将通道添加到通道管理器中：

```c++
  pm.addPass(mlir::createShapeInferencePass());
```

如果我们重新运行原始示例，现在将得到以下结果：

```mlir
toy.func @main() {
  %0 = toy.constant dense<[[1.000000e+00, 2.000000e+00, 3.000000e+00], [4.000000e+00, 5.000000e+00, 6.000000e+00]]> : tensor<2x3xf64>
  %1 = toy.transpose(%0 : tensor<2x3xf64>) to tensor<3x2xf64>
  %2 = toy.mul %1, %1 : tensor<3x2xf64>
  toy.print %2 : tensor<3x2xf64>
  toy.return
}
```

你可以编译 `toyc-ch4` 并自行尝试：`toyc-ch4 test/Examples/Toy/Ch4/codegen.toy -emit=mlir -opt`。

在[下一章](Ch-5.md)中，我们将开始代码生成的过程，以较低层次的方言为目标，优化一些计算密集型的 Toy 操作。
