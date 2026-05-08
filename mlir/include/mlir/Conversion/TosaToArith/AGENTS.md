<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToArith

## Purpose
Declares the TOSA to Arith lowering pass. Lowers TOSA (Tensor Operator Set Architecture) scalar and element-wise arithmetic ops to Arith dialect ops.

## Key Files
| File | Description |
|------|-------------|
| `TosaToArith.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `TosaToArith.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/TosaToArith/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tosa/`
- Target dialect: `include/mlir/Dialect/Arith/`

<!-- MANUAL: -->
