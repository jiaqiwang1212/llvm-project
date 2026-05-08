<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/PDLL/ODS

## Purpose
Provides ODS (Op Definition Spec) integration for the PDLL compiler. Loads op, dialect, constraint, and type definitions from compiled ODS data so that PDLL patterns can reference op names, operand types, and constraints without re-parsing TableGen.

## Key Files
| File | Description |
|------|-------------|
| `Context.cpp` | `ods::Context`: the ODS data store; holds all registered dialects, ops, constraints, and types; looked up by the PDLL parser during name resolution |
| `Dialect.cpp` | `ods::Dialect`: represents a registered dialect with its op and type catalog |
| `Operation.cpp` | `ods::Operation`: represents an op with its operand/result/attribute names and types as seen by PDLL |
| `Constraint.cpp` | `ods::Constraint`: represents a named type/attribute constraint that PDLL patterns can apply |

## For AI Agents

### Working In This Directory
- ODS data is populated either from TableGen records (when invoked from `mlir-tblgen`) or from live `MLIRContext` introspection (when used by the LSP server).
- Changes to how ODS encodes op arguments/results in TableGen require updates to how `ods::Operation` is populated.

### Common Patterns
- `ods::Context` is populated before parsing begins; the parser calls `odsContext.lookupOperation(dialectName, opName)` to resolve op references.

## Dependencies

### Internal
- `mlir/lib/TableGen/` — ODS record wrappers (for TableGen-driven population)
- `mlir/lib/Tools/PDLL/AST/` — type system (for constraint types)

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
