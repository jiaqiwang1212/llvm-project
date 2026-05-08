<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU Dialect

## Purpose
Implements the GPU dialect — the target-independent abstraction for GPU computation. Models GPU kernels, thread/block hierarchy, memory spaces, barriers, shuffles, and async operations. Serves as an intermediate between high-level parallel abstractions and GPU-specific backends (NVVM, ROCDL, SPIR-V).

## Key Files
| File | Description |
|------|-------------|
| `IR/GPUDialect.cpp` | Dialect registration, GPU module/function/launch ops, memory space attrs |
| `IR/InferIntRangeInterfaceImpls.cpp` | Integer range inference for thread/block ID ops |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds for GPU index ops |
| `Transforms/KernelOutlining.cpp` | Outlines GPU launch regions into separate GPU module functions |
| `Transforms/ModuleToBinary.cpp` | Serializes GPU modules to binary blobs (PTX, HSACO, SPIR-V) |
| `Transforms/NVVMAttachTarget.cpp` | Attaches NVVM compilation target to GPU modules |
| `Transforms/ROCDLAttachTarget.cpp` | Attaches ROCDL compilation target to GPU modules |
| `Transforms/AllReduceLowering.cpp` | Lowers `gpu.all_reduce` using shuffle trees |
| `Transforms/EliminateBarriers.cpp` | Removes redundant `gpu.barrier` ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | GPU ops, attributes, memory space, interface impls |
| `Transforms/` | Kernel outlining, binary serialization, target attachment, barrier elimination |
| `Pipelines/` | Predefined GPU lowering pipelines |
| `TransformOps/` | Transform dialect extension for GPU transformations |
| `Utils/` | GPU utility functions |

## For AI Agents

### Working In This Directory
- `gpu.launch` contains a region with thread/block index arguments; `KernelOutlining` lifts this to a `gpu.func` in a `gpu.module`.
- GPU modules are compiled to binary by `ModuleToBinary` using registered target backends — requires target to be attached first via `NVVMAttachTarget` / `ROCDLAttachTarget`.
- Thread hierarchy ops (`gpu.thread_id`, `gpu.block_id`, `gpu.grid_dim`, `gpu.block_dim`) carry a `gpu.Dimension` enum (x/y/z).
- Memory space attributes (`GPUAddressSpaceAttr`) distinguish global, shared (workgroup), and private (private) memory.
- `gpu.shuffle` has variants: `xor`, `up`, `down`, `idx` — each maps to a different hardware shuffle instruction.

### Common Patterns
- Kernel outlining: `GpuKernelOutliningPass` uses `OutlineKernelFuncPass` which walks `gpu.launch` ops and creates corresponding `gpu.func` ops.
- Target attachment: transforms look for `gpu.module` ops and add a `gpu.compilation_target` attribute.
- Subgroup/warp operations use `gpu.subgroup_size` for portability across NVVM/ROCDL/SPIR-V.

## Dependencies
- `mlir/Dialect/Func`, `mlir/Dialect/MemRef`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/NVVM`, `mlir/Dialect/ROCDL`

<!-- MANUAL: -->
