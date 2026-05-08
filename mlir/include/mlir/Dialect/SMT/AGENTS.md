<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SMT Dialect

## Purpose
Satisfiability Modulo Theories (SMT) dialect. Represents SMT-LIB formulas in MLIR IR: bit-vector ops, integer arithmetic, array theory, Boolean connectives, and solver interaction ops. Used for formal verification and bounded model checking.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, and attribute definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- SMT ops model the SMT-LIB 2 standard theories
- Used by the Transform dialect's SMT extension for verification-driven transforms
- Op names follow `smt.*` convention

### Common Patterns
- `smt.bv<N>` is the N-bit bit-vector type; `smt.int` is unbounded integer
- `smt.solve` / `smt.check` drive the solver interaction

## Dependencies
- Depends on: MLIR IR core; integrates with external SMT solvers at runtime

<!-- MANUAL: -->
