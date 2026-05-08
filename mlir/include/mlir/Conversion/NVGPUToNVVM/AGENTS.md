<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPUToNVVM

## Purpose
Declares the NVGPU to NVVM lowering pass. Lowers NVGPU dialect ops (warp-cooperative matrix multiply, async copies, tensor memory access) to NVVM intrinsics for advanced CUDA GPU features.

## Key Files
| File | Description |
|------|-------------|
| `NVGPUToNVVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `NVGPUToNVVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/NVGPUToNVVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/NVGPU/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (NVVM ops)

<!-- MANUAL: -->
