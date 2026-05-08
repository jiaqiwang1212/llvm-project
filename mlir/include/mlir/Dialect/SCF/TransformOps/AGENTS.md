<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF TransformOps

## Purpose
Transform dialect extension ops for SCF: loop peeling, unrolling, coalescing, loop-invariant code motion, and forall-to-for conversion.

## Key Files
| File | Description |
|------|-------------|
| `SCFTransformOps.h` | Transform op declarations |
| `SCFTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.scf.*` convention
- Register via `registerSCFTransformOps`

## Dependencies
- Depends on: Transform IR, SCF dialect IR

<!-- MANUAL: -->
