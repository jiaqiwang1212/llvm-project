<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-lsp-server/

## Purpose
Entry-point header for the MLIR LSP (Language Server Protocol) language server. The server provides IDE features (hover, go-to-definition, diagnostics, code completion) for `.mlir` files. `MlirLspServerMain()` is the main entry point called after registering dialects.

## Key Files
| File | Description |
|------|-------------|
| `MlirLspServerMain.h` | `MlirLspServerMain()` — entry point for the MLIR language server |
| `MlirLspRegistryFunction.h` | `MlirLspRegistryFunction` typedef for dialect registry callbacks |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `MlirLspServerMain(argc, argv, registryFn)` where `registryFn` registers all dialects.
- The server communicates via stdin/stdout using the JSON-RPC LSP protocol.
- LSP protocol implementation is in `lsp-server-support/`; this header is just the entry point.
- `AsmParserState` (from `mlir/AsmParser/`) is used to track source locations for LSP features.

### Common Patterns
- `int main(int argc, char **argv) { return mlir::MlirLspServerMain(argc, argv, registerMyDialects); }`

## Dependencies

### Internal
- `mlir/Tools/lsp-server-support/` (LSP protocol implementation)
- `mlir/IR/` (DialectRegistry, MLIRContext)
- `mlir/AsmParser/` (AsmParserState)

### External
- `llvm/Support/` (JSON, raw_ostream)

<!-- MANUAL: -->
