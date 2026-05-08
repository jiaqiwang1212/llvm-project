<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUCommon Conversion

## Purpose
Provides shared utilities for all GPU-to-LLVM conversions (NVVM, ROCDL, SPIR-V). Contains the common GPU-to-LLVM conversion infrastructure: `gpu.launch_func` lowering, index intrinsic patterns, and op-to-function-call lowering helpers.

## Key Files
| File | Description |
|------|-------------|
| `GPUToLLVMConversion.cpp` | Lowers `gpu.launch_func` to runtime API calls (cuLaunchKernel, hipLaunchKernel, etc.) |
| `GPUOpsLowering.cpp` | Shared patterns for lowering GPU dialect ops (printf, alloc, etc.) |
| `GPUOpsLowering.h` | Internal header for GPUOpsLowering |
| `IndexIntrinsicsOpLowering.cpp` | Lowers `gpu.thread_id`, `gpu.block_id`, etc. to NVVM/ROCDL intrinsics |
| `IndexIntrinsicsOpLowering.h` | Header for index intrinsic patterns |
| `AttrToSPIRVConverter.cpp` | Converts GPU memory space attributes to SPIR-V storage class attributes |
| `OpToFuncCallLowering.h` | Template for lowering single-op to a library function call |

## For AI Agents

### Working In This Directory
- `GPUToLLVMConversion.cpp` is the key file — it generates GPU runtime API calls to launch kernels. The target API (CUDA, HIP, OpenCL) is selected via `GpuRtcLinkingLibrary` attribute.
- `IndexIntrinsicsOpLowering.h` provides a template pattern `IndexIntrinsicOpLowering<GpuOp, IntrinsicOp>` used by both NVVM and ROCDL conversions.
- `OpToFuncCallLowering.h` provides `OpToFuncCallLowering<SourceOp>` template for lowering a single op to a named function call — used by math-to-NVVM/ROCDL.
- `AttrToSPIRVConverter.cpp` maps `gpu.AddressSpaceAttr` (global/shared/private) to SPIR-V storage class (CrossWorkgroup/Workgroup/Function).

## Dependencies
- Source: `mlir/Dialect/GPU`
- Target: `mlir/Dialect/LLVMIR`, `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
