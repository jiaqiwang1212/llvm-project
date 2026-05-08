<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToROCDL Conversion

## Purpose
Lowers GPU dialect ops to ROCDL dialect (AMD GPU LLVM IR intrinsics). Converts thread/block index ops, barriers, shuffles, and subgroup ops to AMD HIP/ROCm equivalents.

## Key Files
| File | Description |
|------|-------------|
| `LowerGpuOpsToROCDLOps.cpp` | Main lowering patterns: GPU index ops, barriers, shuffles → ROCDL |
| `GPUToROCDL.td` | TableGen patterns for simple GPU→ROCDL mappings |

## For AI Agents

### Working In This Directory
- `gpu.thread_id x` → `rocdl.workitem.id.x`, `gpu.block_id x` → `rocdl.workgroup.id.x`.
- `gpu.barrier` → `rocdl.barrier`.
- `gpu.shuffle` maps to `rocdl.ds_swizzle`/`rocdl.permute` depending on the shuffle mode.
- `gpu.subgroup_size` → `rocdl.wavefrontsize`.
- Uses `IndexIntrinsicsOpLowering` template from GPUCommon.

## Dependencies
- Source: `mlir/Dialect/GPU`
- Target: `mlir/Dialect/ROCDL`
- Utility: `mlir/Conversion/GPUCommon`

<!-- MANUAL: -->
