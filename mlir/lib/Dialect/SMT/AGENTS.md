<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SMT Dialect

## Purpose
Implements the SMT (Satisfiability Modulo Theories) dialect — an MLIR representation of SMT-LIB formulas. Models SMT sorts (`!smt.bool`, `!smt.bv<N>`, `!smt.int`, `!smt.array<K,V>`), op formulas, and solver interaction ops for formal verification use cases within MLIR.

## Key Files
| File | Description |
|------|-------------|
| `IR/SMTDialect.cpp` | Dialect registration |
| `IR/SMTOps.cpp` | SMT op implementations: bit-vector arithmetic, boolean logic, array ops, solver ops |
| `IR/SMTAttributes.cpp` | SMT attribute definitions (bit-vector literals, etc.) |
| `IR/SMTTypes.cpp` | SMT sort type implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | SMT sorts, formula ops, solver interaction ops |

## For AI Agents

### Working In This Directory
- SMT dialect is used by CIRCT and MLIR verification tools to encode IR properties as SMT formulas.
- `!smt.bv<N>` is a bitvector sort of width N; arithmetic ops on it correspond to SMT-LIB bitvector theory.
- `smt.solver` creates a solver context; `smt.assert`, `smt.check`, `smt.yield` interact with it.
- No lowering passes in this directory — the SMT dialect is either consumed by a solver backend or used as a target for formal verification passes.
- `Transform/SMTExtension/` in the Transform dialect provides Transform ops that generate SMT queries.

### Common Patterns
- Formula construction: chain SMT ops (bvadd, bvult, etc.) in a `smt.solver` region.
- Solver queries: `smt.check` returns `sat`/`unsat`/`unknown`; use `smt.yield` to extract model values.

## Dependencies
- `mlir/IR`, CIRCT SMT backend (external)

<!-- MANUAL: -->
