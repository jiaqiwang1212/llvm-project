# Transform 方言教程

MLIR 通过 transform 方言支持对编译器变换进行声明式规格说明。它允许使用编译器 IR 本身来请求编译器变换，这些变换既可以嵌入到正在被变换的原始 IR 中（类似于 pragma），也可以单独提供（类似于调度语言）。本教程介绍 MLIR transform 方言及相关基础设施的概念，并辅以三个使用场景的实践演示：

- 组合（上游）MLIR 中已有的 Transform 方言操作，对一个 MLIR 线性代数操作执行一系列优化变换，以生成高效代码。
- 定义新的 Transform 方言操作，并对已有的变换代码进行适配，使其能与 Transform 方言基础设施协同工作。
- 在一个包含自定义方言、变换和 pass 的下游树外项目中搭建并使用 Transform 方言基础设施。

完成本教程后，读者将能够在自己的工作中应用 Transform 方言，并在必要时对其进行扩展。对 MLIR 有基本了解是前提条件，请参阅 [Toy 教程](../Toy) 了解 MLIR 入门知识。

本教程分为以下几章。

-  [第 #0 章](Ch0.md)："结构化" Linalg 操作入门
-  [第 #1 章](Ch1.md): 组合已有变换
-  [第 #2 章](Ch2.md): 添加一个简单的新变换操作
-  [第 #3 章](Ch3.md): 不止于简单的 Transform 操作
-  [第 #4 章](Ch4.md): 用 Transform 操作匹配载荷
-  [第 H 章](ChH.md): 复现 Halide 调度

本教程对应的代码位于 `mlir/Examples/transform` 下，相应的测试位于 `mlir/test/Examples/transform`。
