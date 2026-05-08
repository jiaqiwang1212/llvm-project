<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU TransformOps

## Purpose
Transform dialect extension ops for GPU-level transformations: mapping loops to GPU threads/blocks, GPU kernel outlining, and GPU-specific tiling.

## Key Files
| File | Description |
|------|-------------|
| `GPUTransformOps.h` | Transform op declarations |
| `GPUTransformOps.td` | ODS definitions |
| `Utils.h` | Shared utilities for GPU transform ops |

## For AI Agents

### Working In This Directory
- Op names follow `transform.gpu.*` convention
- `transform.gpu.map_forall_to_blocks` and `transform.gpu.map_nested_forall_to_threads` are key ops

## Dependencies
- Depends on: Transform IR, GPU dialect IR, SCF dialect

<!-- MANUAL: -->
