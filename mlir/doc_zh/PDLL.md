# PDLL - PDL 语言

本文档详细介绍了 PDL 语言（PDLL），这是一种专为编写针对 MLIR 的模式重写而设计的自定义前端语言。

注意：本文档假设读者熟悉 MLIR 的相关概念，特别是
[MLIR 模式重写](PatternRewriter.md) 和
[操作定义规范（ODS）](DefiningDialects/Operations.md)
文档中详细描述的概念。

[TOC]

## 简介

模式匹配是 MLIR 中极其重要的组成部分，它涵盖了编译器的许多不同方面。从规范化，到优化，再到转换；每个基于 MLIR 的编译器都将在某种程度上大量依赖模式匹配基础设施。

PDL 语言（PDLL）提供了一种声明式模式语言，从头开始专门设计用于表示 MLIR 模式重写。PDLL 被设计为通过直观的接口原生支持在 MLIR 的所有构造上编写匹配器，该接口既可用于提前编译（AOT）也可用于即时编译（JIT）模式编译。

## 设计理念

本节提供了关于各种设计决策、其设计理念以及在设计 PDLL 时所考虑的替代方案的详细信息。鉴于软件开发的性质，本节可能包含对已不再存在的 MLIR 编译器区域的引用。

### 为什么要构建一种新语言而不是改进 TableGen DRR？

注意：本节假设读者熟悉
[TDRR](DeclarativeRewrites.md)，请在继续之前参阅相关文档。

