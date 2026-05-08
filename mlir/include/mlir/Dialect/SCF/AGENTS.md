<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF Dialect

## Purpose
Structured Control Flow dialect. Provides structured loop and conditional ops (`scf.for`, `scf.while`, `scf.if`, `scf.forall`, `scf.parallel`) that preserve structured semantics while allowing SSA values as loop-carried state.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and interface definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | SCF utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- SCF ops yield values via `scf.yield`; loop-carried state is explicit in op results
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- `scf.for` is the canonical counted loop; `scf.forall` is the parallel loop
- Lowered to ControlFlow dialect (CFG) via SCFToControlFlow conversion

## Dependencies
- Depends on: Arith dialect, Index dialect (for loop bounds)

<!-- MANUAL: -->
