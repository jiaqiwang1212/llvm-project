# 第 1 章：Toy 语言与抽象语法树

[TOC]

## 语言介绍

本教程将使用一种我们称之为"Toy"的玩具语言（起名真是难...）来进行说明。Toy 是一种基于张量（tensor）的语言，允许你定义函数、执行数学计算并打印结果。

为了保持简洁，代码生成（codegen）仅限于秩（rank）不超过 2 的张量，并且 Toy 中唯一的数据类型是 64 位浮点类型（即 C 语言中的 `double`）。因此，所有值都隐式为双精度浮点数，`Values` 是不可变的（即每个操作都返回一个新分配的值），内存释放由系统自动管理。好了，概述说得够多了，没有什么比通过一个具体示例来加深理解更好的了：

```toy
def main() {
  # Define a variable `a` with shape <2, 3>, initialized with the literal value.
  # The shape is inferred from the supplied literal.
  var a = [[1, 2, 3], [4, 5, 6]];

  # b is identical to a, the literal tensor is implicitly reshaped: defining new
  # variables is the way to reshape tensors (element count must match).
  var b<2, 3> = [1, 2, 3, 4, 5, 6];

  # transpose() and print() are the only builtin, the following will transpose
  # a and b and perform an element-wise multiplication before printing the result.
  print(transpose(a) * transpose(b));
}
```

类型检查通过类型推断（type inference）静态完成；只有在必要时，语言才要求显式声明张量的形状。函数是泛型的（generic）：其参数是无秩的（即我们知道它们是张量，但不知道其维度）。函数会针对调用点（call sites）每次发现的新签名进行特化（specialized）。让我们通过添加一个用户自定义函数来重新审视之前的示例：

```toy
# User defined generic function that operates on unknown shaped arguments.
def multiply_transpose(a, b) {
  return transpose(a) * transpose(b);
}

def main() {
  # Define a variable `a` with shape <2, 3>, initialized with the literal value.
  var a = [[1, 2, 3], [4, 5, 6]];
  var b<2, 3> = [1, 2, 3, 4, 5, 6];

  # This call will specialize `multiply_transpose` with <2, 3> for both
  # arguments and deduce a return type of <3, 2> in initialization of `c`.
  var c = multiply_transpose(a, b);

  # A second call to `multiply_transpose` with <2, 3> for both arguments will
  # reuse the previously specialized and inferred version and return <3, 2>.
  var d = multiply_transpose(b, a);

  # A new call with <3, 2> (instead of <2, 3>) for both dimensions will
  # trigger another specialization of `multiply_transpose`.
  var e = multiply_transpose(c, d);

  # Finally, calling into `multiply_transpose` with incompatible shapes
  # (<2, 3> and <3, 2>) will trigger a shape inference error.
  var f = multiply_transpose(a, c);
}
```

## 抽象语法树（AST）

上述代码所对应的抽象语法树（Abstract Syntax Tree，AST）相当直观，以下是其转储输出：

```
Module:
  Function 
    Proto 'multiply_transpose' @test/Examples/Toy/Ch1/ast.toy:4:1
    Params: [a, b]
    Block {
      Return
        BinOp: * @test/Examples/Toy/Ch1/ast.toy:5:25
          Call 'transpose' [ @test/Examples/Toy/Ch1/ast.toy:5:10
            var: a @test/Examples/Toy/Ch1/ast.toy:5:20
          ]
          Call 'transpose' [ @test/Examples/Toy/Ch1/ast.toy:5:25
            var: b @test/Examples/Toy/Ch1/ast.toy:5:35
          ]
    } // Block
  Function 
    Proto 'main' @test/Examples/Toy/Ch1/ast.toy:8:1
    Params: []
    Block {
      VarDecl a<> @test/Examples/Toy/Ch1/ast.toy:11:3
        Literal: <2, 3>[ <3>[ 1.000000e+00, 2.000000e+00, 3.000000e+00], <3>[ 4.000000e+00, 5.000000e+00, 6.000000e+00]] @test/Examples/Toy/Ch1/ast.toy:11:11
      VarDecl b<2, 3> @test/Examples/Toy/Ch1/ast.toy:15:3
        Literal: <6>[ 1.000000e+00, 2.000000e+00, 3.000000e+00, 4.000000e+00, 5.000000e+00, 6.000000e+00] @test/Examples/Toy/Ch1/ast.toy:15:17
      VarDecl c<> @test/Examples/Toy/Ch1/ast.toy:19:3
        Call 'multiply_transpose' [ @test/Examples/Toy/Ch1/ast.toy:19:11
          var: a @test/Examples/Toy/Ch1/ast.toy:19:30
          var: b @test/Examples/Toy/Ch1/ast.toy:19:33
        ]
      VarDecl d<> @test/Examples/Toy/Ch1/ast.toy:22:3
        Call 'multiply_transpose' [ @test/Examples/Toy/Ch1/ast.toy:22:11
          var: b @test/Examples/Toy/Ch1/ast.toy:22:30
          var: a @test/Examples/Toy/Ch1/ast.toy:22:33
        ]
      VarDecl e<> @test/Examples/Toy/Ch1/ast.toy:25:3
        Call 'multiply_transpose' [ @test/Examples/Toy/Ch1/ast.toy:25:11
          var: c @test/Examples/Toy/Ch1/ast.toy:25:30
          var: d @test/Examples/Toy/Ch1/ast.toy:25:33
        ]
      VarDecl f<> @test/Examples/Toy/Ch1/ast.toy:28:3
        Call 'multiply_transpose' [ @test/Examples/Toy/Ch1/ast.toy:28:11
          var: a @test/Examples/Toy/Ch1/ast.toy:28:30
          var: c @test/Examples/Toy/Ch1/ast.toy:28:33
        ]
    } // Block
```

你可以在 `examples/toy/Ch1/` 目录中重现这一结果并体验示例；尝试运行 `path/to/BUILD/bin/toyc-ch1 test/Examples/Toy/Ch1/ast.toy -emit=ast`。

词法分析器（lexer）的代码相当简洁，全部位于一个头文件中：`examples/toy/Ch1/include/toy/Lexer.h`。解析器（parser）位于 `examples/toy/Ch1/include/toy/Parser.h`，它是一个递归下降解析器（recursive descent parser）。如果你对此类词法分析器/解析器不熟悉，它们与 LLVM Kaleidoscope 中的等价实现非常相似，在以下教程的前两章中有详细介绍：[Kaleidoscope 教程](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html)。

[下一章](Ch-2.md)将演示如何将此 AST 转换为 MLIR。
