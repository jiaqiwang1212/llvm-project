<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor TransformOps

## Purpose
Transform dialect extension ops for Tensor: pad insertion, splitting, packing, and other tensor-level transformations from Transform IR scripts.

## Key Files
| File | Description |
|------|-------------|
| `TensorTransformOps.h` | Transform op declarations |
| `TensorTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.tensor.*` convention
- Register via `registerTensorTransformOps`

## Dependencies
- Depends on: Transform IR, Tensor dialect IR

<!-- MANUAL: -->
