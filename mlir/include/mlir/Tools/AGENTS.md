<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/

## Purpose
Entry-point headers for MLIR command-line tools and LSP language servers. Each subdirectory contains the `*Main()` function declaration that tool executables call after registering dialects and passes. Also contains the PDLL language tooling headers and a shared plugin loading interface.

## Key Files
| File | Description |
|------|-------------|
| `ParseUtilities.h` | `parseSourceFileForTool()` — shared source parsing with split-file support for tools |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `mlir-opt/` | `MlirOptMain()` entry point for `mlir-opt`-style optimization driver tools (see `mlir-opt/AGENTS.md`) |
| `mlir-translate/` | `MlirTranslateMain()` entry point for translation tools (see `mlir-translate/AGENTS.md`) |
| `mlir-lsp-server/` | `MlirLspServerMain()` for the MLIR LSP language server (see `mlir-lsp-server/AGENTS.md`) |
| `mlir-pdll-lsp-server/` | `MlirPdllLspServerMain()` for the PDLL LSP server (see `mlir-pdll-lsp-server/AGENTS.md`) |
| `mlir-query/` | `MlirQueryMain()` for the `mlir-query` tool (see `mlir-query/AGENTS.md`) |
| `mlir-reduce/` | `MlirReduceMain()` for the `mlir-reduce` tool (see `mlir-reduce/AGENTS.md`) |
| `mlir-tblgen/` | `MlirTblgenMain()` for `mlir-tblgen` backends (see `mlir-tblgen/AGENTS.md`) |
| `lsp-server-support/` | Shared LSP protocol implementation used by all MLIR LSP servers (see `lsp-server-support/AGENTS.md`) |
| `tblgen-lsp-server/` | `TableGenLspServerMain()` for the TableGen LSP server (see `tblgen-lsp-server/AGENTS.md`) |
| `PDLL/` | PDLL language AST, parser, code generator, and ODS headers (see `PDLL/AGENTS.md`) |
| `Plugins/` | `DialectPlugin`/`PassPlugin` loading for dynamic extensions (see `Plugins/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- To create a new `mlir-opt`-style tool, call `MlirOptMain()` from `mlir-opt/MlirOptMain.h` after registering dialects/passes.
- LSP servers share the protocol layer in `lsp-server-support/`; dialect-specific servers add on top.
- Plugin headers in `Plugins/` enable loading shared libraries that register dialects or passes at runtime.

### Common Patterns
- Tool main: `int main(int argc, char **argv) { registerMyDialects(); return mlir::MlirOptMain(argc, argv, "my-opt", registry); }`
- Split-file testing: `mlir-opt` automatically splits files on `// -----` using `ParseUtilities.h`.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, DialectRegistry)
- `mlir/Pass/` (PassManager, PassRegistry)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (CommandLine, SourceMgr)

<!-- MANUAL: -->
