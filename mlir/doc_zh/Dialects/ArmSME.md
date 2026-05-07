# 'ArmSME' 方言（Dialect）

用于面向 Arm SME 的基础方言（dialect）。

本方言（dialect）定义了用于面向 Arm 可扩展矩阵扩展（Scalable Matrix Extension，SME）的自定义操作（operation）和 LLVM IR 内置操作。通过可用的转换通道（conversion pass）和 ArmSME 通道（pass），例如可以将
[linalg.matmul](https://mlir.llvm.org/docs/Dialects/Linalg/#linalgmatmul-linalgmatmulop)
操作降低（lower）为 Arm SME
[FMOPA](https://developer.arm.com/documentation/ddi0602/2023-03/SME-Instructions/FMOPA--widening---Half-precision-floating-point-sum-of-outer-products-and-accumulate-)
（浮点外积）操作。相关参考，请参阅树内（in-tree）端到端集成测试：

* [Linalg/CPU/ArmSME/matmul.mlir](https://github.com/llvm/llvm-project/blob/main/mlir/test/Integration/Dialect/Linalg/CPU/ArmSME/matmul.mlir)
* [Vector/CPU/ArmSME/outerproduct-f64.mlir](https://github.com/llvm/llvm-project/blob/main/mlir/test/Integration/Dialect/Vector/CPU/ArmSME/outerproduct-f64.mlir)

要运行 ArmSME 集成测试，请在配置 LLVM 和 MLIR 时在 CMake 调用中加入以下标志：
```bash
  -DMLIR_INCLUDE_INTEGRATION_TESTS=On
  -DMLIR_RUN_ARM_SME_TESTS=On
  -DARM_EMULATOR_EXECUTABLE=<path-to-emulator>
```

这些测试由
[clang-aarch64-sve-vla](https://lab.llvm.org/buildbot/#/builders/197) LLVM
BuildBot 工作节点在"提交后（post-commit）"运行。

**参考资料：**

* [The Scalable Matrix Extension (SME), for Armv9-A](https://developer.arm.com/documentation/ddi0616)
* [A64 -- SME Instructions (alphabetic order)](https://developer.arm.com/documentation/ddi0602/2023-03/SME-Instructions)

[TOC]

## 操作（Operations）

[include "Dialects/ArmSMEOps.md"]

## LLVM IR 内置操作（Operations for LLVM IR Intrinsics）

[include "Dialects/ArmSMEIntrinsicOps.md"]
