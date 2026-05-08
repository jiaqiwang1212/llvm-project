<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-query

## Purpose
Implements `MlirQueryMain()`, the entry-point library for the `mlir-query` tool. Parses an MLIR input file, then runs queries (interactively or from `--query` flags) against the IR using the matcher DSL from `mlir/lib/Query/`.

## Key Files
| File | Description |
|------|-------------|
| `MlirQueryMain.cpp` | `mlir::MlirQueryMain()`: sets up `MLIRContext`, loads the input file, then enters a REPL or processes `--query` arguments by calling `mlir::query::runQuery()` |

## For AI Agents

### Working In This Directory
- Query execution is in `mlir/lib/Query/`; this file is only the CLI driver.
- REPL mode uses `llvm::LineEditor` for readline-like input.

## Dependencies

### Internal
- `mlir/lib/Query/` — query engine
- `mlir/lib/Parser/` — input parsing

### External
- `llvm/lib/Support` — `llvm::LineEditor`, `llvm::cl`

<!-- MANUAL: -->
