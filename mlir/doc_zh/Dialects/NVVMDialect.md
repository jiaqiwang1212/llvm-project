# 'nvvm' 方言

NVVM 方言是 MLIR 基于 LLVM IR 的 NVIDIA 特定后端方言。它对 NVVM 内联函数和公共 ISA 功能进行建模，并向 MLIR/LLVM 类型系统和地址空间（例如，全局、共享和集群内存）引入 NVIDIA 扩展，从而能够将 GPU 内核忠实地降低到 NVPTX 工具链。虽然一个 NVVM 操作通常映射到单个 LLVM IR 内联函数，但 NVVM 方言使用类型多态和其他属性，使单个 NVVM 操作可以映射到不同的 LLVM 内联函数。

[TOC]

## 范围与能力

该方言涵盖核心 GPU 特性，例如线程/块内置函数、屏障和原子操作、束级集合（例如，shuffle/vote）、矩阵/张量核心操作（例如，`mma.sync`、`wgmma`）、张量内存加速器（TMA）操作、带内存屏障的异步拷贝（`cp.async`、bulk/tensor 变体）、缓存和预取控制，以及 NVVM 特定属性和枚举（例如，浮点舍入模式、内存范围和 MMA 类型/布局）。

## 在降低流水线中的位置

NVVM 位于与目标无关的方言（如 `gpu` 和 NVIDIA 的 `nvgpu`）之下。典型的流水线使用 `-convert-gpu-to-nvvm` 和 `-convert-nvgpu-to-nvvm` 将 `gpu`/`nvgpu` 操作转换为 NVVM，然后通过 NVPTX 后端翻译为 LLVM 以进行最终代码生成。

## 目标配置与序列化

NVVM 提供了一个 `#nvvm.target` 属性来描述 GPU 目标（SM、特性和标志）。结合 `gpu` 序列化（例如，`gpu-module-to-binary`），这能够从嵌套的 GPU 模块生成特定架构的 GPU 二进制文件（例如 CUBIN）。

## 内联 PTX

当内联函数不可用或必须直接表达性能关键序列时，NVVM 提供了一个 `nvvm.inline_ptx` 操作，作为最后的应急手段，以内联方式嵌入 PTX，并具有显式操作数和结果。

## 内存空间

NVVM 方言引入了以下内存空间，每个都有不同的作用域和生命周期：

| 内存空间           | 地址空间 | 作用域              |
|-------------------|----------|---------------------|
| `generic`         | 0        | 所有线程             |
| `global`          | 1        | 所有线程（设备）      |
| `shared`          | 3        | 线程块（CTA）        |
| `constant`        | 4        | 所有线程             |
| `local`           | 5        | 单个线程             |
| `tensor`          | 6        | 线程块（CTA）        |
| `shared_cluster`  | 7        | 线程块集群           |

### 内存空间详情

- **generic**：可以指向任何内存空间；需要在运行时解析实际地址空间。当编译时指针来源未知时使用。性能取决于底层内存空间。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerGeneric` 表示。
- **global**：所有块中的所有线程都可访问；在内核启动之间持久存在。延迟最高但容量最大（设备内存）。最适合大数据和内核间通信。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerGlobal` 表示。
- **shared**：在线程块（CTA）内共享；非常快速的片上内存，用于同一块中线程之间的协作。容量有限。非常适合块级协作、缓存和减少全局内存流量。在 NVVMOps 中，此内存通常称为 `shared_cta`，在 PTX ISA 中称为 `shared::cta`。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerShared` 类型表示。
- **constant**：每个 SM 缓存的只读内存。大小通常限制为 64KB。最适合所有线程访问的只读数据和统一值。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerConst` 类型表示。
- **local**：每个线程私有。用于不适合寄存器的每线程私有数据和自动变量。在 NVVM 操作中，指向此内存的指针由 `LLVM_PointerLocal` 类型表示。
- **tensor**：用于张量核心操作的特殊内存空间。由 SM 100+ 上的 `tcgen05` 指令用于张量输入/输出操作。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerTensor` 类型表示。
- **shared_cluster**：跨集群内线程块的分布式共享内存（SM 90+）。支持超越单块范围的协作，对集群线程具有快速访问。在 NVVMOps 中，此内存通常称为 `shared_cluster`，在 PTX ISA 中称为 `shared::cluster`。在 NVVM 操作中，指向此内存空间的指针由 `LLVM_PointerSharedCluster` 类型表示。

## MBarrier 对象

``mbarrier`` 是在共享内存中创建的屏障，支持在 CTA 内同步任意线程子集。*mbarrier 对象*是共享内存中 `.b64` 类型、8 字节对齐的不透明对象。与每个 CTA 只能访问有限数量屏障的 ``nvvm.barrier`` 操作不同，*mbarrier 对象*是用户定义的，仅受可用共享内存总量的限制。*mbarrier 对象*上支持的操作列表通过 ``nvvm.mbarrier.*`` 系列 NVVM 操作暴露。

## 非目标

NVVM 不是便利性或"包装器"操作的地方。它不打算引入扩展为多个不相关 NVVM 内联函数或根本不降低到任何内联函数的高层操作。此类抽象属于更高层次的方言（例如，`nvgpu`、`gpu` 或项目特定方言）。设计意图是具有近乎机械降低到 NVVM/LLVM IR 的薄的、可预测的、低层接口。


## 操作

NVIDIA 指令集中的所有操作在 MLIR 中都有自定义形式。操作的助记符是在 LLVM IR 中使用的名称加上前缀 "`nvvm.`"。

[include "Dialects/NVVMOps.md"]
