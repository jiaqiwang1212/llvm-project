<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shape Dialect

## Purpose
Provides ops for shape computation and constraint: `shape.shape_of`, `shape.broadcast`, `shape.assuming`, and shape witnesses. Enables shape inference and dynamic shape computation as first-class IR.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Analysis/` | Shape mapping analysis (see `Analysis/AGENTS.md`) |
| `IR/` | Op definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `shape.shape_of` returns the shape of a tensor as an `!shape.shape` value
- `shape.assuming` creates a witness-gated region for shape-constrained ops
- Lowered to standard ops via ShapeToStandard conversion

### Common Patterns
- Op names follow `shape.*` convention
- `!shape.shape` and `!shape.size` are the primary shape types

## Dependencies
- Depends on: Tensor dialect, MemRef dialect, Arith dialect

<!-- MANUAL: -->
