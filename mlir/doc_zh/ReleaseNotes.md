# MLIR 发行说明

本文档在 LLVM 发行版的背景下尝试提供 MLIR 重要变更的相关背景信息，并在力所能及的范围内持续更新。

目前 MLIR 社区没有专门对 LLVM 发行分支进行资质认证，发行版是发布时 MLIR 开发的快照。

[TOC]

## LLVM 21

### GPU/NVVM 变更

- 默认 NVVM 目标架构已从 `sm_50` 更改为 `sm_75`。
  `sm_75` 是与最新主要 CUDA 工具包版本（11/12/13）最广泛兼容的最旧 GPU 变体。这会影响 `NVVMTargetAttr`、`GpuNVVMAttachTarget` pass 以及 `gpu-lower-to-nvvm-pipeline`。

## LLVM 20

所有除 `mlir-cpu-runner` 之外的 MLIR 运行器均已被移除，因为其功能已合并到该运行器中，并已将其重命名为 `mlir-runner`。

## LLVM 18

### 属性之外的 Properties

参见下面的 LLVM 17 说明。Dialect 选项 `let usePropertiesForAttributes = 1;` 现已成为默认值。您可以将其设置为 0 以恢复到之前的行为。此选项将在 LLVM 19 中移除。

## LLVM 17

另请参阅[废弃和重构](https://mlir.llvm.org/deprecation/)文档。

### 字节码（Bytecode）

MLIR 现在支持[字节码序列化](https://mlir.llvm.org/docs/BytecodeFormat/)，
具有版本兼容性，允许双向兼容方案以及延迟加载功能。

### 属性之外的 Properties

这是一种在操作中实现存储而无需使用属性的新机制。您可以通过在 dialect 定义中使用 `let usePropertiesForAttributes = 1;` 来选择为 ODS 固有属性使用 Properties（该标志将在下一个版本中成为默认值）。详情请参阅开放会议演示的
[幻灯片](https://mlir.llvm.org/OpenMeetings/2023-02-09-Properties.pdf) 和
[录像](https://youtu.be/7ofnlCFzlqg)。

### Action：跟踪和调试基于 MLIR 的编译器

[Action](https://mlir.llvm.org/docs/ActionTracing/) 是一种新机制，用于以某种可被框架拦截的方式封装任何粒度的任何转换，以便进行调试或跟踪，包括以编程方式跳过某个转换（类似于 LLVM 中的"编译器燃料"或"调试计数器"）。因此，"执行一个 pass"是一个 Action，"尝试应用一个规范化模式"也是，"对这个循环进行平铺"也是。

详情请参阅开放会议演示的
[幻灯片](https://mlir.llvm.org/OpenMeetings/2023-02-23-Actions.pdf) 和
[录像](https://youtu.be/ayQSyekVa3c)。

### 变换 Dialect（Transform Dialect）

请参阅此 [EuroLLVM 演讲](https://www.youtube.com/watch?v=P4gUj3QtH_Y&t=1s) 和
[在线教程](https://mlir.llvm.org/docs/Tutorials/transform/)。

### 其他

- 现在支持
  "[distinct 属性](https://mlir.llvm.org/docs/Dialects/Builtin/#distinctattribute)"。
- "Resources"（一种在 MLIR 上下文之外存储数据的方式）和"configuration"
  现在可以与 IR 一起序列化。
