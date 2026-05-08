<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToAMDGPU

## Purpose
Declares the Arith to AMDGPU lowering pass. Lowers Arith dialect arithmetic operations to AMD GPU-specific ops and intrinsics for improved performance on AMD hardware.

## Key Files
| File | Description |
|------|-------------|
| `ArithToAMDGPU.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArithToAMDGPU.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArithToAMDGPU/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Target dialect: `include/mlir/Dialect/AMDGPU/`

<!-- MANUAL: -->
