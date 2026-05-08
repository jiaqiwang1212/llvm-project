<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/SMTLIB

## Purpose
Implements export of the MLIR SMT dialect to SMTLIB2 text format. Enables using external SMT solvers (Z3, CVC5, etc.) by generating `.smt2` files from MLIR SMT dialect IR.

## Key Files
| File | Description |
|------|-------------|
| `ExportSMTLIB.cpp` | `mlir::exportSMTLIB()`: walks SMT dialect ops and emits SMTLIB2 `(assert ...)`, `(declare-fun ...)`, `(check-sat)` statements to an `llvm::raw_ostream` |

## For AI Agents

### Working In This Directory
- The SMT dialect is in `mlir/lib/Dialect/SMT/`; add new op translations there first.
- SMTLIB2 syntax spec: http://smtlib.cs.uiowa.edu/papers/smt-lib-reference-v2.6-r2021-05-12.pdf

## Dependencies

### Internal
- `mlir/lib/Dialect/SMT/` — SMT dialect ops

### External
- `llvm/lib/Support` — `llvm::raw_ostream`

<!-- MANUAL: -->
