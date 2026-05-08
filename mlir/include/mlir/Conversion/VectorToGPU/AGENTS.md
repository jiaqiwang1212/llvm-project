<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToGPU

## Purpose
Declares the Vector to GPU lowering pass. Lowers Vector dialect ops to GPU dialect warp-level ops and NVGPU/WMMA matrix ops for GPU tensor core acceleration.

## Key Files
| File | Description |
|------|-------------|
| `VectorToGPU.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `VectorToGPU.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/VectorToGPU/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/GPU/`, `include/mlir/Dialect/NVGPU/`

<!-- MANUAL: -->
