<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToTensor

## Purpose
Declares the TOSA to Tensor lowering pass. Lowers TOSA tensor reshape and slice ops to Tensor dialect ops (reshape, extract_slice, insert_slice).

## Key Files
| File | Description |
|------|-------------|
| `TosaToTensor.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `TosaToTensor.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/TosaToTensor/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tosa/`
- Target dialect: `include/mlir/Dialect/Tensor/`

<!-- MANUAL: -->
