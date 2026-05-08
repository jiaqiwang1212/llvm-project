<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AffineToStandard

## Purpose
Declares the Affine to Standard lowering pass. Lowers affine dialect ops (loops, if, apply, load/store) to standard SCF, arithmetic, and memref ops.

## Key Files
| File | Description |
|------|-------------|
| `AffineToStandard.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `AffineToStandard.h` to add or modify pass declarations and pattern helpers
- The implementation lives under `mlir/lib/Conversion/AffineToStandard/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Affine/`
- Target dialects: `include/mlir/Dialect/SCF/`, `include/mlir/Dialect/Arith/`, `include/mlir/Dialect/MemRef/`

<!-- MANUAL: -->
