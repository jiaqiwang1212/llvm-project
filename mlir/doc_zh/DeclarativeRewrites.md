# 表格驱动的声明式重写规则（DRR）

除了继承 `mlir::RewritePattern` C++ 类之外，MLIR 还支持以声明式方式定义重写规则。与
[操作定义规范](DefiningDialects/Operations.md)（ODS）类似，这通过
[TableGen][TableGen] 实现，TableGen 是一种用于维护领域特定信息记录的语言。重写规则以简洁的形式写在
TableGen 记录中，这些记录在编译器构建时会被展开为等效的 `mlir::RewritePattern` 子类。

本手册详细解释了以声明式方式定义重写规则的所有可用机制。它旨在作为规范而非教程。请参阅
[MLIR 图重写快速入门教程](Tutorials/QuickstartRewrites.md)获取教程内容。

由于声明式重写规则依赖于操作定义规范，本手册假设读者已了解 [ODS](DefiningDialects/Operations.md) 文档。

[TOC]

## 优势

与手写 C++ 类相比，这种声明式方法有若干优势，包括但不限于：

*   **声明式**：模式创建者只需以声明式方式说明重写模式，无需担心具体调用哪些 C++ 方法。
*   **减少样板代码，展示重写的本质**：
    `mlir::RewritePattern` 已经很好地隐藏了定义重写规则的样板代码。但我们仍需编写
    C++ 编程语言所要求的类和函数结构，检查操作以进行匹配，并调用操作的 `build()` 方法来构造操作。
    这些语句通常非常简单且相似，因此可以通过自动生成进一步压缩。
    通过将样板代码减少到最低限度，声明式重写规则将只包含重写的核心本质，使得模式非常易于理解。

## 优势与局限性

声明式重写规则是**基于操作**的：它描述了一条规则，用于匹配有向无环图（DAG）中的操作，并生成操作的 DAG。
这赋予了 DRR 其优势和局限性：它擅长表达操作到操作的转换，但不太适合表达将操作转换为循环嵌套这类情况。

按当前实现，DRR 对以下特性的支持不够完善：

