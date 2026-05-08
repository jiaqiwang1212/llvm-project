<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/SMTLIB/

## Purpose
Headers for exporting MLIR SMT dialect operations to SMTLIB2 format. Enables using MLIR's SMT dialect as a frontend for SMT solver interaction by generating standard `.smt2` text files.

## Key Files
| File | Description |
|------|-------------|
| `ExportSMTLIB.h` | `exportSMTLIB()` — translate an MLIR module with SMT ops to SMTLIB2 text |
| `Namespace.h` | Namespace management for SMTLIB symbol naming |
| `SymCache.h` | Symbol cache for SMTLIB export (avoids duplicate declarations) |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Input must be an MLIR module using the SMT dialect (`mlir/Dialect/SMT/`).
- Output is standard SMTLIB2 text consumable by Z3, CVC5, or other SMT solvers.
- Registered as `--export-smtlib` translation in `mlir-translate`.

## Dependencies

### Internal
- `mlir/Dialect/SMT/` (SMT dialect ops)
- `mlir/IR/` (ModuleOp)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/raw_ostream.h`

<!-- MANUAL: -->
