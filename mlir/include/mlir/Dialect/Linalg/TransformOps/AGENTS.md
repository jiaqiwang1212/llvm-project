<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg TransformOps

## Purpose
Transform dialect extension ops for Linalg: tiling, fusion, padding, vectorization, generalization, decomposition, and structured op matching.

## Key Files
| File | Description |
|------|-------------|
| `LinalgTransformOps.h` | Transform op declarations |
| `LinalgTransformOps.td` | ODS definitions for all linalg transform ops |
| `LinalgMatchOps.h` | Match op declarations (structural predicate ops) |
| `LinalgMatchOps.td` | ODS match op definitions |
| `LinalgTransformEnums.td` | Enum definitions for transform op options |
| `DialectExtension.h` | Extension registration entry point |
| `GPUHeuristics.h` | GPU tiling heuristics for linalg transforms |
| `Syntax.h` | Custom assembly syntax helpers |

## For AI Agents

### Working In This Directory
- Op names follow `transform.structured.*` convention (e.g., `transform.structured.tile_using_forall`)
- Match ops (`transform.structured.match`) select ops by name, interface, or attribute

## Dependencies
- Depends on: Transform IR, Linalg IR, SCF dialect, Vector dialect

<!-- MANUAL: -->
