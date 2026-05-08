<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU Dialect

## Purpose
Implements the NVGPU dialect — higher-level NVIDIA GPU abstractions above NVVM intrinsics. Models Tensor Core MMA operations (warp-level matrix multiply), async copy ops (`nvgpu.device_async_copy`), TMA (Tensor Memory Accelerator) operations for Hopper, and shared memory optimizations.

## Key Files
| File | Description |
|------|-------------|
| `IR/NVGPUDialect.cpp` | Dialect registration and type definitions (warp matrix types, TMA descriptor) |
| `Transforms/CreateAsyncGroups.cpp` | Groups async copy ops into commit/wait groups for pipeline scheduling |
| `Transforms/MmaSyncTF32Transform.cpp` | Transforms matmul ops to use TF32 precision MMA instructions |
| `Transforms/OptimizeSharedMemory.cpp` | Inserts shared memory padding to avoid bank conflicts |
| `Transforms/Utils.cpp` | Shared utilities for MMA layout queries |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Warp matrix types, TMA descriptor type, dialect ops |
| `Transforms/` | Async grouping, TF32 MMA, shared memory optimization |
| `TransformOps/` | Transform dialect extension for NVGPU |
| `Utils/` | MMA layout utilities |

## For AI Agents

### Working In This Directory
- NVGPU sits between GPU dialect and NVVM dialect: `nvgpu.mma.sync` → `nvvm.mma.sync` intrinsic.
- Warp matrix types (`nvgpu.warp_matrix_type`) encode MMA accumulator/input layouts (m16n8k16 etc.).
- `nvgpu.device_async_copy` models `cp.async` instructions for async global→shared memory copies.
- TMA ops (`nvgpu.tma_prefetch_descriptor`, etc.) target Hopper (sm_90) and require TMA descriptor setup.
- `CreateAsyncGroups.cpp` analyzes copy/compute overlap to insert `nvgpu.device_async_create_group` and `wait` ops.

### Common Patterns
- MMA ops carry matrix shape attributes matching the hardware tile (m,n,k) and data types.
- Bank conflict avoidance in `OptimizeSharedMemory`: adds padding bytes based on element size and tile width.

## Dependencies
- `mlir/Dialect/GPU`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/NVVM`, `mlir/Dialect/Vector`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
