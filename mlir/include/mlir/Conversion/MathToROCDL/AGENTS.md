<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToROCDL

## Purpose
Declares the Math to ROCDL lowering pass. Lowers Math dialect ops to ROCDL (ROCm Device Library) intrinsics for optimized AMD GPU math execution.

## Key Files
| File | Description |
|------|-------------|
| `MathToROCDL.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToROCDL.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToROCDL/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (ROCDL ops)

<!-- MANUAL: -->
