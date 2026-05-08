<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shape Dialect

## Purpose
Implements the Shape dialect — provides ops for computing and constraining tensor shapes at runtime. Models `shape.shape_of`, `shape.dim`, `shape.broadcast`, `shape.assume`, `shape.cstr_*` constraint ops, and `!shape.shape`/`!shape.size` types for dynamic shape computation.

## Key Files
| File | Description |
|------|-------------|
| `IR/Shape.cpp` | All shape op implementations and folders |
| `IR/ShapeCanonicalization.td` | Canonicalization patterns for shape ops |
| `Transforms/OutlineShapeComputation.cpp` | Outlines shape computations into separate functions |
| `Transforms/RemoveShapeConstraints.cpp` | Removes `shape.assuming` regions after verification |
| `Transforms/ShapeToShapeLowering.cpp` | Lowers complex shape ops to simpler combinations |
| `Transforms/BufferizableOpInterfaceImpl.cpp` | Bufferization support |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Shape ops and canonicalization |
| `Transforms/` | Shape computation outlining, constraint removal, lowering |

## For AI Agents

### Working In This Directory
- `!shape.shape` represents an unknown-rank or dynamic-shape tensor shape; `!shape.size` is a scalar dimension value.
- `shape.assuming` creates a region that assumes a `shape.witness` constraint holds; after shape verification passes, `RemoveShapeConstraints` can eliminate these regions.
- `shape.broadcast` computes the broadcast-compatible shape of two shapes — key for elementwise op shape inference.
- `ShapeToStandard` conversion lowers shape ops to `index` arithmetic and `memref` operations.
- Canonicalization in `ShapeCanonicalization.td` folds static shapes at compile time.

### Common Patterns
- Shape constraint verification: `shape.cstr_broadcastable`, `shape.cstr_eq` produce witnesses consumed by `shape.assuming`.
- `OutlineShapeComputation` is used in ahead-of-time compilation to separate shape from data computation.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/Tensor`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
