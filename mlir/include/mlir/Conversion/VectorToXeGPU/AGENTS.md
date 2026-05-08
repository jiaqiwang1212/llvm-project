<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToXeGPU

## Purpose
Declares the Vector to XeGPU lowering pass. Lowers Vector dialect ops to XeGPU (Intel Xe GPU) dialect ops for Intel GPU matrix and vector acceleration.

## Key Files
| File | Description |
|------|-------------|
| `VectorToXeGPU.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `VectorToXeGPU.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/VectorToXeGPU/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/XeGPU/`

<!-- MANUAL: -->
