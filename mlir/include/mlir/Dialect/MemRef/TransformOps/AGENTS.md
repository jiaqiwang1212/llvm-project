<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRef TransformOps

## Purpose
Transform dialect extension ops for MemRef: multibuffering, allocation hoisting, and memory-layout transformations driven from Transform IR scripts.

## Key Files
| File | Description |
|------|-------------|
| `MemRefTransformOps.h` | Transform op declarations |
| `MemRefTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.memref.*` convention
- Register via `registerMemRefTransformOps`

## Dependencies
- Depends on: Transform IR, MemRef dialect IR

<!-- MANUAL: -->
