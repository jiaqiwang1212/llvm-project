<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Math IR

## Purpose
Core op definitions for the Math dialect: transcendental and special mathematical functions.

## Key Files
| File | Description |
|------|-------------|
| `Math.h` | Op class declarations |
| `MathBase.td` | Dialect definition and base classes |
| `MathOps.td` | ODS op definitions (exp, log, sin, cos, sqrt, etc.) |

## For AI Agents

### Working In This Directory
- Edit `MathOps.td` to add new mathematical ops
- Ops are defined to work on scalars and vectors of float types

## Dependencies
- Depends on: Arith dialect (for generated lowering patterns)

<!-- MANUAL: -->
