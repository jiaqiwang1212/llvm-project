<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmNeon2dToIntr

## Purpose
Declares the ArmNeon 2D to intrinsics lowering pass. Lowers Arm NEON 2D-vector ops to LLVM ArmNeon intrinsic calls for native SIMD execution.

## Key Files
| File | Description |
|------|-------------|
| `ArmNeon2dToIntr.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArmNeon2dToIntr.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArmNeon2dToIntr/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ArmNeon/`
- Target: LLVM ArmNeon intrinsics via `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
