<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform LoopExtension

## Purpose
Transform dialect extension providing loop-level transform ops: loop peeling, loop coalescing, loop unrolling, and other loop manipulation ops not tied to a specific loop dialect.

## Key Files
| File | Description |
|------|-------------|
| `LoopExtension.h` | Extension registration declaration |
| `LoopExtensionOps.h` | Op class declarations |
| `LoopExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.loop.*` convention
- Works on ops implementing `LoopLikeOpInterface`

## Dependencies
- Depends on: Transform IR, SCF dialect

<!-- MANUAL: -->
