# Toy 教程

本教程介绍了如何在 MLIR 之上实现一个基础的玩具语言。本教程的目标是介绍 MLIR 的核心概念；尤其是[方言（dialects）](../../LangRef.md/#dialects)如何轻松支持语言特定的构造和转换，同时仍然提供便捷的途径降级到 LLVM 或其他代码生成基础设施。本教程以
[LLVM Kaleidoscope 教程](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)为蓝本。

另一个很好的入门资源是 2020 年 LLVM 开发者大会的在线[录像](https://www.youtube.com/watch?v=Y4SvqTtOIDk)（[幻灯片](https://llvm.org/devmtg/2020-09/slides/MLIR_Tutorial.pdf)）。

本教程假设您已经克隆并构建了 MLIR；如果尚未完成，请参阅
[MLIR 入门](../../../getting_started/)。

本教程分为以下几章：

-   [第 1 章](Ch-1.md)：介绍 Toy 语言及其 AST 的定义。
-   [第 2 章](Ch-2.md)：遍历 AST 以在 MLIR 中生成方言，介绍 MLIR 基础概念。本章展示如何开始为 MLIR 中的自定义操作附加语义。
-   [第 3 章](Ch-3.md)：使用模式重写系统进行高层次的语言特定优化。
-   [第 4 章](Ch-4.md)：通过接口编写与方言无关的通用转换。本章将展示如何将方言特定信息插入到形状推断和内联等通用转换中。
-   [第 5 章](Ch-5.md)：部分降级到低层次方言。我们将把部分高层次语言特定语义转换为面向优化的通用仿射方言。
-   [第 6 章](Ch-6.md)：降级到 LLVM 并生成代码。本章将以 LLVM IR 为目标进行代码生成，并详细介绍降级框架。
-   [第 7 章](Ch-7.md)：扩展 Toy：添加对复合类型的支持。我们将演示如何向 MLIR 添加自定义类型，以及它如何融入现有的流水线。

[第一章](Ch-1.md)将介绍 Toy 语言和 AST。
