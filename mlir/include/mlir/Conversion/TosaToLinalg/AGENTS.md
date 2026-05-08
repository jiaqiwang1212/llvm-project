<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToLinalg

## Purpose
Declares the TOSA to Linalg lowering pass. Lowers TOSA tensor ops (conv, matmul, element-wise) to Linalg named and generic ops for further loop optimization.

## Key Files
| File | Description |
|------|-------------|
| `TosaToLinalg.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `TosaToLinalg.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/TosaToLinalg/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tosa/`
- Target dialect: `include/mlir/Dialect/Linalg/`

<!-- MANUAL: -->