*   匹配和生成带有 Region 的操作。
*   匹配和生成带有块参数的操作。
*   在嵌套模式中匹配多结果操作。
*   在嵌套模式中匹配和生成可变参数操作数/结果的操作。
*   在生成过程中打包和解包可变参数操作数/结果。
*   [`NativeCodeCall`](#nativecodecall-transforming-the-generated-op) 返回多个结果。

## 规则定义

定义重写规则的核心构造在
[`PatternBase.td`][PatternBase] 中定义如下：

```tablegen
class Pattern<
    dag sourcePattern, list<dag> resultPatterns,
    list<dag> additionalConstraints = [],
    list<dag> supplementalPatterns = [],
    dag benefitsAdded = (addBenefit 0)>;
```

一个声明式重写规则包含两个主要组件：

*   一个*源模式*，用于匹配操作的 DAG。
*   一个或多个*结果模式*，用于生成操作的 DAG 以替换匹配的操作 DAG。

我们允许多个结果模式来支持
[多结果操作](#supporting-multi-result-ops) 和
[辅助操作](#supporting-auxiliary-ops)，但通常我们只想将一个操作 DAG 转换为另一个操作 DAG。`Pattern` 有一个方便的包装类 `Pat`，它接受单个结果模式：

```tablegen
class Pat<
    dag sourcePattern, dag resultPattern,
    list<dag> additionalConstraints = [],
    dag benefitsAdded = (addBenefit 0)> :
  Pattern<sourcePattern, [resultPattern], additionalConstraints, benefitAdded>;
```

每个模式都被指定为一个 TableGen `dag` 对象，语法为 `(operator arg0, arg1, ...)`。

`operator` 通常是一个 MLIR 操作，但也可以是其他
[指令](#rewrite-directives)。`argN` 用于匹配（如果用于源模式）或生成（如果用于结果模式）`operator` 的第 `N` 个参数。如果 `operator` 是某个 MLIR 操作，则表示操作定义的 `arguments` 列表中指定的第 `N` 个参数。因此，我们说模式中的操作参数规范是**基于位置**的：它们出现的位置很重要。

`argN` 本身可以是一个 `dag` 对象，因此我们可以用嵌套的 `dag` 树来模拟操作之间的定义-使用关系。

### 源模式

源模式用于匹配操作的 DAG。`dag` 对象中的参数旨在**捕获**操作参数。它们也可以用于**进一步限制**匹配条件。捕获通过指定以 `$` 符号开头的符号来完成，而进一步的约束则通过指定 `TypeConstraint`（用于操作数）、`AttrConstraint`（用于属性）或 `PropConstraint`（用于属性）来引入。

#### 绑定操作参数并限制匹配

例如：

```tablegen
def AOp : Op<"a_op"> {
    let arguments = (ins
      AnyType:$a_input,
      AnyAttr:$a_attr
    );

    let results = (outs
      AnyType:$a_output
    );
}

def : Pat<(AOp $input, F32Attr:$attr), ...>;
```

在上面的例子中，我们正在匹配一个 `AOp`，其 `$input` 可以是操作定义中任何有效的值，而 `$attr` 必须是一个浮点属性。如果匹配成功，我们将 `$input` 符号绑定到操作的唯一输入（`$a_input`），将 `$attr` 绑定到唯一属性（`$a_attr`）；我们可以在结果模式和附加约束中使用 `$input` 和 `$attr` 来引用它们。

模式是基于位置的：这里用于捕获的符号名称不需要与操作定义中的名称匹配，如上例所示。另一个例子，模式可以写成 `def : Pat<(AOp $a, F32Attr:$b), ...>;`，并使用 `$a` 和 `$b` 来引用捕获的输入和属性。但直接在模式中使用 ODS 名称也是允许的。源模式中的操作数可以具有相同的名称，这将一个操作数绑定到该名称，同时验证其余操作数都相等。

另外请注意，只有在需要进一步限制匹配条件时，才需要添加 `TypeConstraint` 或 `AttributeConstraint`。如果操作的所有有效情况都可以接受，则可以不指定约束。

`$_` 是一个特殊符号，表示忽略对某个参数的捕获。例如，`def : Pat<(AOp $_, $b), ...>` 表示只有 `$b` 需要捕获并在后续结果模式中引用。即使符号不需要捕获，也可以放置附加约束；对于这种情况，可以只使用 `TypeConstraint` 或 `AttributeConstraint` 而不带绑定符号，例如 `def : Pat<(AOp $a, F32Attr), ...>`。

#### 匹配操作的 DAG

要匹配操作的 DAG，请使用嵌套的 `dag` 对象：

```tablegen

def BOp : Op<"b_op"> {
    let arguments = (ins);

    let results = (outs
      AnyType:$b_output
    );
}


def : Pat<(AOp (BOp), $attr), ...>;
```

上面的模式匹配一个 `AOp`，其唯一操作数由 `BOp` 生成，即以下 MLIR 代码：

```mlir
%0 = "b_op"() : () -> (...)
%1 = "a_op"(%0) {attr: ...} : () -> (...)
```

#### 绑定操作结果

要将符号绑定到匹配操作的结果以便后续引用，请将符号附加到操作本身：

```tablegen
def : Pat<(AOp (BOp:$b_result), $attr), ...>;
```

上面的代码将 `$b_result` 绑定到匹配的 `BOp` 的结果。（关于多结果操作有更多细节，将在
[后面](#supporting-multi-result-ops)介绍。）

### 结果模式

结果模式用于生成操作的 DAG。`dag` 对象中的参数旨在**引用**源模式中捕获的值，并可能**应用转换**。

#### 引用绑定的符号

例如：

```tablegen
def COp : Op<"c_op"> {
    let arguments = (ins
      AnyType:$c_input,
      AnyAttr:$c_attr
    );

    let results = (outs
      AnyType:$c_output
    );
}

def : Pat<(AOp $input, $attr), (COp $input, $attr)>;
```

在上面的例子中，`AOp` 的唯一操作数和属性分别绑定到 `$input` 和 `$attr`。然后我们在结果模式中引用它们，将它们作为参数传递给 `COp` 的 `build()` 方法来生成 `COp`。

我们也可以引用绑定到匹配操作结果的符号：

```tablegen
def : Pat<(AOp (BOp:$b_result) $attr), (COp $b_result $attr)>;
```

在上面的例子中，我们使用 `BOp` 的结果来构建 `COp`。

#### 构建操作

由于 `COp` 是通过表格驱动的操作定义来指定的，因此将为其生成多个 `build()` 方法。其中一个在签名中聚合了结果类型、操作数和属性的参数：`void COp::build(..., ArrayRef<Type> resultTypes, Array<Value> operands, const COp::Properties& properties)`。上面的模式调用此 `build()` 方法来构造 `COp`。

通常，结果模式中的参数会直接传递给 `build()` 方法，以利用自动生成的 `build()` 方法，请按照与 ODS `arguments` 定义完全相同的顺序在模式中列出它们。否则，需要一个与参数列表匹配的自定义 `build()` 方法。

目前，所有 ODS 生成的 `build()` 方法都需要指定结果类型，除非操作具有类似 `SameOperandsAndResultType` 这样已知的 trait，使我们可以自动生成具有结果类型推断能力的 `build()` 方法。在生成操作以替换匹配的根操作的结果时，我们可以在调用 ODS 生成的构建器时使用匹配的根操作的结果类型。否则（例如，生成[辅助操作](#supporting-auxiliary-ops)或生成带有嵌套结果模式的操作），DRR 将无法推断结果类型。模式作者需要在 ODS 中通过 `OpBuilder` 定义一个具有结果类型推断能力的自定义构建器。例如，在以下模式中：

```tablegen
def : Pat<(AOp $input, $attr), (COp (AOp $input, $attr) $attr)>;
```

`AOp` 是通过嵌套结果模式生成的；DRR 无法为其推断结果类型。应为 `AOp` 定义一个自定义构建器，它应该自行推断结果类型。构建器应该为每个操作数和属性设置单独的参数，并在内部自行推断结果类型。例如，对于上面的 `AOp`，一个可能的构建器是：

```c++

void AOp::build(OpBuilder &builder, OperationState &state,
                Value input, Attribute attr) {
  state.addOperands({input});
  state.addAttribute("a_attr", attr);
  Type type = ...; // 在此推断结果类型
  state.addTypes({type});
}
```

如果没有定义这样的构建器，将在 C++ 编译时产生错误，提示由于参数数量不匹配，对 `AOp::build()` 的调用无法解析。

#### 生成操作的 DAG

`dag` 对象可以嵌套以生成操作的 DAG：

```tablegen
def : Pat<(AOp $input, $attr), (COp (BOp), $attr)>;
```

在上面的例子中，我们生成一个 `BOp`，然后使用其结果来生成 `COp` 以替换匹配的 `AOp`。

#### 绑定操作结果

在结果模式中，我们可以通过将符号附加到操作来绑定新构建操作的结果。（但我们**不能**绑定到操作参数，因为它们引用了之前绑定的符号。）这对于在适当位置重用新创建的结果很有用。例如：

```tablegen
def DOp : Op<"d_op"> {
    let arguments = (ins
      AnyType:$d_input1,
      AnyType:$d_input2,
    );

    let results = (outs
      AnyType:$d_output
    );
}

def : Pat<(AOp $input, $ignored_attr), (DOp (BOp:$b_result) $b_result)>;
```

在这个模式中，匹配了一个 `AOp` 并用 `DOp` 替换，`DOp` 的两个操作数都来自单个 `BOp` 的结果。这只有通过将 `BOp` 的结果绑定到一个名称并将其重用为 `DOp` 的第二个操作数才能实现。

#### `NativeCodeCall`：转换生成的操作

有时捕获的参数并不是我们想要的确切形式，因此不能直接用作构建新操作的参数。对于这类情况，我们可以通过调用 C++ 辅助函数对参数应用转换。这通过 `NativeCodeCall` 实现。

例如，如果我们想捕获某个操作的属性并将它们组合为一个数组属性以构造一个新操作：

```tablegen

def TwoAttrOp : Op<"two_attr_op"> {
    let arguments = (ins
      AnyAttr:$op_attr1,
      AnyAttr:$op_attr2
    );

    let results = (outs
      AnyType:$op_output
    );
}

def OneAttrOp : Op<"one_attr_op"> {
    let arguments = (ins
      ArrayAttr:$op_attr
    );

    let results = (outs
      AnyType:$op_output
    );
}
```

我们可以编写一个 C++ 辅助函数：

```c++
ArrayAttr createArrayAttr(Builder &builder, Attribute a, Attribute b) {
  return builder.getArrayAttr({a, b});
}
```

然后将模式写成：

```tablegen
def createArrayAttr : NativeCodeCall<"createArrayAttr($_builder, $0, $1)">;

def : Pat<(TwoAttrOp $attr1, $attr2),
          (OneAttrOp (createArrayAttr $attr1, $attr2))>;
```

并确保上述模式生成的 C++ 代码能够访问 C++ 辅助函数的定义。

在上面的例子中，我们使用一个字符串来特化 `NativeCodeCall` 模板。该字符串可以是任意 C++ 表达式，计算结果为 `NativeCodeCall` 调用点所期望的某个 C++ 对象（这里期望是一个数组属性）。通常该字符串应该是一个函数调用。

在属性（properties）的情况下，`NativeCodeCall` 的返回值应该是属性的_接口_类型。例如，`StringProp` 的 `NativeCodeCall` 应该返回一个 `StringRef`，它将被复制到底层的 `std::string` 中，就像它是操作构建器的参数一样。

##### `NativeCodeCall` 占位符

在 `NativeCodeCall` 中，我们可以使用占位符，如 `$_builder`、`$N` 和 `$N...`。前者称为*特殊占位符*，后者称为*位置占位符*和*位置范围占位符*。

`NativeCodeCall` 目前只支持三个特殊占位符：`$_builder`、`$_loc` 和 `$_self`：

*   `$_builder` 将被替换为当前的 `mlir::PatternRewriter`。
*   `$_loc` 将被替换为融合位置或自定义位置（由位置指令确定）。
*   `$_self` 将被替换为源模式中的定义操作。

我们已经看到了 `$_builder` 的用法；它允许我们将 `mlir::Builder`（`mlir::PatternRewriter` 是 `mlir::OpBuilder` 的子类，而 `mlir::OpBuilder` 是 `mlir::Builder` 的子类）传递给 C++ 辅助函数，以使用 `mlir::Builder` 上的便捷方法。

以下是在源模式中如何使用 `$_self` 的示例：

```tablegen

def : Pat<(OneAttrOp (NativeCodeCall<"Foo($_self, &$0)"> I32Attr:$val)),
          (TwoAttrOp $val, $val)>;
```

在上面的例子中，`$_self` 被 `OneAttrOp` 第一个操作数的定义操作替换。请注意，我们不支持在源模式中将名称绑定到 `NativeCodeCall`。要从辅助函数中携带一些返回值，请将名称（约束是可选的）放入参数列表中，它们将绑定到相应类型的变量。然后这些名称必须通过引用或指针传递给用作参数的变量，以便可以返回匹配的值。在同一个例子中，`$val` 将绑定到一个 `Attribute` 类型的变量（作为 `I32Attr`），`Foo()` 中第二个参数的类型可以是 `Attribute&` 或 `Attribute*`。具有属性约束的名称将被捕获为 `Attribute`，具有属性约束的名称（必须具有具体的接口类型）将被视为该类型，其他所有内容将被视为 `Value`。

位置占位符将被 `NativeCodeCall` 使用位置处的 `dag` 对象参数替换。例如，如果我们定义 `SomeCall : NativeCodeCall<"someFn($1, $2, $0)">` 并像 `(SomeCall $in0, $in1, $in2)` 这样使用它，那么这将被翻译为 C++ 调用 `someFn($in1, $in2, $in0)`。

对于属性（properties）的情况，占位符将绑定到属性的_接口_类型的值。例如，将 `StringProp` 作为参数传递给 `NativeCodeCall` 将传递一个 `StringRef`（就像调用匹配操作的 getter 一样），而不是 `std::string`。请参阅 `mlir/include/mlir/IR/Properties.td` 了解接口类型与存储类型的详细信息。

位置范围占位符将被 `NativeCodeCall` 使用位置处的多个 `dag` 对象参数替换。例如，如果我们定义 `SomeCall : NativeCodeCall<"someFn($1...)">` 并像 `(SomeCall $in0, $in1, $in2)` 这样使用它，那么这将被翻译为 C++ 调用 `someFn($in1, $in2)`。

##### `NativeCodeCall` 绑定多个结果

要绑定多个结果并使用 `$<name>__N` 访问第 N 个结果，请在模板中指定返回值的数量。注意，多个结果绑定只支持 `Value` 类型。例如：

```tablegen

def PackAttrs : NativeCodeCall<"packAttrs($0, $1)", 2>;
def : Pattern<(TwoResultOp $attr1, $attr2),
              [(OneResultOp (PackAttr:$res__0, $attr1, $attr2)),
               (OneResultOp $res__1)]>;

```

对于没有返回值的情况，使用 `NativeCodeCallVoid`。

在 `NativeCodeCall` 中指定正确的返回值数量非常重要。它将用于验证返回值数量的一致性。此外，`mlir-tblgen` 将尝试在生成的代码中捕获 `NativeCodeCall` 的返回值，这样如果一个不返回任何结果的 `NativeCodeCall` 没有标记为返回 0 个值，就会在后面触发编译错误。

##### 自定义整个操作构建过程

`NativeCodeCall` 不仅限于为构建操作转换参数；它也可以用于指定如何完整地构建一个操作。示例如下：

如果我们有一个用于构建操作的 C++ 函数：

```c++
Operation *createMyOp(OpBuilder builder, Value input, Attribute attr);
```

我们可以将其包装并像这样调用：

```tablegen
def createMyOp : NativeCodeCall<"createMyOp($_builder, $0, $1)">;

def : Pat<(... $input, $attr), (createMyOp $input, $attr)>;
```

### 支持辅助操作

声明式重写规则支持多个结果模式。其目的之一是允许生成*辅助操作*。辅助操作是用于构建替换操作的操作；但它们本身不直接用于替换。

对于单结果操作，如果有多个结果模式，只有最后一个结果模式生成的值将用于替换匹配的根操作的结果；所有其他结果模式将被视为生成辅助操作。

通常我们希望将操作指定为嵌套的 `dag` 对象，如果它们的定义-使用关系可以表示为一个操作的结果可以作为使用操作的参数的方式。但这并不总是可能的。例如，如果我们想分配内存并存储某些计算结果（伪代码）：

```mlir
%dst = arith.addi %lhs, %rhs
```

转换为：

```mlir
%shape = shape %lhs
%mem = memref.alloc %shape
%sum = arith.addi %lhs, %rhs
memref.store %mem, %sum
%dst = memref.load %mem
```

由于 `store` 不返回值，我们不能只用一个结果模式。可以改用多个结果模式：

```tablegen
def : Pattern<(AddIOp $lhs, $rhs),
              [(StoreOp (AllocOp:$mem (ShapeOp $lhs)), (AddIOp $lhs, $rhs)),
               (LoadOp $mem)];
```

在上面的例子中，我们使用第一个结果模式来生成前四个操作，使用最后一个模式来生成最后一个操作，该操作用于替换匹配的操作。

### 支持多结果操作

多结果操作给声明式重写规则带来了额外的复杂性。我们使用 TableGen `dag` 对象来表示模式中的操作；没有原生方法来表示一个操作生成多个结果。采用的方法基于**命名约定**：在符号后添加 `__N` 后缀来表示第 `N` 个结果。

#### `__N` 后缀

`__N` 后缀指定第 `N` 个结果作为一个整体（可以是[可变参数](#supporting-variadic-ops)）。例如，我们可以将符号绑定到某个多结果操作，并在后续引用特定结果：

```tablegen
def ThreeResultOp : Op<"three_result_op"> {
    let arguments = (ins ...);

    let results = (outs
      AnyTensor:$output1,
      AnyTensor:$output2,
      AnyTensor:$output3
    );
}

def : Pattern<(ThreeResultOp:$results ...),
              [(... $results__0), ..., (... $results__2), ...]>;
```

在上面的模式中，我们将 `$results` 绑定到 `ThreeResultOp` 生成的所有结果，并在结果模式中引用其 `$output1` 和 `$output3`。

我们也可以同时绑定符号并引用其特定结果，这在生成多结果操作时通常很有用：

```tablegen
// TwoResultOp 的定义与 ThreeResultOp 类似，但只有两个结果。

def : Pattern<(TwoResultOp ...),
              [(ThreeResultOp:$results__2, ...),
               (replaceWithValue $results__0)]>;
```

在上面的例子中，我们创建了一个 `ThreeResultOp` 并将 `results` 绑定到其结果，分别使用其最后一个结果（`$output3`）和第一个结果（`$output1`）来替换 `TwoResultOp` 的两个结果。

#### 替换多结果操作

上面的例子也展示了如何替换匹配的多结果操作。

要替换一个 `N` 结果操作，结果模式必须生成至少 `N` 个已声明的值（有关定义，请参见[声明的值与实际值](#declared-vs-actual-value)）。如果生成的已声明值超过 `N` 个，只有最后 `N` 个已声明值将用于替换匹配的操作。注意，由于多结果操作的存在，一个结果模式**可能**生成多个已声明值。因此，替换一个 `N` 结果操作不一定需要 `N` 个结果模式。例如，要替换一个有三个结果的操作，可以使用：

```tablegen
// ThreeResultOp/TwoResultOp/OneResultOp 分别生成三/两/一个结果。

// 用各自操作生成的结果替换每个结果。
def : Pattern<(ThreeResultOp ...),
              [(OneResultOp ...), (OneResultOp ...), (OneResultOp ...)]>;

// 用同一操作生成的两个结果替换前两个结果。
def : Pattern<(ThreeResultOp ...),
              [(TwoResultOp ...), (OneResultOp ...)]>;

// 用同一操作生成的三个结果替换所有三个结果。
def : Pat<(ThreeResultOp ...), (ThreeResultOp ...)>;

def : Pattern<(ThreeResultOp ...),
              [(AuxiliaryOp ...), (ThreeResultOp ...)]>;
```

但使用单个操作同时作为辅助操作和替换操作是被禁止的，即以下是不允许的，因为第一个 `TwoResultOp` 生成两个结果，但只有第二个结果用于替换匹配操作的结果：

```tablegen
def : Pattern<(ThreeResultOp ...),
              [(TwoResultOp ...), (TwoResultOp ...)]>;
```

### 支持可变参数操作

#### 声明的值与实际值

在深入可变参数操作支持的细节之前，我们需要定义一些关于操作值的术语。

*   *值*：操作数或结果
*   *已声明的操作数/结果/值*：在操作 ODS 中静态声明的操作数/结果/值
*   *实际操作数/结果/值*：运行时操作实例的操作数/结果/值

需要这些术语，因为操作可以有多个结果，而某些结果也可以是可变参数的。例如：

```tablegen
def MultiVariadicOp : Op<"multi_variadic_op"> {
    let arguments = (ins
      AnyTensor:$input1,
      Variadic<AnyTensor>:$input2,
      AnyTensor:$input3
    );

    let results = (outs
      AnyTensor:$output1,
      Variadic<AnyTensor>:$output2,
      AnyTensor:$output3
    );
}
```

我们说上面的操作有 3 个已声明的操作数和 3 个已声明的结果。但在运行时，一个实例可以有 3 个值对应于 `$input2`，2 个值对应于 `$output2`；我们说它有 5 个实际操作数和 4 个实际结果。可变参数操作数/结果被视为可以对应多个实际值的已声明值。

[TODO]

#### 匹配可变参数操作数

使用 `variadic` DAG 节点来匹配具有固定数量实际子操作数的可变参数操作数。

例如，假设 `ConcatenateOp` 是一个具有可变参数操作数的操作：

```tablegen
def ConcatenateOp : TEST_Op<"concatenate"> {
  let arguments = (ins
    Variadic<AnyTensor>:$inputs,
    I32Attr:$axis
  );

  let results = (outs
    AnyTensor$output
  );
}
```

我们可以用以下方式匹配恰好有 2 个实际操作数的 `ConcatenateOp`：

```tablegen
def : Pat<(ConcatenateOp (variadic $input0, $input1), $axis),
          ...>;
```

可变参数子操作数可以是要匹配的子 DAG：

```tablegen
def : Pat<(ConcatenateOp (variadic (SomeOp $a), (AnotherOp $b, $c)), $axis),
          (OtherOp $a, $b, $c)>;
```

可变参数 DAG 可以绑定到一个符号，该符号引用完整的 `operand_range`：

```tablegen
def : Pat<(ConcatenateOp (variadic:$inputs $input0, $input1),
                         ConstantAttr<I32Attr, "0">),
          (VStackOp $inputs)>;
```

### 提供附加约束

在匹配时可以对操作参数施加约束。但有时我们还需要对匹配操作的结果施加约束，或者需要用同时覆盖参数和结果的约束来限制匹配。`Pattern`（和 `Pat`）的第三个参数就是为此目的服务的。

例如，我们可以写：

```tablegen
def HasNoUseOf: Constraint<CPred<"$_self.use_empty()">, "has no use">;

def HasSameElementType : Constraint<
    CPred<"cast<ShapedType>($0).getElementType() == "
          "cast<ShapedType>($1).getElementType()">,
    "has same element type">;

def : Pattern<(TwoResultOp:$results $input),
              [(...), (...)],
              [(F32Tensor:$results__0), (HasNoUseOf:$results__1),
               (HasSameElementShape $results__0, $input)]>;
```

您可以：

*   对之前绑定的符号使用普通的 `TypeConstraint`（`TwoResultOp` 的第一个结果必须是浮点张量）；
*   为之前绑定的符号定义新的 `Constraint`（`TwoResultOp` 的第二个结果必须没有使用）；
*   对多个绑定符号应用约束（`$input` 和 `TwoResultOp` 的第一个结果必须具有相同的元素类型）。

### 提供附加结果模式

有时我们需要在结果模式之后添加额外的代码，例如将源操作的属性复制到结果操作。这可以通过 `SupplementalPatterns` 参数指定。与辅助模式类似，它们不用于替换源模式中的结果。

例如，我们可以写：

```tablegen
def GetOwner: NativeCodeCall<"$0.getOwner()">;

def CopyAttrFoo: NativeCodeCallVoid<
  "$1->setAttr($_builder.getStringAttr(\"foo\"), $0->getInherentAttr(\"foo\"))">;

def CopyAttrBar: NativeCodeCallVoid<
  "$1->setAttr($_builder.getStringAttr(\"bar\"), $0->getInherentAttr(\"bar\"))">;


def : Pattern<
  (ThreeResultOp:$src ...),
  [(ZeroResultOp:$dest1 ...), (ThreeResultOp:$dest2 ...)],
  [(CopyAttrFoo (GetOwner $src), $dest1),
    (CopyAttrBar (GetOwner $src), (GetOwner $dest2))]>;
```

这将分别把源模式中 `ThreeResultOp` 的属性 `foo` 和 `bar` 复制到结果模式中的 `ZeroResultOp` 和 `ThreeResultOp`。模式按指定的顺序执行。

### 调整收益

`Pattern` 的收益是一个整数值，表示匹配该模式的收益。它决定了模式重写驱动程序中模式的优先级。收益较高的模式会在收益较低的模式之前应用。

在 DRR 中，规则的收益被设置为源模式中操作的数量。这基于以下启发式假设：

*   较大的匹配比较小的匹配更有益。
*   如果先应用较小的匹配，较大的匹配可能就无法应用了。

`Pattern`（和 `Pat`）的第四个参数允许手动调整模式的收益。只需提供 `(addBenefit N)` 以将 `N` 添加到收益值。

## 重写指令

### `location`

默认情况下，从 DRR 模式展开的 C++ 模式使用所有源操作的融合位置作为所有生成操作的位置。这并不总是最佳的位置映射关系。对于这类情况，DRR 提供了 `location` 指令来提供更精细的控制。

`location` 的语法如下：

```tablegen
(location $symbol0, $symbol1, ...)
```

其中所有 `$symbol` 都应该在模式中预先绑定，并且可以可选地指定一个字符串作为属性。将创建以下位置：

*   如果只指定了 1 个符号，则使用该符号的位置；
*   如果指定了多个符号，则创建融合位置；
*   如果未指定符号，则必须指定字符串，此时创建 NamedLoc；

`location` 必须作为操作创建的尾随参数使用。例如：

```tablegen
def : Pat<(LocSrc1Op:$src1 (LocSrc2Op:$src2 ...),
          (LocDst1Op (LocDst2Op ..., (location $src2)), (location "outer"))>;
```

在上面的模式中，生成的 `LocDst2Op` 将使用 `LocSrc2Op` 的匹配位置，而根 `LocDst1Op` 节点将使用命名位置 `outer`。

### `replaceWithValue`

`replaceWithValue` 指令用于通过将匹配操作的所有使用替换为捕获的值来消除该操作。其语法如下：

```tablegen
(replaceWithValue $symbol)
```

其中 `$symbol` 应该是模式中预先绑定的符号。

例如：

```tablegen
def : Pat<(Foo $input), (replaceWithValue $input)>;
```

上面的模式删除 `Foo` 并将 `Foo` 的所有使用替换为 `$input`。

### `returnType`

`returnType` 指令允许模式直接为缺乏通过操作 trait 或用户定义的带有返回类型推断的构建器的替换操作指定返回类型。

`returnType` 指令必须作为描述替换操作的节点的尾随参数使用。该指令有三种形式：

*   `(returnType $value)`：复制绑定到 `value` 的操作数或结果的类型。
*   `(returnType "$_builder.getI32Type()")`：嵌入 C++ 的字符串字面量。嵌入的代码片段预期返回 `Type` 或 `TypeRange`。
*   `(returnType (NativeCodeCall<"myFunc($0)"> $value))`：带有原生代码调用的 DAG 节点，可以传递任何绑定的变量参数。

可以使用上述任意组合来指定多个返回类型。示例：

```tablegen
def : Pat<(SourceOp $arg0, $arg1),
          (OpA $arg0, (TwoResultOp:$res__1 $arg1,
                         (returnType $arg1, "$_builder.getI64Type()")))>;
```

显式指定的返回类型优先于从操作 trait 或用户定义的构建器推断的返回类型。替换根操作结果的值的返回类型不能被覆盖。

### `either`

`either` 指令用于指定操作数可以以任一顺序匹配。

```tablegen
def : Pat<(TwoArgOp (either $firstArg, (AnOp $secondArg))),
          (...)>;
```

上面的模式将同时接受 `"test.TwoArgOp"(%I32Arg, %AnOpArg)` 和 `"test.TwoArgOp"(%AnOpArg, %I32Arg)`。

`either` 只支持操作数，请注意，具有 `Commutative` trait 的操作并不意味着它在模式匹配时与 `either` 具有相同的行为。

## 调试技巧

### 运行 `mlir-tblgen` 查看生成的内容

TableGen 语法有时会比较晦涩；阅读生成的内容是理解和调试问题的非常有效的方式。要构建 `mlir-tblgen`，请在构建目录中运行 `cmake --build . --target mlir-tblgen`，并在 `bin/` 子目录中找到 `mlir-tblgen` 二进制文件。所有支持的生成器可以通过 `mlir-tblgen --help` 找到。

要查看生成的代码，请通过 `-I` 提供包含路径，使用特定的生成器调用 `mlir-tblgen`。例如：

```sh
# 查看所有 C++ 模式重写类
mlir-tblgen --gen-rewriters -I /path/to/mlir/include /path/to/input/td/file
```

### 编译错误：没有匹配的成员函数调用 'build'

这是因为 DRR 无法调用具有结果类型推断能力的 `build()` 方法。有关更多详细信息，请参见[构建操作](#building-operations)。

[TableGen]: https://llvm.org/docs/TableGen/index.html
[OpBase]: https://github.com/llvm/llvm-project/blob/main/mlir/include/mlir/IR/OpBase.td
