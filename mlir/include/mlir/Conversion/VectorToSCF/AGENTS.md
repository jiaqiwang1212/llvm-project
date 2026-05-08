<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToSCF

## Purpose
Declares the Vector to SCF lowering pass. Lowers Vector dialect transfer ops (transfer_read/write) to SCF loops and memref scalar loads/stores, enabling progressive vectorization lowering.

## Key Files
| File | Description |
|------|-------------|
| `VectorToSCF.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `VectorToSCF.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/VectorToSCF/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialects: `include/mlir/Dialect/SCF/`, `include/mlir/Dialect/MemRef/`

<!-- MANUAL: -->
