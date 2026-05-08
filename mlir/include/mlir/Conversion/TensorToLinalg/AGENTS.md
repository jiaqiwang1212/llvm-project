<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TensorToLinalg

## Purpose
Declares the Tensor to Linalg lowering pass. Lowers Tensor dialect ops (pack, unpack, pad) to Linalg generic ops for further loop-level optimization.

## Key Files
| File | Description |
|------|-------------|
| `TensorToLinalg.h` | Conversion pattern declarations |
| `TensorToLinalgPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `TensorToLinalgPass.h` for pass registration; edit `TensorToLinalg.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/TensorToLinalg/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tensor/`
- Target dialect: `include/mlir/Dialect/Linalg/`

<!-- MANUAL: -->
