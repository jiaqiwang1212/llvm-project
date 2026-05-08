<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shape IR

## Purpose
Core op definitions for the Shape dialect: shape computation, broadcasting, constraint checking, and shape witness ops.

## Key Files
| File | Description |
|------|-------------|
| `Shape.h` | Op class declarations |
| `ShapeBase.td` | Dialect definition and base classes |
| `ShapeOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Edit `ShapeOps.td` to add new shape computation ops
- `shape.assuming` is the key op for guarding computations behind shape constraints

## Dependencies
- Depends on: MLIR built-in types, Tensor dialect

<!-- MANUAL: -->
