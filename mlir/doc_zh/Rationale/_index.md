# 设计原理

本节收录了一系列文档，介绍 MLIR 若干设计决策背后的动机与原理。

[MLIR：在机器学习框架图算法中的渐进式应用](MLIRForGraphAlgorithms.md)
:   讨论如何以渐进的方式采用 MLIR，每一步都能在推进过程中带来切实的收益。驳斥了"必须完全采用 MLIR 才能从中受益"的观点。

[MLIR 设计原理](Rationale.md)
:   介绍 MLIR 的设计动机，并梳理针对 MLIR 各核心特性所做的设计讨论与决策。

[通用 DAG 重写基础设施设计原理](RationaleGenericDAGRewriter.md)
:   详述 MLIR 通用 DAG 到 DAG 重写基础设施的设计依据。

[Linalg 方言设计原理：对编译器友好的自定义操作](RationaleLinalgDialect.md)
:   描述促成 Linalg 现有实现的关键设计原则以及过程中积累的经验教训。

[MLIR：简化多面体形式的理由](RationaleSimplifiedPolyhedralForm.md)
:   一份早期设计提案，探讨了在 MLIR 中使用简化形式的多面体编译器技术相对于传统多面体调度列表形式的权衡取舍。

[MLIR 核心 IR 类型中 'const' 的使用](UsageOfConst.md)
:   解释了在 MLIR 核心 IR 类型中完全回避使用 `const` 的设计原理。
