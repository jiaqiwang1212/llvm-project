<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-query/

## Purpose
Entry-point header for the `mlir-query` interactive query tool. Provides `MlirQueryMain()` which runs an interactive or batch query session against a loaded MLIR module using the matcher framework from `mlir/Query/`.

## Key Files
| File | Description |
|------|-------------|
| `MlirQueryMain.h` | `MlirQueryMain()` — main entry point for the mlir-query tool |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `MlirQueryMain(argc, argv, registry)` after registering dialects.
- The query DSL is defined in `mlir/Query/Matcher/` and parsed at runtime.
- Supports both interactive (REPL) and batch (`-c "query"`) modes.

## Dependencies

### Internal
- `mlir/Query/` (Query, QuerySession, Matcher framework)
- `mlir/IR/` (MLIRContext, DialectRegistry)

### External
- `llvm/Support/` (raw_ostream)
- `llvm/LineEditor/` (REPL line editing)

<!-- MANUAL: -->
