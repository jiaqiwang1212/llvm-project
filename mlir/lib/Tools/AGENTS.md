<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools

## Purpose
Implements the driver logic for all MLIR command-line tools. Each subdirectory provides a `*Main()` entry point that is linked into the corresponding binary. Tools include the optimizer (`mlir-opt`), translator (`mlir-translate`), LSP language servers, the query tool, the reducer, and the TableGen code generator.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `mlir-opt/` | `mlir-opt` driver: loads passes, parses pipelines, and runs them on input files (see `mlir-opt/AGENTS.md`) |
| `mlir-translate/` | `mlir-translate` driver: dispatches to registered `TranslateFromMLIR` / `TranslateToMLIR` functions (see `mlir-translate/AGENTS.md`) |
| `mlir-lsp-server/` | MLIR `.mlir` file LSP server: hover, go-to-definition, diagnostics for MLIR assembly (see `mlir-lsp-server/AGENTS.md`) |
| `mlir-pdll-lsp-server/` | PDLL `.pdll` file LSP server: language support for the PDL DSL (see `mlir-pdll-lsp-server/AGENTS.md`) |
| `tblgen-lsp-server/` | TableGen `.td` file LSP server: language support for TableGen/ODS files (see `tblgen-lsp-server/AGENTS.md`) |
| `lsp-server-support/` | Shared LSP protocol infrastructure: JSON-RPC, `CompilationDatabase`, source manager utilities (see `lsp-server-support/AGENTS.md`) |
| `mlir-query/` | `mlir-query` driver: REPL and batch query runner for the query DSL (see `mlir-query/AGENTS.md`) |
| `mlir-reduce/` | `mlir-reduce` driver: entry point for the IR minimization tool (see `mlir-reduce/AGENTS.md`) |
| `mlir-tblgen/` | `mlir-tblgen` driver: dispatches to registered TableGen backends (see `mlir-tblgen/AGENTS.md`) |
| `PDLL/` | PDLL compiler library: AST, parser, code generator for `.pdll` pattern files (see `PDLL/AGENTS.md`) |
| `Plugins/` | Dynamic plugin loading: dialect plugins and pass plugins via `dlopen` (see `Plugins/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Tool drivers are thin; all business logic is in the corresponding library (e.g., `mlir-opt` delegates to `MlirOptMain` which calls the pass manager).
- LSP servers share JSON-RPC infrastructure from `lsp-server-support/`; new servers should reuse it.
- When adding a new tool, add a subdirectory here with a `*Main.cpp`, add a CMake `add_mlir_tool()` target, and wire up the binary in `mlir/tools/`.

### Common Patterns
- Tool `main()` functions live in `mlir/tools/<tool-name>/` (not here); this directory provides the `*Main()` library function.
- All tools call `mlir::registerMLIRContextCLOptions()` and `mlir::registerPassManagerCLOptions()` before `cl::ParseCommandLineOptions`.

## Dependencies

### Internal
- `mlir/lib/IR/`, `mlir/lib/Pass/`, `mlir/lib/Parser/` — used by nearly every tool
- `mlir/lib/TableGen/` — used by `mlir-tblgen/`
- `mlir/lib/Query/`, `mlir/lib/Reducer/` — used by `mlir-query/`, `mlir-reduce/`

### External
- `llvm/lib/Support` — command-line option parsing, `llvm::SourceMgr`

<!-- MANUAL: -->
