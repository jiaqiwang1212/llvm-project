<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ShapeToStandard

## Purpose
Declares the Shape to Standard lowering pass. Lowers Shape dialect ops (shape.broadcast, shape.num_elements, etc.) to standard Arith, Tensor, and SCF ops.

## Key Files
| File | Description |
|------|-------------|
| `ShapeToStandard.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ShapeToStandard.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ShapeToStandard/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Shape/`
- Target dialects: `include/mlir/Dialect/Arith/`, `include/mlir/Dialect/Tensor/`, `include/mlir/Dialect/SCF/`

<!-- MANUAL: -->
