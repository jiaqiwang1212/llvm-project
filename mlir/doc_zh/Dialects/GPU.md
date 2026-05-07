# 'gpu' 方言（Dialect）

注意：与其他方言相比，该方言在近期更有可能发生变化，请谨慎使用。

本方言为按照类似 CUDA 或 OpenCL 编程模型启动 GPU 内核（Kernel）提供中层抽象。它提供了内核调用的抽象（未来可能还会提供设备管理方面的抽象），这些抽象在较低层级（例如作为 GPU 的 LLVM IR 内置函数）中并不存在。其目标是屏蔽启动 GPU 内核时与设备及驱动相关的具体操作，为从 MLIR 执行 GPU 计算提供一条简洁的路径。例如，使用 MLIR 的领域特定语言（DSL）可以以本方言为目标。本方言使用 `gpu` 作为其规范前缀。

本方言还对 GPU 代码中常见的原语进行了抽象，例如 `gpu.thread_id`（一个返回线程在线程块/工作组内沿给定维度的 ID 的操作）。虽然下文记录的编译流水线期望此类代码位于 `gpu.module` 和 `gpu.func` 内部，但这些内置封装操作也可在此上下文之外使用。

封装内置函数的操作不应要求其父级类型为 `gpu.func`。然而，涉及编译和启动 GPU 函数的操作，如 `gpu.launch_func` 或 `gpu.binary`，可以假定方言的完整层次结构正在被使用。

[TOC]

## GPU 地址空间（GPU address spaces）

GPU 方言暴露了 `gpu.address_space` 属性（Attribute），目前有四个取值：`global`、`workgroup`、`private` 和 `constant`。

这些地址空间代表了 GPU 编译中常见的缓冲区类型。`global` 内存是驻留在 GPU 全局内存中的内存。`workgroup` 内存是一种有限的、每工作组独有的资源：同一工作组/线程块中的所有线程访问 `workgroup` 内存中相同的值。`private` 内存用于表示类似 `alloca` 的缓冲区，这些缓冲区对单个线程/工作项是私有的。`constant` 内存是驻留在全局地址空间中的只读内存，保证在内核执行期间不会改变，允许后端进行特定优化（例如，在 AMD GPU 上进行标量读取）。

这些地址空间可用作 `memref` 值上的 `memorySpace` 属性。`gpu.module`/`gpu.func` 编译流水线会将此类内存空间用法降级（Lower）到目标平台上正确的地址空间。内存归属（Memory attribution）应使用 memref 上正确的内存空间来创建。

## 内存归属（Memory attribution）

内存缓冲区在函数级别定义，可以在 `gpu.launch` 或 `gpu.func` 操作中定义。这种编码方式明确了内存的归属，并使内存的生命周期清晰可见。内存仅在内核启动/函数当前调用期间可访问。后者比实际的 GPU 实现更为严格，但在函数级别使用静态内存只是为了方便。也可以始终将指向工作组内存的指针传递给其他函数，前提是这些函数期望正确的内存空间。

缓冲区在整个 GPU 函数体的执行过程中被视为存活状态。若不存在内存归属语法，则表示该函数不需要特殊缓冲区。设计理由：尽管底层模型在模块级别声明内存缓冲区，但我们选择在函数级别进行声明，以便为这些缓冲区的生命周期提供一定的结构；这避免了将缓冲区用于不同内核之间或同一内核的不同启动之间进行通信的诱因——这类通信应通过函数参数来完成；我们也选择不使用 `alloca` 风格的方法，因为那需要更复杂的生命周期分析，而 MLIR 的设计原则是倡导在中间表示（IR）中体现结构并表示分析结果。

## GPU 编译（GPU Compilation）
### 编译概述（Compilation overview）
GPU 方言中的编译过程分为两个主要阶段：GPU 模块序列化（Serialization）和卸载操作翻译（Offloading operations translation）。这两个阶段共同作用，可以生成 GPU 二进制文件以及执行它们所需的代码。

编译工作流示例如下：

```
mlir-opt example.mlir                   \
  --pass-pipeline="builtin.module(      \
    gpu-kernel-outlining,               \ # Outline gpu.launch body to a kernel.
    nvvm-attach-target{chip=sm_90 O=3}, \ # Attach an NVVM target to a gpu.module op.
    gpu.module(convert-gpu-to-nvvm),    \ # Convert GPU to NVVM.
    gpu-to-llvm,                        \ # Convert GPU to LLVM.
    gpu-module-to-binary                \ # Serialize GPU modules to binaries.
  )" -o example-nvvm.mlir
mlir-translate example-nvvm.mlir        \
  --mlir-to-llvmir                      \ # Obtain the translated LLVM IR.
  -o example.ll
```

此编译过程要求所有 GPU 代码位于 `gpu.module` 中，并要求所有内核为 `gpu.func` 操作。非内核函数（如设备库调用）可以使用 `func.func` 或其他非 GPU 方言操作来定义。这允许下游系统使用这些封装器，而无需强制使用 GPU 方言的函数操作——那些操作可能不包含这些系统希望作为内置值保留在函数上的信息。此外，这也允许在 `gpu.module` 中使用 `func.func` 定义设备端库函数。

### 默认 NVVM 编译流水线（Default NVVM Compilation Pipeline）：gpu-lower-to-nvvm-pipeline

`gpu-lower-to-nvvm-pipeline` 编译流水线是 MLIR 中 NVVM 目标编译的默认方式。该流水线通过将主要方言（arith、memref、scf、vector、gpu 和 nvgpu）降级到 NVVM 目标来运作。它首先将 GPU 代码区域降级到指定的 NVVM 编译目标，然后再处理宿主机（Host）代码。