Tablegen DRR（TDRR），即
[表驱动声明式重写规则](DeclarativeRewrites.md)，
是一种用于在
[TableGen](https://llvm.org/docs/TableGen/index.html) 语言中定义 MLIR 模式重写的声明式 DSL。这套基础设施目前是在 MLIR 中以声明方式定义模式的主要方式。TDRR 利用 TableGen 的 `dag` 支持来定义适合 DAG 结构的 MLIR 模式；这与 TableGen 被用来为 LLVM 后端基础设施（SelectionDAG/Global Isel 等）定义模式的方式类似。然而不幸的是，TableGen 语言对 MLIR 模式结构的适应性不如对 LLVM 的适应性好。

TDRR 的问题主要源于将 TableGen 用作 DSL 的宿主语言。这些问题源于 TableGen 结构与 MLIR 结构之间的不匹配，以及 TableGen 与 MLIR 具有不同的动机目标。我们在 TDRR 中遇到的大多数（或全部，取决于您的坚持程度）问题在某种形式上都是可以解决的；关键在于这些问题的解决方案往往比我们希望的更"创造性"。这是一个问题，也是我们决定不投入更大努力改进 TDRR 的原因；用户通常不想要"创造性"的 API，他们想要直观易读易写的东西。

为了突出这些问题，下面我们将浏览一些已经出现的问题，以及我们是如何"修复"它们的。

#### 多结果操作

MLIR 原生支持可变数量的操作结果。对于 TDRR 基于 DAG 的结构，任何形式的多个结果（在此实例中是操作）都会产生问题。这是因为 DAG 需要一个单一的根节点，并且没有很好的工具来索引或命名多个结果。让我们通过一个简单的例子来看看这是如何体现的：

```tablegen
// 假设我们有一个三结果操作，定义如下所示。
def ThreeResultOp : Op<"three_result_op"> {
    let arguments = (ins ...);

    let results = (outs
      AnyTensor:$output1,
      AnyTensor:$output2,
      AnyTensor:$output3
    );
}

// 要在 TDRR 模式中绑定 `ThreeResultOp` 的结果，我们将所有结果
// 绑定到一个名称，并使用特殊的命名约定：`__N`，其中 `N` 是第 N 个结果。
def : Pattern<(ThreeResultOp:$results ...),
              [(... $results__0), ..., (... $results__2), ...]>;
```

在 TDRR 中，我们"解决"了访问多个结果的问题，但这对用户来说并不是一个非常直观的接口。神奇的命名约定会混淆代码，容易引入错误和其他问题。我们可以尝试各种方法来改善这种情况，但鉴于 TableGen dag 结构的限制，我们能做的有根本的上限。然而在 PDLL 中，我们有自由和灵活性来提供一个适当的操作接口，无论其结构如何：

```pdll
// 导入我们对 `ThreeResultOp` 的定义。
#include "ops.td"

Pattern {
  ...

  // 在 PDLL 中，我们可以直接引用操作变量的结果。
  // 这为用户提供了更接近其预期的心智模型。
  let threeResultOp = op<my_dialect.three_result_op>;
  let userOp = op<my_dialect.user_op>(threeResultOp.output1, ..., threeResultOp.output3);

  ...
}
```

#### 约束

在 TDRR 中，匹配 dag 定义了要匹配的输入 IR 的总体结构。对输入的任何非结构性/非类型约束通常被归入在重写 dag 之后指定的约束列表中。对于非常简单的模式，这可能就足够了，但对于较大的模式，它变得相当有问题，因为它将约束与其所约束的实体分离，并对模式的可读性产生负面影响。作为例子，让我们看一个向其输入添加额外约束的简单模式：

```tablegen
// 假设我们有一个两结果操作，定义如下所示。
def TwoResultOp : Op<"two_result_op"> {
    let arguments = (ins ...);

    let results = (outs
      AnyTensor:$output1,
      AnyTensor:$output2
    );
}

// 一个简单的约束来检查值是否 use_empty。
def HasNoUseOf: Constraint<CPred<"$_self.use_empty()">, "has no use">;

// 检查两个值是否具有相同元素类型的 ShapedType。
def HasSameElementType : Constraint<
    CPred<"cast<ShapedType>($0.getType()).getElementType() == "
          "cast<ShapedType>($1.getType()).getElementType()">,
    "values have same element type">;

def : Pattern<(TwoResultOp:$results $input),
              [(...), (...)],
              [(HasNoUseOf:$results__1),
               (HasSameElementType $results__0, $input)]>;
```

上面，当观察约束时，我们需要在输入 dag 中搜索输入（同时还要记住多个结果的神奇命名约定）。对于这个简单的模式，它可能只在几行以上，但复杂的模式通常会增长到 10 多行长。在 PDLL 中，这些约束可以直接应用于或紧邻其所应用的实体：

```pdll
// 我们上面定义的相同约束：
Constraint HasNoUseOf(value: Value) [{
  return success(value.use_empty());
}];
Constraint HasSameElementType(value1: Value, value2: Value) [{
  return success(cast<ShapedType>(value1.getType()).getElementType() ==
                 cast<ShapedType>(value2.getType()).getElementType());
}];

Pattern {
  // 在 PDLL 中，我们可以尽早（或尽晚）应用约束。这
  // 使得匹配器代码的结构更好，并提高了模式的
  // 可读性/可维护性。
  let op = op<my_dialect.two_result_op>(input: Value);
  HasNoUseOf(op.output2);
  HasSameElementType(input, op.output2);

  // ...
}
```

#### 替换多个操作

通常，模式会将 N 个输入操作转换为 N 个结果操作。在 PDLL 中，替换多个操作就像添加两个 [`replace` 语句](#replace-statement) 一样简单。在 TDRR 中，情况要更微妙一些。鉴于 TableGen dag 的单根结构，替换非根操作并不能很好地支持。目前它不能原生实现，而是需要使用多个模式。我们可能会添加另一个特殊的重写指令，或扩展 `replaceWithValue`，但这只是进一步说明了即使是基本的 IR 转换也会被宿主语言的复杂性所困扰。

### 为什么不在"X"中构建 DSL？

可以！嗯，可以也不可以。要理解其原因，我们必须考虑我们试图服务的用户类型以及我们对他们施加的约束。PDLL 的目标是为 MLIR 提供一种默认且有效的模式语言，无论其宿主环境如何，所有 MLIR 用户都可以立即与之交互。这种语言无需任何额外的依赖项，并与 MLIR 一起"免费"提供。如果我们使用现有的宿主语言来构建我们的新 DSL，我们将需要根据该语言做出相应的妥协。对于某些语言，存在如何强制匹配环境的问题（python2 还是 python3？哪个版本？）、性能考虑、集成等问题。作为 LLVM 项目，这也可能意味着对 MLIR 用户强制添加新的语言依赖项（其中许多用户可能不希望/不需要这样的依赖项）。嵌入另一种语言的任何 DSL 都会带来另一个问题：减轻用户在对宿主语言的预期与我们的"后端"支持之间的阻抗失配。例如，PDL IR 抽象仅包含对控制流的有限支持。如果我们要在 python 中构建 DSL，我们需要确保复杂的控制流要么被完全处理，要么有效地报错。即使有理想的错误处理，缺少预期的功能也会造成用户沮丧。除了环境约束之外，还有语言工具的问题。使用 PDLL，我们打算构建一个非常强大和现代化的工具集，专为满足模式开发人员的需求而设计，包括代码补全、签名帮助以及许多其他针对我们所解决问题的特定功能。将自定义语言工具集成到现有语言中可能很困难，在某些情况下甚至不可能（因为我们的 DSL 仅仅是现有语言的一个小子集）。

这些不同的观点使我们得出了初步结论：我们可以为用户提供的最有效工具是专为手头问题设计的自定义工具。话虽如此，我们理解并非所有用户都有我们所施加的相同约束。我们绝对鼓励并支持以不同语言定义的各种 PDL 前端的存在。这是首先构建 PDL IR 抽象的最初动机之一；为我们的用户（以及他们的用户）实现创新和灵活性。对于某些人，例如研究人员和机器学习领域的人，他们可能已经将某种语言（如 Python）深度集成到他们的工作流程中。对于这些用户，其语言中的 PDL DSL 可能是理想的，我们将始终致力于从基础设施角度支持和认可这一点。

## 语言规范

注意：PDLL 仍在积极开发中，下面讨论的设计不一定是最终的，可能会发生变化。

PDLL 的设计受到
[PDL IR 抽象](https://mlir.llvm.org/docs/Dialects/PDLOps/)的深刻影响，而 PDL IR 又被设计为核心 MLIR 结构的抽象模型。这导致了一种设计和结构，感觉非常类似于直接编写您想要匹配的 IR。

### 包含文件

PDLL 支持 `include` 指令来导入在其他源文件中定义的内容。可以包含两种类型的文件：`.pdll` 和 `.td` 文件。

#### `.pdll` 包含

包含 `.pdll` 文件时，该文件的内容将直接复制到正在处理的当前文件中。这意味着在该文件中定义的任何模式、约束、重写等都将与当前文件中的内容一起处理。

#### `.td` 包含

包含 `.td` 文件时，PDLL 将自动导入该文件中任何相关的
[ODS](DefiningDialects/Operations.md) 信息。
这包括任何已定义的操作、约束、接口等，使它们在 PDLL 中隐式可访问。这很重要，因为 ODS 信息允许某些 PDLL 构造（例如
[`operation` 表达式](#operation)）变得更加强大。

### 模式

在任何模式描述语言中，模式定义都是核心。在 PDLL 中，模式以 `Pattern` 开始，可选地跟随名称和一组模式元数据，最后以模式主体结束。下面显示了几个简单的例子：

```pdll
// 这里我们定义了一个匿名模式：
Pattern {
  // 模式主体分为两个组件：
  // * 匹配部分
  //    - 描述输入 IR。
  let root = op<toy.reshape>(op<toy.reshape>(arg: Value));

  // * 重写部分
  //    - 描述如何转换 IR。
  //    - 最后一条语句开始重写。
  replace root with op<toy.reshape>(arg);
}

// 这里我们定义了一个名为 `ReshapeReshapeOptPattern`、收益为 10 的模式：
Pattern ReshapeReshapeOptPattern with benefit(10) {
  replace op<toy.reshape>(op<toy.reshape>(arg: Value))
    with op<toy.reshape>(arg);
}
```

在定义模式元数据之后，我们指定模式主体。模式主体的结构由两个主要部分组成：`match` 部分和 `rewrite` 部分。模式的 `match` 部分描述预期的输入 IR，而 `rewrite` 部分描述如何转换该 IR。这种区分非常重要，因为 PDLL 在不同部分中对某些变量和表达式的处理方式不同。在下面每个部分的相关内容中，我们将明确指出任何行为差异。

`match` 和 `rewrite` 部分的总体布局如下：模式主体的*最后*一条语句必须是
[`操作重写语句`](#operation-rewrite-statements)，它表示 `rewrite` 部分；之前的每条语句表示 `match` 部分。

#### 模式元数据

MLIR 中的重写模式具有一组元数据，可以控制某些行为，并向应用模式的重写驱动程序提供信息。在 PDLL 中，模式可以在模式名称之后为此元数据提供非默认值。下面显示了支持的不同类型元数据的示例：

##### 收益

模式的收益是一个整数值，代表匹配该模式的"收益"。模式驱动程序使用它来确定模式在应用期间的相对优先级；收益较高的模式通常在收益较低的模式之前应用。

在 PDLL 中，模式的默认收益设置为匹配部分中输入操作的数量，即不同 `Op` 表达式/变量的数量。这条规则基于以下观察：较大的匹配比较小的匹配更有收益，如果先应用较小的匹配，较大的匹配可能不再适用。模式可以通过在模式的元数据部分中指定收益来覆盖此行为：

```pdll
// 这里我们指定此模式的收益为 `10`，覆盖默认行为。
Pattern with benefit(10) {
  ...
}
```

##### 有界重写递归

在模式应用期间，存在模式可能适用于同一模式先前应用结果的情况。如果模式不能正确处理这种递归应用，模式驱动程序可能会陷入无限的应用循环。为了防止这种情况，默认情况下模式被认为没有适当的递归边界，不会被递归应用。模式可以通过在模式元数据部分中指定 `recusion` 标志来表示它确实具有适当的递归处理：

```pdll
// 这里我们表示此模式正确地限制了递归应用。
Pattern with recusion {
  ...
}
```

#### 单行"Lambda"主体

模式通常使用复合语句块来定义其主体，如下所示：

```pdll
Pattern {
  replace op<my_dialect.foo>(operands: ValueRange) with operands;
}
```

模式也支持类似 lambda 的语法来指定简单的单行主体。模式的 lambda 主体期望一个单一的
[操作重写语句](#operation-rewrite-statements)：

```pdll
Pattern => replace op<my_dialect.foo>(operands: ValueRange) with operands;
```

### 变量

PDLL 中的变量代表 IR 实体的特定实例，例如 `Value`、`Operation`、`Type` 等。考虑下面这个简单模式：

```pdll
Pattern {
  let value: Value;
  let root = op<mydialect.foo>(value);

  replace root with value;
}
```

在这个模式中，我们使用 `let` 语句定义了两个变量 `value` 和 `root`。`let` 语句允许定义变量并对其进行约束。PDLL 中的每个变量都具有某种类型，该类型定义了变量所代表的 IR 实体的类型。变量的类型可以通过约束或初始化表达式来确定。

#### 变量"绑定"

除了具有类型之外，变量还必须被"绑定"，可以通过初始化表达式绑定，也可以在模式的 `match` 部分中绑定到非原生约束或重写使用。"绑定"变量在上下文中标识该变量在输入（即 `match` 部分）或输出（即 `rewrite` 部分）IR 中对应什么。在 `match` 部分，这允许从模式的根操作构建匹配树，根操作必须被"绑定"到表示模式 `rewrite` 部分的[操作重写语句](#operation-rewrite-statements)。`match` 部分中所有非根变量必须以某种方式绑定到"根"操作。为了帮助说明这个概念，让我们看一个简单的例子。考虑下面的 `.mlir` 片段：

```mlir
func.func @baz(%arg: i32) {
  %result = my_dialect.foo %arg, %arg -> i32
}
```

假设我们想编写一个匹配 `my_dialect.foo` 并用其唯一输入参数替换它的模式。在 PDLL 中编写此模式的一种简单方式如下所示：

```pdll
Pattern {
  // ** 匹配部分 ** //
  let arg: Value;
  let root = op<my_dialect.foo>(arg, arg);

  // ** 重写部分 ** //
  replace root with arg;
}
```

在上面的模式中，`arg` 变量被"绑定"到 `root` 操作的第一和第二个操作数。`arg` 的每次使用都被约束为相同的 `Value`，即 `root` 的第一和第二个操作数将被约束为引用相同的输入 Value。对于 `root` 操作也是如此，它被绑定到模式的"根"操作，因为它用于模式 `rewrite` 部分顶层 [`replace` 语句](#replace-statement) 的输入中。使用 C++ API 编写此模式，"绑定"的概念变得更加清晰：

```c++
struct Pattern : public OpRewritePattern<my_dialect::FooOp> {
  LogicalResult matchAndRewrite(my_dialect::FooOp root, PatternRewriter &rewriter) {
    Value arg = root->getOperand(0);
    if (arg != root->getOperand(1))
      return failure();

    rewriter.replaceOp(root, arg);
    return success();
  }
};
```

如果变量没有被正确"绑定"，PDLL 将无法识别它在 IR 中对应什么值。作为最后一个例子，让我们考虑一个没有被绑定的变量：

```pdll
Pattern {
  // ** 匹配部分 ** //
  let arg: Value;
  let root = op<my_dialect.foo>

  // ** 重写部分 ** //
  replace root with arg;
}
```

如果我们用 C++ 编写这个确切的模式，我们最终会得到：

```c++
struct Pattern : public OpRewritePattern<my_dialect::FooOp> {
  LogicalResult matchAndRewrite(my_dialect::FooOp root, PatternRewriter &rewriter) {
    // `arg` 从未被绑定，因此我们不知道它要对应的输入 Value 是什么。
    Value arg;

    rewriter.replaceOp(root, arg);
    return success();
  }
};
```

#### 变量约束

```pdll
// 此语句定义了一个被约束为 `Value` 的变量 `value`。
let value: Value;

// 此语句定义了一个被约束为 `Value` 的变量 `value`，
// *并且*被约束为只有一个使用。
let value: [Value, HasOneUse];
```

任意数量的单实体约束可以在声明时直接附加到变量上。在 `matcher` 部分中，这些约束可以对输入 IR 添加额外的检查。在 `rewriter` 部分中，约束*仅*用于定义变量的类型。有许多内置约束与核心 MLIR 构造相对应：`Attr`、`Op`、`Type`、`TypeRange`、`Value`、`ValueRange`。除此之外，用户还可以定义在 PDLL 中实现的自定义约束，或者原生实现的约束（即在 PDLL 外部）。有关更多详细信息，请参见[约束](#constraints)部分。

#### 内联变量定义

除了 `let` 语句之外，变量也可以通过在变量首次使用的地方指定约束列表和所需的变量名来内联定义。定义后，变量从所有后续点都可见。请参见下面的例子：

```pdll
// `value` 被用作操作 `root` 的操作数：
let value: Value;
let root = op<my_dialect.foo>(value);
replace root with value;

// `value` 也可以"内联"定义：
let root = op<my_dialect.foo>(value: Value);
replace root with value;
```

注意，内联变量的定义点就是引用点，这意味着内联变量可以在其定义的同一父表达式中立即使用：

```pdll
let root = op<my_dialect.foo>(value: Value, _: Value, value);
replace root with value;
```

##### 通配符变量定义

通常在内联定义变量时，该变量并不打算在模式的其他地方使用。例如，如果您想将约束附加到变量但没有其他用途，就可能发生这种情况。在这些情况下，可以使用"通配符"变量来消除提供名称的需要，因为"通配符"变量在定义点之外不可见。下面显示了一个例子：

```pdll
Pattern {
  let root = op<my_dialect.foo>(arg: Value, _: Value, _: [Value, I64Value], arg);
  replace root with arg;
}
```

在上面的例子中，第二个操作数在模式中不需要，但我们需要提供它来表示第二个操作数确实存在（我们只是不关心它在这个模式中是什么）。

### 操作表达式

PDLL 中的操作表达式代表一个 MLIR 操作。在模式的 `match` 部分，该表达式模拟模式的一个输入操作。在模式的 `rewrite` 部分，该表达式模拟要创建的操作之一。操作表达式的总体结构与 MLIR 文本汇编的"通用形式"非常相似：

```pdll
let root = op<my_dialect.foo>(operands: ValueRange) {attr = attr: Attr} -> (resultTypes: TypeRange);
```

让我们逐一了解表达式的各个不同组成部分：

#### 操作名称

操作名称表示此操作对应的 MLIR Op 类型。在模式的 `match` 部分，名称可以省略。这将导致该模式匹配满足操作其余约束的*任何*操作类型。在 `rewrite` 部分，名称是必需的。

```pdll
// `root` 对应于 `my_dialect.foo` 操作的一个实例。
let root = op<my_dialect.foo>;

// `root` 可以是任何操作类型的实例。
let root = op<>;
```

#### 操作数

操作数部分对应于操作的操作数。操作表达式的这个部分可以省略，在 `match` 部分中意味着操作数不受任何约束。如果在 `rewrite` 部分中省略，操作将被视为没有操作数。当存在时，操作表达式的操作数按以下方式解释：

1) 类型为 `ValueRange` 的单个实例：

在这种情况下，单个范围被视为操作的所有操作数：

```pdll
// 定义一个具有单个操作数范围的实例。
let root = op<my_dialect.foo>(allOperands: ValueRange);
```

2) 可变数量的 `Value` 或 `ValueRange`：

在这种情况下，输入应与 ODS 中操作上定义的操作数组相对应。

给定 ODS 中的以下操作定义：

```tablegen
def MyIndirectCallOp {
  let arguments = (ins FunctionType:$call, Variadic<AnyType>:$args);
}
```

我们可以这样匹配操作数：

```pdll
let root = op<my_dialect.indirect_call>(call: Value, args: ValueRange);
```

#### 结果

结果部分对应于操作的结果类型。操作表达式的这个部分可以省略，在 `match` 部分中意味着结果类型不受任何约束。如果在 `rewrite` 部分中省略，操作的结果将被[推断](#inferred-results)。当存在时，操作表达式的结果类型按以下方式解释：

1) 类型为 `TypeRange` 的单个实例：

在这种情况下，单个范围被视为操作的所有结果类型：

```pdll
// 定义一个具有单个类型范围的实例。
let root = op<my_dialect.foo> -> (allResultTypes: TypeRange);
```

2) 可变数量的 `Type` 或 `TypeRange`：

在这种情况下，输入应与 ODS 中操作上定义的结果组相对应。

给定 ODS 中的以下操作定义：

```tablegen
def MyOp {
  let results = (outs SomeType:$result, Variadic<SomeType>:$otherResults);
}
```

我们可以这样匹配结果类型：

```pdll
let root = op<my_dialect.op> -> (result: Type, otherResults: TypeRange);
```

#### 推断结果

在模式的 `rewrite` 部分中，如果操作的结果类型被省略或尚未被绑定，则会被推断。上面的["变量绑定"](#variable-binding)部分更详细地讨论了"绑定"的概念。下面是各种例子，基于此来展示结果类型如何被"绑定"：

* 绑定到[常量](#type-expression)：

```pdll
op<my_dialect.op> -> (type<"i32">);
```

* 绑定到 `match` 部分中的类型：

```pdll
Pattern {
  replace op<dialect.inputOp> -> (resultTypes: TypeRange)
    with op<dialect.outputOp> -> (resultTypes);
}
```

* 绑定到先前推断的类型：

```pdll
Pattern {
  rewrite root: Op with {
    // 这里的 `resultTypes` 尚未绑定，将在
    // 创建 `dialect.op` 时被推断。在此表达式之后
    // 对 `resultTypes` 的任何使用都将使用创建此操作时推断的类型。
    op<dialect.op> -> (resultTypes: TypeRange);

    // 这里的 `resultTypes` 绑定到创建 `dialect.op` 时推断的类型。
    op<dialect.bar> -> (resultTypes);
  };
}
```

* 绑定到[`原生重写`](#native-rewriters)方法的结果：

```pdll
Rewrite BuildTypes() -> TypeRange;

Pattern {
  rewrite root: Op with {
    op<dialect.op> -> (BuildTypes());
  };
}
```

以下是支持结果类型推断的上下文集合：

##### 替换操作的推断结果

替换具有不变量，即替换值的类型必须与输入操作的结果类型匹配。这意味着当用另一个操作替换一个操作时，替换操作的结果类型可以从被替换操作的结果类型中推断出来。例如，考虑以下模式：

```pdll
Pattern => replace op<dialect.inputOp> with op<dialect.outputOp>;
```

此模式可以以更明确的方式编写为：

```pdll
Pattern {
  replace op<dialect.inputOp> -> (resultTypes: TypeRange)
    with op<dialect.outputOp> -> (resultTypes);
}
```

##### 使用 InferTypeOpInterface 的推断结果

`InferTypeOpInterface` 是一个接口，使操作能够从其输入属性、操作数、区域等推断其结果类型。当无法从任何其他上下文推断操作的结果类型时，将调用此接口来推断操作的结果类型。

#### 属性

操作表达式的属性部分对应于操作的属性字典。操作表达式的这个部分可以省略，在这种情况下，属性不受任何约束。此组件的组合与 MLIR 文本汇编格式中属性字典的结构完全对应：

```pdll
let root = op<my_dialect.foo> {attr1 = attrValue: Attr, attr2 = attrValue2: Attr};
```

在 `{}` 中，属性条目由标识符或字符串名称指定，对应于属性名称，后跟对属性值的赋值。如果属性值被省略，则属性的值被隐式定义为
[`UnitAttr`](https://mlir.llvm.org/docs/Dialects/Builtin/#unitattr)。

```pdll
let unitConstant = op<my_dialect.constant> {value};
```

##### 访问操作结果

在多操作模式中，一个操作的结果通常作为输入馈送到另一个操作。操作的结果组可以通过 `.` 运算符按名称或索引访问：

注意：记得通过
[include](#`.td`_includes) 导入您操作的定义，以确保它对 PDLL 可见。

给定 ODS 中的以下操作定义：

```tablegen
def MyResultOp {
  let results = (outs SomeType:$result);
}
def MyInputOp {
  let arguments = (ins SomeType:$input, SomeType:$input);
}
```

我们可以编写一个 `MyResultOp` 馈送到 `MyInputOp` 的模式，如下所示：

```pdll
// 在此示例中，我们使用 `result`（名称）和 `0`（索引）来引用
// `resultOp` 的第一个结果组。
// 注意：如果我们在匹配部分省略结果类型部分，这意味着
//       它们不受约束，而不是操作没有结果。
let resultOp = op<my_dialect.result_op>;
let inputOp = op<my_dialect.input_op>(resultOp.result, resultOp.0);
```

除了按结果名称访问之外，`Op` 类型的变量还可以隐式转换为 `Value` 或 `ValueRange`。如果这些变量已注册（有 ODS 条目），当已知它们只有一个结果时，它们将转换为 `Value`，否则将转换为 `ValueRange`：

```pdll
// `resultOp` 也可以隐式转换为 Value 以在 `inputOp` 中使用：
let resultOp = op<my_dialect.result_op>;
let inputOp = op<my_dialect.input_op>(resultOp);

// 我们也可以直接内联 `resultOp`：
let inputOp = op<my_dialect.input_op>(op<my_dialect.result_op>);
```

#### 未注册操作

未注册 op 的变量仍然可用于数字结果索引。由于我们不了解其结果组，数字索引返回给定索引处单个结果对应的 Value。

```pdll
// 使用索引 `0` 引用未注册 op 的第一个结果值。
let inputOp = op<my_dialect.input_op>(op<my_dialect.unregistered_op>.0);
```

### 属性表达式

属性表达式代表字面量 MLIR 属性。它允许通过指定该属性的文本形式来静态指定要使用的 MLIR 属性。

```pdll
let trueConstant = op<arith.constant> {value = attr<"true">};

let applyResult = op<affine.apply>(args: ValueRange) {map = attr<"affine_map<(d0, d1) -> (d1 - 3)>">}
```

### 类型表达式

类型表达式代表字面量 MLIR 类型。它允许通过指定该类型的文本形式来静态指定要使用的 MLIR 类型。

```pdll
let i32Constant = op<arith.constant> -> (type<"i32">);
```

### 元组

PDLL 为元组提供原生支持，用于将多个元素分组为单个复合值。元组中的值可以是任何类型，不需要是相同类型。元组持有的元素数量也没有限制。元组的元素可以通过索引访问：

```pdll
let tupleValue = (op<my_dialect.foo>, attr<"10 : i32">, type<"i32">);

let opValue = tupleValue.0;
let attrValue = tupleValue.1;
let typeValue = tupleValue.2;
```

您还可以为元组的元素命名，并使用这些名称来引用各个元素的值。元素名称由紧跟等号（=）的标识符组成。

```pdll
let tupleValue = (
  opValue = op<my_dialect.foo>,
  attr<"10 : i32">,
  typeValue = type<"i32">
);

let opValue = tupleValue.opValue;
let attrValue = tupleValue.1;
let typeValue = tupleValue.typeValue;
```

元组用于表示来自[约束](#constraints-with-multiple-results)或[重写](#rewrites-with-multiple-results)的多个结果。

### 约束

约束提供了在模式的 `match` 部分向输入 IR 注入额外检查的能力。约束可以应用于 `match` 部分的任何地方，根据类型可以通过[变量](#variables)的约束列表或通过调用运算符（例如 `MyConstraint(...)`）来应用。约束有三大类别：

#### 核心约束

PDLL 定义了许多约束 IR 实体类型的核心约束。这些约束只能通过变量的[约束列表](#variable-constraints)应用。

*   `Attr` (`<` type `>`)?

对应于 `mlir::Attribute` 的单实体约束。此约束可选地接受一个类型组件，用于约束属性的结果类型。

```pdll
// 使用 `Attr` 约束定义一个简单变量。
let attr: Attr;
let constant = op<arith.constant> {value = attr};

// 使用 `Attr` 约束定义一个简单变量，其类型也受到约束。
let attrType: Type;
let attr: Attr<attrType>;
let constant = op<arith.constant> {value = attr};
```

*   `Op` (`<` op-name `>`)?

对应于 `mlir::Operation *` 的单实体约束。

```pdll
// 仅当输入来自另一个操作时才匹配。
let inputOp: Op;
let root = op<my_dialect.foo>(inputOp);

// 仅当输入来自另一个 `my_dialect.foo` 操作时才匹配。
let inputOp: Op<my_dialect.foo>;
let root = op<my_dialect.foo>(inputOp);
```

*   `Type`

对应于 `mlir::Type` 的单实体约束。

```pdll
// 使用 `Type` 约束定义一个简单变量。
let resultType: Type;
let root = op<my_dialect.foo> -> (resultType);
```

*   `TypeRange`

对应于 `mlir::TypeRange` 的单实体约束。

```pdll
// 使用 `TypeRange` 约束定义一个简单变量。
let resultTypes: TypeRange;
let root = op<my_dialect.foo> -> (resultTypes);
```

*   `Value` (`<` type-expr `>`)?

对应于 `mlir::Value` 的单实体约束。此约束可选地接受一个类型组件，用于约束值的结果类型。

```pdll
// 使用 `Value` 约束定义一个简单变量。
let value: Value;
let root = op<my_dialect.foo>(value);

// 使用 `Value` 约束定义一个变量，其类型被约束为与 `root` op 的结果类型相同。
let valueType: Type;
let input: Value<valueType>;
let root = op<my_dialect.foo>(input) -> (valueType);
```

*   `ValueRange` (`<` type-expr `>`)?

对应于 `mlir::ValueRange` 的单实体约束。此约束可选地接受一个类型组件，用于约束值范围的结果类型。

```pdll
// 使用 `ValueRange` 约束定义一个简单变量。
let inputs: ValueRange;
let root = op<my_dialect.foo>(inputs);

// 使用 `ValueRange` 约束定义一个变量，其类型被约束为与 `root` op 的结果类型相同。
let valueTypes: TypeRange;
let inputs: ValueRange<valueTypes>;
let root = op<my_dialect.foo>(inputs) -> (valueTypes);
```

#### 在 PDLL 中定义约束

除了核心约束之外，还可以在 PDLL 中定义额外的约束。这允许构建可以在许多不同模式中组合的匹配器片段。PDLL 中的约束的定义类似于传统编程语言中的函数；它包含名称、一组输入参数、一组结果类型和一个主体。约束的结果通过 `return` 语句返回。下面显示了几个例子：

```pdll
/// 一个接受输入并将使用约束到给定类型操作的约束。
Constraint UsedByFooOp(value: Value) {
  op<my_dialect.foo>(value);
}

/// 一个返回现有操作结果的约束。
Constraint ExtractResult(op: Op<my_dialect.foo>) -> Value {
  return op.result;
}

Pattern {
  let value = ExtractResult(op<my_dialect.foo>);
  UsedByFooOp(value);
}
```

##### 具有多个结果的约束

约束可以通过返回值元组来返回多个结果。返回多个结果时，每个结果也可以被赋予一个名称，在索引该元组元素时使用。元组元素可以通过其索引号引用，如果被赋予名称，也可以通过名称引用。

```pdll
// 一个返回多个结果的约束，其中一些结果被赋予了更具可读性的名称。
Constraint ExtractMultipleResults(op: Op<my_dialect.foo>) -> (Value, result1: Value) {
  return (op.result1, op.result2);
}

Pattern {
  // 返回值的元组。
  let result = ExtractMultipleResults(op: op<my_dialect.foo>);

  // 通过索引或名称索引元组元素。
  replace op<my_dialect.foo> with (result.0, result.1, result.result1);
}
```

##### 约束结果类型推断

除了通过约束签名明确指定约束的结果之外，PDLL 定义的约束也支持从 return 语句推断结果类型。当约束定义为没有结果约束时，结果类型推断处于活动状态：

```pdll
// 此约束返回一个派生操作。
Constraint ReturnSelf(op: Op<my_dialect.foo>) {
  return op;
}
// 此约束返回两个 Value 的元组。
Constraint ExtractMultipleResults(op: Op<my_dialect.foo>) {
  return (result1 = op.result1, result2 = op.result2);
}

Pattern {
  let values = ExtractMultipleResults(op<my_dialect.foo>);
  replace op<my_dialect.foo> with (values.result1, values.result2);
}
```

##### 单行"Lambda"主体

约束通常使用复合语句块来定义其主体，如下所示：

```pdll
Constraint ReturnSelf(op: Op<my_dialect.foo>) {
  return op;
}
Constraint ExtractMultipleResults(op: Op<my_dialect.foo>) {
  return (result1 = op.result1, result2 = op.result2);
}
```

约束也支持类似 lambda 的语法来指定简单的单行主体。约束的 lambda 主体期望一个单一的表达式，该表达式被隐式返回：

```pdll
Constraint ReturnSelf(op: Op<my_dialect.foo>) => op;

Constraint ExtractMultipleResults(op: Op<my_dialect.foo>)
  => (result1 = op.result1, result2 = op.result2);
```

#### 原生约束

约束也可以在 PDLL 外部定义，并在 C++ API 中原生注册。

##### 导入现有原生约束

外部定义的约束可以通过指定约束"声明"导入到 PDLL 中。这类似于 PDLL 定义约束的形式，但省略了主体。以这种形式导入声明允许 PDLL 静态地了解预期的输入和输出类型。

```pdll
// 导入一个检查值是否只有一个使用的单实体值原生约束。此约束必须由
// 已编译 PDL 的使用者注册。
Constraint HasOneUse(value: Value);

// 导入一个检查两个值是否具有相同元素类型的多实体类型约束。
Constraint HasSameElementType(value1: Value, value2: Value);

Pattern {
  // 单实体约束可以通过变量参数列表应用。
  let value: HasOneUse;

  // 否则，约束可以通过调用运算符应用：
  let value: Value = ...;
  let value2: Value = ...;
  HasOneUse(value);
  HasSameElementType(value, value2);
}
```

外部约束是通过 C++ PDL API 使用 `RewritePatternSet` 显式注册的那些约束。例如，上面的约束可以注册为：

```c++
static LogicalResult hasOneUseImpl(PatternRewriter &rewriter, Value value) {
  return success(value.hasOneUse());
}
static LogicalResult hasSameElementTypeImpl(PatternRewriter &rewriter,
                                            Value value1, Value Value2) {
  return success(cast<ShapedType>(value1.getType()).getElementType() ==
                 cast<ShapedType>(value2.getType()).getElementType());
}

void registerNativeConstraints(RewritePatternSet &patterns) {
    patternList.getPDLPatterns().registerConstraintFunction(
        "HasOneUse", hasOneUseImpl);
    patternList.getPDLPatterns().registerConstraintFunction(
        "HasSameElementType", hasSameElementTypeImpl);
}
```

##### 在 PDLL 中定义原生约束

除了导入原生约束之外，PDLL 还支持在为 C++ 提前编译（AOT）时直接定义原生约束。这些约束可以通过在约束声明后指定字符串代码块来定义：

```pdll
Constraint HasOneUse(value: Value) [{
  return success(value.hasOneUse());
}];
Constraint HasSameElementType(value1: Value, value2: Value) [{
  return success(cast<ShapedType>(value1.getType()).getElementType() ==
                 cast<ShapedType>(value2.getType()).getElementType());
}];

Pattern {
  // 单实体约束可以通过变量参数列表应用。
  let value: HasOneUse;

  // 否则，约束可以通过调用运算符应用：
  let value: Value = ...;
  let value2: Value = ...;
  HasOneUse(value);
  HasSameElementType(value, value2);
}
```

约束的参数可以通过相同的名称在代码块中访问。有关 PDLL 类型如何转换为原生类型的详细信息，请参见下面的["类型翻译"](#native-constraint-type-translations)。除了 PDLL 参数之外，代码块还可以使用 `rewriter` 访问当前的 `PatternRewriter`。原生约束函数的结果类型被隐式定义为 `::llvm::LogicalResult`。

以上面定义的约束为例，这些函数大致会被翻译为：

```c++
LogicalResult HasOneUse(PatternRewriter &rewriter, Value value) {
  return success(value.hasOneUse());
}
LogicalResult HasSameElementType(Value value1, Value value2) {
  return success(cast<ShapedType>(value1.getType()).getElementType() ==
                 cast<ShapedType>(value2.getType()).getElementType());
}
```

TODO: 在某些情况下，原生约束也应该被允许返回值。

###### 原生约束类型翻译

参数和结果变量的类型通常映射到所使用的[约束](#constraints)对应的 MLIR 类型。以下是对各种不同类型的约束如何确定变量的映射类型的详细描述。

* Attr, Op, Type, TypeRange, Value, ValueRange:

这些都是核心约束，直接映射到对应的 MLIR 等效类型（如其名称所示），即：

  * `Attr`       -> "::mlir::Attribute"
  * `Op`         -> "::mlir::Operation *"
  * `Type`       -> "::mlir::Type"
  * `TypeRange`  -> "::mlir::TypeRange"
  * `Value`      -> "::mlir::Value"
  * `ValueRange` -> "::mlir::ValueRange"

* Op<dialect.name>

命名操作约束有独特的翻译。如果引用操作的 ODS 注册已被包含，则使用限定的 C++ 形式。如果 ODS 信息不可用，此约束将映射到 "::mlir::Operation *"，类似于未命名的变体。例如，给定以下内容：

```pdll
// `my_ops.td` 提供了 `my_dialect` 操作的 ODS 定义，例如
// 下面使用的 `my_dialect.bar`。
#include "my_ops.td"

Constraint Cst(op: Op<my_dialect.bar>) [{
  return success(op ... );
}];
```

`op` 使用的原生类型可能采用 `my_dialect::BarOp` 的形式，而不是默认的 `::mlir::Operation *`。以下是上述约束的一个示例翻译：

```c++
LogicalResult Cst(my_dialect::BarOp op) {
  return success(op ... );
}
```

* 导入的 ODS 约束

除核心约束之外，从 ODS 导入的某些约束可能使用独特的原生类型。如何启用这种独特类型取决于导入的 ODS 约束构造：

  * `Attr` 约束
    - 导入的 `Attr` 约束使用 `storageType` 字段进行原生类型翻译。

  * `Type` 约束
    - 导入的 `Type` 约束使用 `cppClassName` 字段进行原生类型翻译。

  * `AttrInterface`/`OpInterface`/`TypeInterface` 约束
    - 导入的接口使用 `cppInterfaceName` 字段进行原生类型翻译。

#### 内联定义约束

除了全局范围之外，在 PDLL 中定义的 PDLL 约束和原生约束也可以在任何嵌套级别*内联*指定。这意味着它们可以在模式、其他约束、重写等中定义：

```pdll
Constraint GlobalConstraint() {
  Constraint LocalConstraint(value: Value) {
    ...
  };
  Constraint LocalNativeConstraint(value: Value) [{
    ...
  }];
  let someValue: [LocalConstraint, LocalNativeConstraint] = ...;
}
```

内联定义的约束在直接使用时也可以省略名称：

```pdll
Constraint GlobalConstraint(inputValue: Value) {
  Constraint(value: Value) { ... }(inputValue);
  Constraint(value: Value) [{ ... }](inputValue);
}
```

内联定义时，PDLL 约束可以引用任何先前定义的变量：

```pdll
Constraint GlobalConstraint(op: Op<my_dialect.foo>) {
  Constraint LocalConstraint() {
    let results = op.results;
  };
}
```

### 重写器

重写器定义在模式的 `rewrite` 部分中要执行的转换集合，更具体地说，是如何在成功的模式匹配之后转换输入 IR。所有 PDLL 重写必须在模式的 `rewrite` 部分中定义。`rewrite` 部分由 `Pattern` 主体中的最后一条语句表示，该语句必须是[操作重写语句](#operation-rewrite-statements)。PDLL 中有两大类重写：操作重写语句和用户定义的重写。

#### 操作重写语句

操作重写语句是内置的 PDLL 语句，根据根操作执行 IR 转换。这些语句是唯一能够开始模式 `rewrite` 部分的语句，因为它们允许正确地["绑定"](#variable-binding)模式的根操作。

##### `erase` 语句

```pdll
// 删除所有 `my_dialect.foo` 操作的模式。
Pattern => erase op<my_dialect.foo>;
```

`erase` 语句删除给定的操作。

##### `replace` 语句

```pdll
// 用其输入值替换根操作的模式。
Pattern {
  let root = op<my_dialect.foo>(input: Value);
  replace root with input;
}

// 用多个输入值替换根操作的模式。
Pattern {
  let root = op<my_dialect.foo>(input: Value, _: Value, input2: Value);
  replace root with (input, input2);
}

// 用另一个操作替换根操作的模式。
// 注意，当操作被用作替换时，我们可以从输入操作推断其
// 结果类型。在这些情况下，替换操作的结果类型可以省略。
Pattern {
  // 注意：在此模式中，我们还内联了 `root` 表达式。
  replace op<my_dialect.foo> with op<my_dialect.bar>;
}
```

`replace` 语句允许用另一个操作或一组输入 `Value` 和 `ValueRange` 值替换给定的根操作。当操作被用作替换时，我们允许从输入操作推断结果类型。在这些情况下，替换操作的结果类型可以省略。注意，在替换期间，除了结果类型之外，不会从输入操作推断其他组件。

##### `rewrite` 语句

```pdll
// 用其输入值替换根操作的简单模式。
Pattern {
  let root = op<my_dialect.foo>(input: Value);
  rewrite root with {
    ...

    replace root with input;
  };
}
```

`rewrite` 语句允许用嵌套重写器块重写给定的根操作。根操作不会被隐式删除或替换，对它的任何转换都必须在嵌套的重写块中表达。内部主体可以包含任意数量的其他重写语句、变量或表达式。

#### 在 PDLL 中定义重写器

额外的重写也可以在 PDLL 中定义，这允许构建可以在许多不同模式中组合的重写片段。PDLL 中的重写器的定义类似于传统编程语言中的函数；它包含名称、一组输入参数、一组结果类型和一个主体。重写的结果通过 `return` 语句返回。下面显示了几个例子：

```pdll
// 一个构造并返回新操作的重写器，给定一个输入值。
Rewrite BuildFooOp(value: Value) -> Op {
  return op<my_dialect.foo>(value);
}

Pattern {
  // 我们以与传统语言中函数相同的方式调用重写器。
  replace op<my_dialect.old_op>(input: Value) with BuildFooOp(input);
}
```

##### 具有多个结果的重写

重写可以通过返回值元组来返回多个结果。返回多个结果时，每个结果也可以被赋予一个名称，在索引该元组元素时使用。元组元素可以通过其索引号引用，如果被赋予名称，也可以通过名称引用。

```pdll
// 一个返回多个结果的重写，其中一些结果被赋予了更具可读性的名称。
Rewrite CreateRewriteOps() -> (Op, result1: ValueRange) {
  return (op<my_dialect.bar>, op<my_dialect.foo>);
}

Pattern {
  rewrite root: Op<my_dialect.foo> with {
    // 调用重写，返回值的元组。
    let result = CreateRewriteOps();

    // 通过索引或名称索引元组元素。
    replace root with (result.0, result.1, result.result1);
  }
}
```

##### 重写结果类型推断

除了通过重写签名明确指定重写的结果之外，PDLL 定义的重写也支持从 return 语句推断结果类型。当重写定义为没有结果约束时，结果类型推断处于活动状态：

```pdll
// 此重写返回一个派生操作。
Rewrite ReturnSelf(op: Op<my_dialect.foo>) => op;
// 此重写返回两个 Value 的元组。
Rewrite ExtractMultipleResults(op: Op<my_dialect.foo>) {
  return (result1 = op.result1, result2 = op.result2);
}

Pattern {
  rewrite root: Op<my_dialect.foo> with {
    let values = ExtractMultipleResults(op<my_dialect.foo>);
    replace root with (values.result1, values.result2);
  }
}
```

##### 单行"Lambda"主体

重写器通常使用复合语句块来定义其主体，如下所示：

```pdll
Rewrite ReturnSelf(op: Op<my_dialect.foo>) {
  return op;
}
Rewrite EraseOp(op: Op) {
  erase op;
}
```

重写器也支持类似 lambda 的语法来指定简单的单行主体。重写器的 lambda 主体期望一个单一的表达式，该表达式被隐式返回，或者一个单一的[操作重写语句](#operation-rewrite-statements)：

```pdll
Rewrite ReturnSelf(op: Op<my_dialect.foo>) => op;
Rewrite EraseOp(op: Op) => erase op;
```

#### 原生重写器

重写器也可以在 PDLL 外部定义，并在 C++ API 中原生注册。

##### 导入现有原生重写

外部定义的重写可以通过指定重写"声明"导入到 PDLL 中。这类似于 PDLL 定义重写的形式，但省略了主体。以这种形式导入声明允许 PDLL 静态地了解预期的输入和输出类型。

```pdll
// 导入一个接受单个输入并返回新操作的原生重写。此
// 重写必须由已编译 PDL 的使用者注册。
Rewrite BuildOp(value: Value) -> Op;

Pattern {
  replace op<my_dialect.old_op>(input: Value) with BuildOp(input);
}
```

外部重写是通过 C++ PDL API 使用 `RewritePatternSet` 显式注册的那些重写。例如，上面的重写可以注册为：

```c++
static Operation *buildOpImpl(PDLResultList &results, Value value) {
  // 在此处插入特殊重写逻辑。
  Operation *resultOp = ...;
  return resultOp;
}

void registerNativeRewrite(RewritePatternSet &patterns) {
  patterns.getPDLPatterns().registerRewriteFunction("BuildOp", buildOpImpl);
}
```

##### 在 PDLL 中定义原生重写

除了导入原生重写之外，PDLL 还支持在为 C++ 提前编译（AOT）时直接定义原生重写。这些重写可以通过在重写声明后指定字符串代码块来定义：

```pdll
Rewrite BuildOp(value: Value) -> (foo: Op<my_dialect.foo>, bar: Op<my_dialect.bar>) [{
  return {my_dialect::FooOp::create(rewriter, value), my_dialect::BarOp::create(rewriter)};
}];

Pattern {
  let root = op<my_dialect.foo>(input: Value);
  rewrite root with {
    // 调用原生重写并在替换根时使用结果。
    let results = BuildOp(input);
    replace root with (results.foo, results.bar);
  }
}
```

重写的参数可以通过相同的名称在代码块中访问。有关 PDLL 类型如何转换为原生类型的详细信息，请参见下面的["类型翻译"](#native-rewrite-type-translations)。除了 PDLL 参数之外，代码块还可以使用 `rewriter` 访问当前的 `PatternRewriter`。有关原生函数结果类型如何确定的详细信息，请参见["结果翻译"](#native-rewrite-result-translation)部分。

以上面定义的重写为例，这个函数大致会被翻译为：

```c++
std::tuple<my_dialect::FooOp, my_dialect::BarOp> BuildOp(Value value) {
  return {my_dialect::FooOp::create(rewriter, value), my_dialect::BarOp::create(rewriter)};
}
```

###### 原生重写类型翻译

参数和结果变量的类型通常映射到所使用的[约束](#constraints)对应的 MLIR 类型。原生 `Rewrite` 类型翻译的规则与原生 `Constraint` 的规则相同，请查看相应的[原生 `Constraint` 类型翻译](#native-constraint-type-translations)部分，了解变量映射类型如何确定的详细描述。

###### 原生重写结果翻译

原生重写的结果直接翻译为原生函数的结果，使用[上面描述的](#native-rewrite-type-translations)类型翻译规则。以下部分描述了各种结果翻译场景：

* 零结果

```pdll
Rewrite createOp() [{
  my_dialect::FooOp::create(rewriter);
}];
```

在原生 `Rewrite` 没有结果的情况下，原生函数返回 `void`：

```c++
void createOp(PatternRewriter &rewriter) {
  my_dialect::FooOp::create(rewriter);
}
```

* 单结果

```pdll
Rewrite createOp() -> Op<my_dialect.foo> [{
  return my_dialect::FooOp::create(rewriter);
}];
```

在原生 `Rewrite` 有单个结果的情况下，原生函数返回该单个结果对应的原生类型：

```c++
my_dialect::FooOp createOp(PatternRewriter &rewriter) {
  return my_dialect::FooOp::create(rewriter);
}
```

* 多结果

```pdll
Rewrite complexRewrite(value: Value) -> (Op<my_dialect.foo>, FunctionOpInterface) [{
  ...
}];
```

在原生 `Rewrite` 有多个结果的情况下，原生函数返回包含每个结果对应原生类型的 `std::tuple<...>`：

```c++
std::tuple<my_dialect::FooOp, FunctionOpInterface>
complexRewrite(PatternRewriter &rewriter, Value value) {
  ...
}
```

#### 内联定义重写

除了全局范围之外，在 PDLL 中定义的 PDLL 重写和原生重写也可以在任何嵌套级别*内联*指定。这意味着它们可以在模式、其他重写等中定义：

```pdll
Rewrite GlobalRewrite(inputValue: Value) {
  Rewrite localRewrite(value: Value) {
    ...
  };
  Rewrite localNativeRewrite(value: Value) [{
    ...
  }];
  localRewrite(inputValue);
  localNativeRewrite(inputValue);
}
```

内联定义的重写在直接使用时也可以省略名称：

```pdll
Rewrite GlobalRewrite(inputValue: Value) {
  Rewrite(value: Value) { ... }(inputValue);
  Rewrite(value: Value) [{ ... }](inputValue);
}
```

内联定义时，PDLL 重写可以引用任何先前定义的变量：

```pdll
Rewrite GlobalRewrite(op: Op<my_dialect.foo>) {
  Rewrite localRewrite() {
    let results = op.results;
  };
}
```
