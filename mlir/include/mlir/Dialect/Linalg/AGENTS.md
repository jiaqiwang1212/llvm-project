<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg Dialect

## Purpose
Structured linear algebra and tensor compute dialect. Provides named ops (matmul, conv, pooling) and a generic op (`linalg.generic`) for expressing structured computations as index-projected loops over tensors or memrefs. Central to MLIR's tensor compilation pipeline.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, interface, and structured op definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation implementations (see `Transforms/AGENTS.md`) |
| `Utils/` | Utility functions (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `linalg.generic` is the canonical structured op; named ops are defined via YAML and ODS
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- All structured ops implement `LinalgOp` interface
- Indexing maps (`AffineMap`) define how loop indices access operand dimensions
- Lowered via LinalgToLoops, LinalgToStandard, or vectorization transforms

## Dependencies
- Depends on: Tensor dialect, MemRef dialect, Affine dialect, Vector dialect

<!-- MANUAL: -->
