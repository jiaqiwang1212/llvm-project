# MLIR 语言参考

MLIR（多层次 IR）是一种编译器中间表示，与传统的三地址 SSA 表示（如
[LLVM IR](http://llvm.org/docs/LangRef.html) 或
[SIL](https://github.com/apple/swift/blob/main/docs/SIL.rst)）有相似之处，但引入了多面体循环优化中的概念作为一等概念。这种混合设计旨在表示、分析和转换高层数据流图以及为高性能数据并行系统生成的目标特定代码。除了其表示能力之外，其单一连续设计还提供了一个从数据流图降级到高性能目标特定代码的框架。

本文档定义和描述了 MLIR 中的关键概念，旨在作为简明的参考文档——
[基本原理文档](Rationale/Rationale.md)、
[词汇表](../getting_started/Glossary.md) 及其他内容托管在其他地方。

MLIR 设计用于三种不同的形式：适合调试的人类可读文本形式、适合程序化转换和分析的内存形式，以及适合存储和传输的紧凑序列化形式。不同形式都描述相同的语义内容。本文档描述人类可读的文本形式。

\[TOC\]

## 高层结构

MLIR 从根本上基于节点（称为*操作*）和边（称为*值*）的图状数据结构。每个值恰好是一个操作或块参数的结果，并有一个由[类型系统](#type-system)定义的*值类型*。[操作](#operations)包含在[块](#blocks)中，块包含在[区域](#regions)中。操作在其包含的块中是有序的，块在其包含的区域中也是有序的，但这种顺序在给定的[区域类型](Interfaces.md/#regionkindinterfaces)中可能具有或不具有语义意义。操作也可以包含区域，从而能够表示层次结构。

操作可以表示许多不同的概念，从高层次概念（如函数定义、函数调用、缓冲区分配、缓冲区的视图或切片以及进程创建）到低层次概念（如目标无关的算术运算、目标特定的指令、配置寄存器和逻辑门）。这些不同的概念由 MLIR 中的不同操作表示，MLIR 中可用的操作集合可以任意扩展。

MLIR 还为操作上的转换提供了一个可扩展框架，使用熟悉的编译器 [Pass](Passes.md) 概念。在任意操作集合上启用任意 pass 集合会带来显著的扩展挑战，因为每个转换都可能需要考虑任何操作的语义。MLIR 通过允许使用 [Traits](Traits) 和 [Interfaces](Interfaces.md) 以抽象方式描述操作语义来解决这种复杂性，使转换能够更通用地在操作上运行。Traits 通常描述有效 IR 上的验证约束，使得可以捕获和检查复杂的不变量。（参见
[Op vs Operation](Tutorials/Toy/Ch-2.md/#op-vs-operation-using-mlir-operations)）

MLIR 的一个明显应用是表示基于 [SSA](https://en.wikipedia.org/wiki/Static_single_assignment_form) 的 IR，如 LLVM 核心 IR，通过适当选择操作类型来定义模块、函数、分支、内存分配，并通过验证约束来确保 SSA 支配属性。MLIR 包含了一组定义了此类结构的方言。但是，MLIR 旨在足够通用，以表示其他类似编译器的数据结构，例如语言前端中的抽象语法树、目标特定后端中的生成指令或高层次综合工具中的电路。

以下是一个 MLIR 模块示例：

```mlir
// 使用乘法内核的实现计算 A*B，并使用 TensorFlow 操作打印结果。
// A 和 B 的维度部分已知，形状假设匹配。
func.func @mul(%A: tensor<100x?xf32>, %B: tensor<?x50xf32>) -> (tensor<100x50xf32>) {
  // 使用 dim 操作计算 %A 的内部维度。
  %n = memref.dim %A, 1 : tensor<100x?xf32>

  // 分配可寻址的"缓冲区"，并将张量 %A 和 %B 复制到其中。
  %A_m = memref.alloc(%n) : memref<100x?xf32>
  bufferization.materialize_in_destination %A in writable %A_m
      : (tensor<100x?xf32>, memref<100x?xf32>) -> ()

  %B_m = memref.alloc(%n) : memref<?x50xf32>
  bufferization.materialize_in_destination %B in writable %B_m
      : (tensor<?x50xf32>, memref<?x50xf32>) -> ()

  // 调用函数 @multiply，传入 memref 作为参数，
  // 并获得乘法结果的返回。
  %C_m = call @multiply(%A_m, %B_m)
          : (memref<100x?xf32>, memref<?x50xf32>) -> (memref<100x50xf32>)

  memref.dealloc %A_m : memref<100x?xf32>
  memref.dealloc %B_m : memref<?x50xf32>

  // 将缓冲区数据加载到更高层的"张量"值中。
  %C = memref.tensor_load %C_m : memref<100x50xf32>
  memref.dealloc %C_m : memref<100x50xf32>

  // 调用 TensorFlow 内置函数打印结果张量。
  "tf.Print"(%C){message: "mul result"} : (tensor<100x50xf32>) -> (tensor<100x50xf32>)

  return %C : tensor<100x50xf32>
}

// 将两个 memref 相乘并返回结果的函数。
func.func @multiply(%A: memref<100x?xf32>, %B: memref<?x50xf32>)
          -> (memref<100x50xf32>)  {
  // 计算 %A 的内部维度。
  %n = memref.dim %A, 1 : memref<100x?xf32>

  // 为乘法结果分配内存。
  %C = memref.alloc() : memref<100x50xf32>

  // 乘法循环嵌套。
  affine.for %i = 0 to 100 {
     affine.for %j = 0 to 50 {
        memref.store 0 to %C[%i, %j] : memref<100x50xf32>
        affine.for %k = 0 to %n {
           %a_v  = memref.load %A[%i, %k] : memref<100x?xf32>
           %b_v  = memref.load %B[%k, %j] : memref<?x50xf32>
           %prod = arith.mulf %a_v, %b_v : f32
           %c_v  = memref.load %C[%i, %j] : memref<100x50xf32>
           %sum  = arith.addf %c_v, %prod : f32
           memref.store %sum, %C[%i, %j] : memref<100x50xf32>
        }
     }
  }
  return %C : memref<100x50xf32>
}
```

## 符号表示

MLIR 具有简单而无歧义的语法，使其能够可靠地通过文本形式进行往返转换。这对于编译器开发非常重要——例如，用于理解代码在转换过程中的状态以及编写测试用例。

本文档使用[扩展巴科斯-瑙尔范式（EBNF）](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form)描述语法。

以下是本文档中使用的 EBNF 语法，以黄色框显示。

```
alternation ::= expr0 | expr1 | expr2  // expr0、expr1 或 expr2 之一。
sequence    ::= expr0 expr1 expr2      // expr0 expr1 expr2 的序列。
repetition0 ::= expr*  // 0 次或多次出现。
repetition1 ::= expr+  // 1 次或多次出现。
optionality ::= expr?  // 0 次或 1 次出现。
grouping    ::= (expr) // 括号内的所有内容被组合在一起。
literal     ::= `abcd` // 匹配字面量 `abcd`。
```

代码示例以蓝色框显示。

```
// 这是上述语法的示例用法：
// 匹配如下内容：ba、bana、boma、banana、banoma、bomana...
example ::= `b` (`an` | `om`)* `a`
```

### 通用语法

以下核心语法产生式在本文档中使用：

```
// TODO: 澄清词法分析（词元）和语法分析（语法）之间的划分。
digit     ::= [0-9]
hex_digit ::= [0-9a-fA-F]
letter    ::= [a-zA-Z]
id-punct  ::= [$._-]

integer-literal ::= decimal-literal | hexadecimal-literal
decimal-literal ::= digit+
hexadecimal-literal ::= `0x` hex_digit+
float-literal ::= [-+]?[0-9]+[.][0-9]*([eE][-+]?[0-9]+)?
string-literal  ::= `"` [^"\n\f\v\r]* `"`   TODO: 定义转义规则
```

此处未列出，但 MLIR 确实支持注释。它们使用标准的 BCPL 语法，以 `//` 开头，直到行尾。

### 顶层产生式

```
// 顶层产生式
toplevel := (operation | attribute-alias-def | type-alias-def)*
```

产生式 `toplevel` 是任何解析 MLIR 语法的解析器所解析的顶层产生式。[操作](#operations)、[属性别名](#attribute-value-aliases)和[类型别名](#type-aliases)可以在顶层声明。

### 标识符和关键字

语法：

```
// 标识符
bare-id ::= (letter|[_]) (letter|digit|[_$.])*
bare-id-list ::= bare-id (`,` bare-id)*
value-id ::= `%` suffix-id
alias-name :: = bare-id
suffix-id ::= (digit+ | ((letter|id-punct) (letter|id-punct|digit)*))

symbol-ref-id ::= `@` (suffix-id | string-literal) (`::` symbol-ref-id)?
value-id-list ::= value-id (`,` value-id)*

// 值的使用，例如在操作的操作数列表中。
value-use ::= value-id (`#` decimal-literal)?
value-use-list ::= value-use (`,` value-use)*
```

标识符用于命名值、类型和函数等实体，由 MLIR 代码的编写者选择。标识符可以是描述性的（如 `%batch_size`、`@matmul`），或者在自动生成时可以是非描述性的（如 `%23`、`@func42`）。MLIR 文本文件中可以使用值的标识符名称，但它们不会作为 IR 的一部分持久化——打印器会给它们匿名名称，如 `%42`。

MLIR 通过为标识符添加符号前缀（如 `%`、`#`、`@`、`^`、`!`）来保证标识符永远不会与关键字冲突。在某些无歧义的上下文中（如仿射表达式），标识符不添加前缀，以保持简洁。新的关键字可以添加到 MLIR 的未来版本中，而不会与现有标识符发生冲突。

值标识符的作用域仅限于定义它们的（嵌套）区域，不能在该区域外访问或引用。映射函数中的参数标识符在映射体中有效。特定操作可以进一步限制其区域中哪些标识符在作用域内。例如，具有 [SSA 控制流语义](#control-flow-and-ssacfg-regions)的区域中值的作用域根据标准
[SSA 支配关系](<https://en.wikipedia.org/wiki/Dominator_(graph_theory)>)的定义来约束。另一个例子是 [IsolatedFromAbove trait](Traits/#isolatedfromabove)，它限制了直接访问包含区域中定义的值。

函数标识符和映射标识符与 [Symbols](SymbolsAndSymbolTables.md) 关联，具有依赖于符号属性的作用域规则。

## 方言

方言是与 MLIR 生态系统交互和扩展的机制。它们允许定义新的[操作](#operations)、[属性](#attributes)和[类型](#type-system)。每个方言都有一个唯一的 `namespace`，作为每个定义的属性/操作/类型的前缀。例如，[Affine 方言](Dialects/Affine.md)定义了命名空间：`affine`。

MLIR 允许多个方言（甚至是主树之外的方言）在一个模块中共存。方言由某些 pass 产生和消费。MLIR 提供了一个[框架](DialectConversion.md)用于在不同方言之间及方言内部进行转换。

MLIR 支持的几个方言：

- [Affine 方言](Dialects/Affine.md)
- [Func 方言](Dialects/Func.md)
- [GPU 方言](Dialects/GPU.md)
- [LLVM 方言](Dialects/LLVM.md)
- [SPIR-V 方言](Dialects/SPIR-V.md)
- [Vector 方言](Dialects/Vector.md)

### 目标特定操作

方言提供了一种模块化方式，使目标可以通过 MLIR 直接公开目标特定操作。例如，某些目标通过 LLVM 实现。LLVM 拥有丰富的内部函数，用于某些目标无关操作（如带溢出检查的加法）以及提供对其支持目标的目标特定操作的访问（如向量排列操作）。MLIR 中的 LLVM 内部函数通过以 "llvm." 名称开头的操作来表示。

示例：

```mlir
// LLVM: %x = call {i16, i1} @llvm.sadd.with.overflow.i16(i16 %a, i16 %b)
%x:2 = "llvm.sadd.with.overflow.i16"(%a, %b) : (i16, i16) -> (i16, i1)
```

这些操作仅在以 LLVM 作为后端时有效（例如用于 CPU 和 GPU），并且需要与这些内部函数的 LLVM 定义保持一致。

## 操作

语法：

```
operation             ::= op-result-list? (generic-operation | custom-operation)
                          trailing-location?
generic-operation     ::= string-literal `(` value-use-list? `)`  successor-list?
                          dictionary-properties? region-list? dictionary-attribute?
                          `:` function-type
custom-operation      ::= bare-id custom-operation-format
op-result-list        ::= op-result (`,` op-result)* `=`
op-result             ::= value-id (`:` integer-literal)?
successor-list        ::= `[` successor (`,` successor)* `]`
successor             ::= caret-id (`:` block-arg-list)?
dictionary-properties ::= `<` dictionary-attribute `>`
region-list           ::= `(` region (`,` region)* `)`
dictionary-attribute  ::= `{` (attribute-entry (`,` attribute-entry)*)? `}`
trailing-location     ::= `loc` `(` location `)`
```

MLIR 引入了一个统一的概念，称为*操作*，用于描述许多不同层次的抽象和计算。MLIR 中的操作是完全可扩展的（没有固定的操作列表），并且具有应用特定的语义。例如，MLIR 支持[目标无关操作](Dialects/MemRef.md)、[仿射操作](Dialects/Affine.md)和[目标特定机器操作](#target-specific-operations)。

操作的内部表示很简单：操作由一个唯一字符串标识（如 `dim`、`tf.Conv2d`、`x86.repmovsb`、`ppc.eieio` 等），可以返回零个或多个结果，接受零个或多个操作数，有用于[属性（properties）](#properties)的存储，有一个[属性（attributes）](#attributes)字典，有零个或多个后继，以及零个或多个封闭的[区域](#regions)。通用打印形式包含所有这些元素，使用函数类型来表示结果和操作数的类型。

示例：

```mlir
// 产生两个结果的操作。
// %result 的结果可以通过 <name> `#` <opNo> 语法访问。
%result:2 = "foo_div"() : () -> (f32, i32)

// 为每个结果定义唯一名称的美化形式。
%foo, %bar = "foo_div"() : () -> (f32, i32)

// 调用名为 tf.scramble 的 TensorFlow 函数，带有两个输入
// 和存储在属性中的属性 "fruit"。
%2 = "tf.scramble"(%result#0, %bar) <{fruit = "banana"}> : (f32, i32) -> f32

// 调用带有一些可丢弃属性的操作
%foo, %bar = "foo_div"() {some_attr = "value", other_attr = 42 : i64} : () -> (f32, i32)
```

除了上面的基本语法之外，方言还可以注册已知的操作。这允许这些方言支持用于解析和打印操作的*自定义汇编形式*。在下面列出的操作集中，我们展示了两种形式。

### 内置操作

[builtin 方言](Dialects/Builtin.md)定义了一些 MLIR 方言广泛适用的操作，例如简化方言间/内部转换的通用转换强制操作。此方言还定义了一个顶层 `module` 操作，表示一个有用的 IR 容器。

## 块

语法：

```
block           ::= block-label operation+
block-label     ::= block-id block-arg-list? `:`
block-id        ::= caret-id
caret-id        ::= `^` suffix-id
value-id-and-type ::= value-id `:` type

// 非空的名称和类型列表。
value-id-and-type-list ::= value-id-and-type (`,` value-id-and-type)*

block-arg-list ::= `(` value-id-and-type-list? `)`
```

*块*是操作的列表。在 [SSACFG 区域](#control-flow-and-ssacfg-regions)中，每个块表示一个编译器[基本块](https://en.wikipedia.org/wiki/Basic_block)，块内的指令按顺序执行，终止操作实现基本块之间的控制流分支。

块中的最后一个操作必须是[终止操作](#control-flow-and-ssacfg-regions)。如果包含操作附加了 `NoTerminator` trait，则具有单个块的区域可以选择退出此要求。顶层 `ModuleOp` 就是一个定义了此 trait 并且其块体没有终止符的操作示例。

MLIR 中的块接受一组块参数，以类似函数的方式表示。块参数绑定到由各个操作语义指定的值。区域的入口块的块参数也是区域的参数，绑定到这些参数的值由包含操作的语义决定。其他块的块参数由终止操作的语义决定，例如分支操作，这些操作将块作为后继。在具有[控制流](#control-flow-and-ssacfg-regions)的区域中，MLIR 利用此结构隐式表示控制流相关值的传递，而无需传统 SSA 表示中 PHI 节点的复杂细节。请注意，不依赖控制流的值可以直接引用，不需要通过块参数传递。

以下是一个展示分支、返回和块参数的简单示例函数：

```mlir
func.func @simple(i64, i1) -> i64 {
^bb0(%a: i64, %cond: i1): // 由 ^bb0 支配的代码可以引用 %a
  cf.cond_br %cond, ^bb1, ^bb2

^bb1:
  cf.br ^bb3(%a: i64)    // 分支传递 %a 作为参数

^bb2:
  %b = arith.addi %a, %a : i64
  cf.br ^bb3(%b: i64)    // 分支传递 %b 作为参数

// ^bb3 从前驱接收一个名为 %c 的参数，
// 并将其与 %a 一起传递给 bb4。%a 直接从其定义操作引用，
// 不通过 ^bb3 的参数传递。
^bb3(%c: i64):
  cf.br ^bb4(%c, %a : i64, i64)

^bb4(%d : i64, %e : i64):
  %0 = arith.addi %d, %e : i64
  return %0 : i64   // return 也是终止符。
}
```

**上下文：**"块参数"表示消除了 IR 中的许多特殊情况，与传统的"PHI 节点是操作"的 SSA IR（如 LLVM）相比。例如，SSA 的[并行复制语义](https://ieeexplore.ieee.org/document/4907656)立即显现，函数参数不再是特殊情况：它们成为入口块的参数
\[[更多基本原理](Rationale/Rationale.md/#block-arguments-vs-phi-nodes)\]。块也是一个基本概念，无法由操作表示，因为在操作中定义的值无法在操作外部访问。

## 区域

### 定义

区域是 MLIR [块](#blocks)的有序列表。区域内的语义不由 IR 强加。相反，包含操作定义了其所包含区域的语义。MLIR 目前定义了两种区域：描述块之间控制流的 [SSACFG 区域](#control-flow-and-ssacfg-regions)，以及不需要块之间控制流的[图区域](#graph-regions)。操作中区域的类型使用 [RegionKindInterface](Interfaces.md/#regionkindinterfaces) 来描述。

区域没有名称或地址，只有区域中包含的块才有。区域必须包含在操作中，没有类型或属性。区域中的第一个块是一个特殊块，称为"入口块"。入口块的参数也是区域本身的参数。入口块不能列为任何其他块的后继。区域的语法如下：

```
region      ::= `{` entry-block? block* `}`
entry-block ::= operation+
```

函数体是区域的一个例子：它由一个块的 CFG 组成，并具有其他类型的区域可能没有的额外语义限制。例如，在函数体中，块终止符必须分支到不同的块，或者从函数返回，其中 `return` 参数的类型必须与函数签名的结果类型匹配。类似地，函数参数必须与区域参数的类型和数量匹配。一般来说，带有区域的操作可以任意定义这些对应关系。

*入口块*是一个没有标签和参数、可以出现在区域开头的块。它实现了使用区域打开新作用域的常见模式。

### 值作用域

区域提供了程序的层次封装：不可能引用（即分支到）不在引用源所在的同一区域中的块（即终止操作）。同样，区域为值可见性提供了自然作用域：在区域中定义的值不会逃逸到封闭区域（如果有的话）。默认情况下，区域内的操作可以引用在区域外定义的值，只要包含操作的操作数引用这些值是合法的，但这可以通过 trait（例如 [OpTrait::IsolatedFromAbove](Traits/#isolatedfromabove)）或自定义验证器来限制。

示例：

```mlir
  "any_op"(%a) ({ // 如果 %a 在包含区域中有效...
     // 那么 %a 在这里也有效。
    %new_value = "another_op"(%a) : (i64) -> (i64)
  }) : (i64) -> (i64)
```

MLIR 定义了一个通用的"层次支配"概念，该概念跨层次操作，并定义值是否"在作用域内"，可以被特定操作使用。在同一区域中另一操作是否可以使用某个值，由区域的类型决定。如果某个父区域可以使用某个值，那么在同一区域中有父的操作也可以使用该值。区域参数定义的值始终可以被区域中深度嵌套的任何操作使用。在区域中定义的值永远不能在区域外使用。

### 控制流与 SSACFG 区域

在 MLIR 中，区域的控制流语义由
[RegionKind::SSACFG](Interfaces.md/#regionkindinterfaces) 表示。非正式地说，这些区域支持区域中的操作"顺序执行"的语义。在操作执行之前，其操作数具有明确定义的值。在操作执行之后，操作数具有相同的值，结果也具有明确定义的值。在操作执行之后，块中的下一个操作执行，直到操作是块末尾的终止操作，此时某个其他操作将执行。确定下一个要执行的指令是"传递控制流"。

通常，当控制流传递给一个操作时，MLIR 不限制控制流何时进入或退出该操作所包含的区域。但是，当控制流进入一个区域时，它总是从区域的第一个块（称为*入口*块）开始。每个块末尾的终止操作通过显式指定块的后继块来表示控制流。控制流只能传递到指定的后继块之一（如在 `branch` 操作中），或返回到包含操作（如在 `return` 操作中）。没有后继的终止操作只能将控制权传回给包含操作。在这些限制内，终止操作的特定语义由所涉及的特定方言操作决定。不作为终止操作后继列出的块（除入口块外）被定义为不可达，可以在不影响包含操作语义的情况下删除。

虽然控制流总是通过入口块进入区域，但控制流可以通过任何具有适当终止符的块退出区域。标准方言利用此功能定义具有单入口多出口（SEME）区域的操作，可能流经区域中的不同块并通过任何带有 `return` 操作的块退出。这种行为类似于大多数编程语言中函数体的行为。此外，控制流也可能无法到达块或区域的末尾，例如当函数调用没有返回时。

示例：

```mlir
func.func @accelerator_compute(i64, i1) -> i64 { // SSACFG 区域
^bb0(%a: i64, %cond: i1): // 由 ^bb0 支配的代码可以引用 %a
  cf.cond_br %cond, ^bb1, ^bb2

^bb1:
  // %value 的定义不支配 ^bb2
  %value = "op.convert"(%a) : (i64) -> i64
  cf.br ^bb3(%a: i64)    // 分支传递 %a 作为参数

^bb2:
  accelerator.launch() { // SSACFG 区域
    ^bb0:
      // 嵌套在 "accelerator.launch" 下的代码区域，可以引用 %a 但
      // 不能引用 %value。
      %new_value = "accelerator.do_something"(%a) : (i64) -> ()
  }
  // %new_value 不能在区域外引用

^bb3:
  ...
}
```

#### 包含多个区域的操作

包含多个区域的操作也完全决定这些区域的语义。特别是，当控制流传递给一个操作时，它可以将控制流传递给任何包含的区域。当控制流从区域中退出并返回到包含操作时，包含操作可以将控制流传递给同一操作中的任何区域。操作也可以同时将控制流传递给多个包含的区域。操作还可以将控制流传递给其他操作中指定的区域，特别是那些定义了给定操作使用的值或符号的操作（如调用操作）。这种控制传递通常独立于通过包含区域的基本块的控制流传递。

#### 闭包

区域允许定义创建闭包的操作，例如通过将区域体"装箱"到它们产生的值中。定义其语义仍然由操作负责。请注意，如果操作触发了区域的异步执行，操作调用者有责任等待区域执行完毕，以保证任何直接使用的值保持存活。

### 图区域

在 MLIR 中，区域中类图的语义由
[RegionKind::Graph](Interfaces.md/#regionkindinterfaces) 表示。图区域适用于没有控制流的并发语义，或用于建模通用有向图数据结构。图区域适用于表示耦合值之间没有基本顺序关系的循环关系。例如，图区域中的操作可能表示具有表示数据流的值的独立控制线程。与 MLIR 中通常一样，区域的特定语义完全由其包含操作决定。图区域只能包含单个基本块（入口块）。

**基本原理：**目前图区域任意限制为单个基本块，尽管这个限制没有特别的语义原因。添加此限制是为了更容易稳定 pass 基础设施和常用于处理图区域以正确处理反馈循环的 pass。如果出现需要多块区域的用例，将来可能会允许多块区域。

在图区域中，MLIR 操作自然表示节点，而每个 MLIR 值表示连接单个源节点和多个目标节点的多边。区域中所有作为操作结果定义的值都在区域内有效，可以被区域中的任何其他操作访问。在图区域中，块内操作的顺序和区域内块的顺序在语义上没有意义，非终止符操作可以自由重新排序，例如通过规范化。其他类型的图，例如具有多个源节点和多个目标节点的图，也可以通过将图边表示为 MLIR 操作来表示。

注意，循环可以在图区域中的单个块内发生，也可以在基本块之间发生。

```mlir
"test.graph_region"() ({ // 图区域
  %1 = "op1"(%1, %3) : (i32, i32) -> (i32)  // 合法：%1、%3 在此处允许
  %2 = "test.ssacfg_region"() ({
     %5 = "op2"(%1, %2, %3, %4) : (i32, i32, i32, i32) -> (i32) // 合法：%1、%2、%3、%4 都在包含区域中定义
  }) : () -> (i32)
  %3 = "op2"(%1, %4) : (i32, i32) -> (i32)  // 合法：%4 在此处允许
  %4 = "op3"(%1) : (i32) -> (i32)
}) : () -> ()
```

### 参数和结果

区域第一个块的参数被视为区域的参数。这些参数的来源由父操作的语义定义。它们可能对应于操作本身使用的某些值。

区域产生一个（可能为空的）值列表。操作语义定义了区域结果与操作结果之间的关系。

## 类型系统

MLIR 中的每个值都有一个由类型系统定义的类型。MLIR 具有开放的类型系统（即没有固定的类型列表），类型可以具有应用特定的语义。MLIR 方言可以定义任意数量的类型，对其表示的抽象没有限制。

```
type ::= type-alias | dialect-type | builtin-type

type-list-no-parens ::=  type (`,` type)*
type-list-parens ::= `(` `)`
                   | `(` type-list-no-parens `)`

// 这是引用具有指定类型的值的常用方式。
ssa-use-and-type ::= ssa-use `:` type
ssa-use ::= value-use

// 非空的名称和类型列表。
ssa-use-and-type-list ::= ssa-use-and-type (`,` ssa-use-and-type)*

function-type ::= (type | type-list-parens) `->` (type | type-list-parens)
```

### 类型别名

```
type-alias-def ::= `!` alias-name `=` type
type-alias ::= `!` alias-name
```

MLIR 支持为类型定义命名别名。类型别名是一个标识符，可以用来代替它所定义的类型。这些别名*必须*在使用之前定义。别名名称不能包含 '.'，因为这些名称保留给[方言类型](#dialect-types)。

示例：

```mlir
!avx_m128 = vector<4 x f32>

// 使用原始类型。
"foo"(%x) : vector<4 x f32> -> ()

// 使用类型别名。
"foo"(%x) : !avx_m128 -> ()
```

### 方言类型

与操作类似，方言可以定义对类型系统的自定义扩展。

```
dialect-namespace ::= bare-id

dialect-type ::= `!` (opaque-dialect-type | pretty-dialect-type)
opaque-dialect-type ::= dialect-namespace dialect-type-body
pretty-dialect-type ::= dialect-namespace `.` pretty-dialect-type-lead-ident
                                              dialect-type-body?
pretty-dialect-type-lead-ident ::= `[A-Za-z][A-Za-z0-9._]*`

dialect-type-body ::= `<` dialect-type-contents+ `>`
dialect-type-contents ::= dialect-type-body
                            | `(` dialect-type-contents+ `)`
                            | `[` dialect-type-contents+ `]`
                            | `{` dialect-type-contents+ `}`
                            | [^\[<({\]>)}\0]+
```

方言类型通常以不透明形式指定，其中类型的内容在用方言命名空间和 `<>` 包裹的主体中定义。考虑以下示例：

```mlir
// TensorFlow 字符串类型。
!tf<string>

// 具有复杂组件的类型。
!foo<something<abcd>>

// 更复杂的类型。
!foo<"a123^^^" + bar>
```

足够简单的方言类型可以使用更美观的格式，它将部分语法展开为等效但更轻量的形式：

```mlir
// TensorFlow 字符串类型。
!tf.string

// 具有复杂组件的类型。
!foo.something<abcd>
```

参见[此处](DefiningDialects/AttributesAndTypes.md)了解如何定义方言类型。

### 内置类型

[builtin 方言](Dialects/Builtin.md)定义了一组可被 MLIR 中任何其他方言直接使用的类型。这些类型涵盖了从原始整数和浮点类型、函数类型等各种范围。

## 属性（Properties）

属性（Properties）是直接存储在操作类上的额外数据成员。它们提供了一种存储[固有属性（inherent attributes）](#attributes)和其他任意数据的方式。数据的语义特定于给定的操作，可以通过 [Interfaces](Interfaces.md) 访问器和其他方法公开。属性（Properties）始终可以序列化为 Attribute 以便通用打印。

## 属性（Attributes）

语法：

```
attribute-entry ::= (bare-id | string-literal) `=` attribute-value
attribute-value ::= attribute-alias | dialect-attribute | builtin-attribute
```

属性（Attributes）是在不允许变量的地方指定操作上常量数据的机制——例如 [`cmpi` 操作](Dialects/ArithOps.md/#arithcmpi-arithcmpiop)的比较谓词。每个操作都有一个属性字典，将一组属性名称与属性值关联。MLIR 的内置方言提供了一组丰富的[内置属性值](#builtin-attribute-values)（如数组、字典、字符串等）。此外，方言还可以定义自己的[方言属性值](#dialect-attribute-values)。

对于尚未采用属性（Properties）的方言，附加到操作的顶层属性字典具有特殊语义。属性条目根据其字典键是否具有方言前缀分为两种不同类型：

- *固有属性*（inherent attributes）是操作语义定义所固有的。操作本身预期验证这些属性的一致性。一个例子是 `arith.cmpi` 操作的 `predicate` 属性。这些属性的名称不能以方言前缀开头。

- *可丢弃属性*（discardable attributes）在操作本身之外具有语义定义，但必须与操作的语义兼容。这些属性的名称必须以方言前缀开头。方言前缀所指示的方言预期验证这些属性。一个例子是 `gpu.container_module` 属性。

请注意，属性值本身允许是字典属性，但只有附加到操作的顶层字典属性受到上述分类的约束。

采用属性（Properties）后，只有可丢弃属性存储在顶层字典中，而固有属性存储在属性（Properties）存储中。

### 属性值别名

```
attribute-alias-def ::= `#` alias-name `=` attribute-value
attribute-alias ::= `#` alias-name
```

MLIR 支持为属性值定义命名别名。属性别名是一个标识符，可以用来代替它所定义的属性。这些别名*必须*在使用之前定义。别名名称不能包含 '.'，因为这些名称保留给[方言属性](#dialect-attribute-values)。

示例：

```mlir
#map = affine_map<(d0) -> (d0 + 10)>

// 使用原始属性。
%b = affine.apply affine_map<(d0) -> (d0 + 10)> (%a)

// 使用属性别名。
%b = affine.apply #map(%a)
```

### 方言属性值

与操作类似，方言可以定义自定义属性值。

```
dialect-namespace ::= bare-id

dialect-attribute ::= `#` (opaque-dialect-attribute | pretty-dialect-attribute)
opaque-dialect-attribute ::= dialect-namespace dialect-attribute-body
pretty-dialect-attribute ::= dialect-namespace `.` pretty-dialect-attribute-lead-ident
                                              dialect-attribute-body?
pretty-dialect-attribute-lead-ident ::= `[A-Za-z][A-Za-z0-9._]*`

dialect-attribute-body ::= `<` dialect-attribute-contents+ `>`
dialect-attribute-contents ::= dialect-attribute-body
                            | `(` dialect-attribute-contents+ `)`
                            | `[` dialect-attribute-contents+ `]`
                            | `{` dialect-attribute-contents+ `}`
                            | [^\[<({\]>)}\0]+
```

方言属性通常以不透明形式指定，其中属性的内容在用方言命名空间和 `<>` 包裹的主体中定义。考虑以下示例：

```mlir
// 字符串属性。
#foo<string<"">>

// 复杂属性。
#foo<"a123^^^" + bar>
```

足够简单的方言属性可以使用更美观的格式，它将部分语法展开为等效但更轻量的形式：

```mlir
// 字符串属性。
#foo.string<"">
```

参见[此处](DefiningDialects/AttributesAndTypes.md)了解如何定义方言属性值。

### 内置属性值

[builtin 方言](Dialects/Builtin.md)定义了一组可被 MLIR 中任何其他方言直接使用的属性值。这些类型涵盖了从原始整数和浮点值、属性字典、稠密多维数组等各种范围。

### IR 版本控制

方言可以通过 `BytecodeDialectInterface` 选择加入版本控制处理。向方言公开了少量钩子以允许管理编码到字节码文件中的版本。版本延迟加载，允许在解析输入 IR 时检索版本信息，并通过 `upgradeFromVersion` 方法为存在版本的每个方言提供在解析后执行 IR 升级的机会。自定义属性和类型编码也可以根据方言版本使用 readAttribute 和 readType 方法进行升级。

对于方言允许编码的版本控制信息类型没有限制。目前，版本控制仅支持字节码格式。
