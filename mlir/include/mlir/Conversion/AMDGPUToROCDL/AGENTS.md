<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPUToROCDL

## Purpose
Declares the AMDGPU to ROCDL lowering pass. Lowers AMD GPU dialect ops to ROCDL (ROCm Device Library) intrinsics targeting AMD GPU hardware.

## Key Files
| File | Description |
|------|-------------|
| `AMDGPUToROCDL.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `AMDGPUToROCDL.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/AMDGPUToROCDL/`

## Dependencies
- Source dialect: `include/mlir/Dialect/AMDGPU/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (ROCDL ops)

<!-- MANUAL: -->
