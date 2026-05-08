<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToNVVM Conversion

## Purpose
Lowers GPU dialect ops to NVVM dialect (NVIDIA LLVM IR intrinsics). Converts thread/block index ops, memory barriers, shuffles, subgroup ops, and warp matrix ops to their NVVM equivalents for CUDA compilation.

## Key Files
| File | Description |
|------|-------------|
| `LowerGpuOpsToNVVMOps.cpp` | Main lowering patterns: GPU index ops, barriers, shuffles, memory ops → NVVM |
| `WmmaOpsToNvvm.cpp` | Lowers `gpu.subgroup_mma_*` ops to NVVM WMMA intrinsics |
| `GPUToNVVM.td` | TableGen patterns for simple GPU→NVVM mappings |

## For AI Agents

### Working In This Directory
- `gpu.thread_id x` → `nvvm.read.ptx.sreg.tid.x` via `IndexIntrinsicsOpLowering` from GPUCommon.
- `gpu.barrier` → `nvvm.barrier0`.
- `gpu.shuffle xor` → `nvvm.shfl.sync.bfly.i32` etc.
- WMMA lowering in `WmmaOpsToNvvm.cpp` maps `gpu.subgroup_mma_load_matrix`, `gpu.subgroup_mma_compute`, `gpu.subgroup_mma_store_matrix` to `nvvm.wmma.*` intrinsics.
- `GPUToNVVM.td` handles trivial 1:1 op name mappings.

## Dependencies
- Source: `mlir/Dialect/GPU`
- Target: `mlir/Dialect/NVVM`
- Utility: `mlir/Conversion/GPUCommon`

<!-- MANUAL: -->