该流水线特别要求显式并行的中间表示（IR），不进行 GPU 并行化。若要启用并行性，必须在使用此流水线之前应用必要的变换。

它旨在为 NVVM 目标提供通用解决方案，生成与 `mlir-runner` 或执行引擎兼容的 NVVM 和 LLVM 方言代码。

#### 示例：

以下代码片段展示了在 GPU 代码执行中使用主要方言（包括 arith）的用法：

```
func.func @main() {
    %c2 = arith.constant 2 : index
    %c1 = arith.constant 1 : index
    gpu.launch
        blocks(%0, %1, %2) in (%3 = %c1, %4 = %c1, %5 = %c1)
        threads(%6, %7, %8) in (%9 = %c2, %10 = %c1, %11 = %c1) {
        gpu.printf "Hello from %d\n", %6 : index
        gpu.terminator
    }
    return
}
```

`gpu-lower-to-nvvm` 流水线将此输入代码编译为如下所示的 NVVM 格式。它提供了诸如指定 SM 架构版本、PTX 版本和优化级别等自定义选项。编译完成后，生成的中间表示即可使用 `mlir-runner` 执行。也可以将其翻译为 LLVM，从而扩展其在系统中的应用范围。

```
mlir-opt example.mlir -gpu-lower-to-nvvm-pipeline="cubin-chip=sm_90a cubin-features=+ptx80 opt-level=3"
```

### 模块序列化（Module serialization）
实现了 GPU 目标属性接口（GPU Target Attribute Interface）的属性负责处理序列化过程，称为目标属性（Target attributes）。这些属性可以附加到 GPU 模块上，指示将模块编译为二进制字符串的序列化方案。

`gpu-module-to-binary` 通路（Pass）搜索所有嵌套的 GPU 模块，并使用附加到模块上的目标属性对其进行序列化，为每个目标生成一个包含对象的二进制文件。

示例：
```
// Input:
gpu.module @kernels [#nvvm.target<chip = "sm_90">, #nvvm.target<chip = "sm_60">] {
  ...
}
// mlir-opt --gpu-module-to-binary:
gpu.binary @kernels [
  #gpu.object<#nvvm.target<chip = "sm_90">, "sm_90 cubin">,
  #gpu.object<#nvvm.target<chip = "sm_60">, "sm_60 cubin">
]
```

### 卸载 LLVM 翻译（Offloading LLVM translation）
实现了 GPU 卸载 LLVM 翻译属性接口（GPU Offloading LLVM Translation Attribute Interface）的属性负责将 GPU 二进制文件和内核启动翻译为 LLVM 指令，称为卸载属性（Offloading attributes）。这些属性附加到 GPU 二进制操作上。

在 LLVM 翻译过程中，GPU 二进制文件使用卸载属性提供的方案进行翻译，将 GPU 二进制文件转换为 LLVM 指令。与此同时，内核启动的翻译通过搜索相应的二进制文件，并调用二进制文件中卸载属性所提供的将内核启动翻译为 LLVM 指令的过程来完成。

示例：
```
// Input:
// Binary with multiple objects but selecting the second one for embedding.
gpu.binary @binary <#gpu.select_object<#rocdl.target<chip = "gfx90a">>> [
    #gpu.object<#nvvm.target, "NVPTX">,
    #gpu.object<#rocdl.target<chip = "gfx90a">, "AMDGPU">
  ]
llvm.func @foo() {
  ...
  // Launching a kernel inside the binary.
  gpu.launch_func @binary::@func blocks in (%0, %0, %0)
                                 threads in (%0, %0, %0) : i64
                                 dynamic_shared_memory_size %2
                                 args(%1 : i32, %1 : i32)
  ...
}
// mlir-translate --mlir-to-llvmir:
@binary_bin_cst = internal constant [6 x i8] c"AMDGPU", align 8
@binary_func_kernel_name = private unnamed_addr constant [7 x i8] c"func\00", align 1
@binary_module = internal global ptr null
@llvm.global_ctors = appending global [1 x {i32, ptr, ptr}] [{i32 123, ptr @binary_load, ptr null}]
@llvm.global_dtors = appending global [1 x {i32, ptr, ptr}] [{i32 123, ptr @binary_unload, ptr null}]
define internal void @binary_load() section ".text.startup" {
entry:
  %0 = call ptr @mgpuModuleLoad(ptr @binary_bin_cst)
  store ptr %0, ptr @binary_module
  ...
}
define internal void @binary_unload() section ".text.startup" {
entry:
  %0 = load ptr, ptr @binary_module, align 8
  call void @mgpuModuleUnload(ptr %0)
  ...
}
...
define void @foo() {
  ...
  %module = load ptr, ptr @binary_module, align 8
  %kernel = call ptr @mgpuModuleGetFunction(ptr %module, ptr @binary_func_kernel_name)
  call void @mgpuLaunchKernel(ptr %kernel, ...) ; Launch the kernel
  ...
  call void @mgpuModuleUnload(ptr %module)
  ...
}
...
```

### 二进制操作（The binary operation）
从语义角度来看，GPU 二进制文件允许实现许多概念，从简单的对象文件到胖二进制文件（Fat Binaries）。默认情况下，二进制操作使用 `#gpu.select_object` 卸载属性；该属性将二进制文件中的单个对象作为全局字符串嵌入，详见该属性的文档。

## 操作（Operations）

[include "Dialects/GPUOps.md"]
