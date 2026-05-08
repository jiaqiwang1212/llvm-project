<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization TransformOps

## Purpose
Transform dialect extension ops for driving bufferization from a Transform IR script. Enables selective or staged bufferization of op handles.

## Key Files
| File | Description |
|------|-------------|
| `BufferizationTransformOps.h` | Transform op declarations |
| `BufferizationTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.bufferization.*` convention
- Register via `registerBufferizationTransformOps`

## Dependencies
- Depends on: Transform IR, Bufferization IR

<!-- MANUAL: -->
