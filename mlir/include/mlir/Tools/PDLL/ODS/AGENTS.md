<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/PDLL/ODS/

## Purpose
ODS (Operation Definition Spec) integration for PDLL. Provides a `Context` that holds op, type, constraint, and dialect definitions extracted from TableGen (`.td`) files, making them available to the PDLL parser for type checking and code completion.

## Key Files
| File | Description |
|------|-------------|
| `Context.h` | `ods::Context` — registry of op/type/constraint/dialect definitions for PDLL type checking |
| `Dialect.h` | `ods::Dialect` — a dialect definition with its namespace and ops |
| `Operation.h` | `ods::Operation` — an op definition with argument/result/attribute types |
| `Constraint.h` | `ods::Constraint` — a type/attr constraint definition |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- The ODS context is populated by reading `.td` files via the TableGen infrastructure before invoking the PDLL parser.
- Op definitions in the ODS context enable PDLL to type-check pattern variables against known op signatures.
- Used exclusively by the PDLL frontend; not related to runtime pattern execution.

### Common Patterns
- Populate ODS: `ods::Context odsCtx; populateODSFromTableGen(tablegenRecords, odsCtx);`
- Then pass to parser: `parsePDLL(sourceMgr, astCtx, odsCtx, nullptr);`

## Dependencies

### Internal
- `mlir/Tools/PDLL/AST/` (types used to represent ODS constraints in PDLL)

### External
- `llvm/TableGen/Record.h` (TableGen records for ODS extraction)
- `llvm/ADT/` (StringRef, SmallVector)

<!-- MANUAL: -->
