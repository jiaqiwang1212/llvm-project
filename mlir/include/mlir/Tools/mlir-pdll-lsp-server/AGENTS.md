<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/mlir-pdll-lsp-server/

## Purpose
Entry-point header for the PDLL LSP language server. Provides IDE features for `.pdll` (Pattern Description Language) files including diagnostics, hover, go-to-definition, and code completion.

## Key Files
| File | Description |
|------|-------------|
| `MlirPdllLspServerMain.h` | `MlirPdllLspServerMain()` — main entry point for the PDLL language server |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `MlirPdllLspServerMain(argc, argv)` from the tool's `main()`.
- Dialect registration is not needed since PDLL is a source-level language, not an MLIR dialect.
- PDLL parsing infrastructure is in `mlir/Tools/PDLL/`.

## Dependencies

### Internal
- `mlir/Tools/lsp-server-support/` (LSP protocol)
- `mlir/Tools/PDLL/` (PDLL parser and type system)

### External
- `llvm/Support/`

<!-- MANUAL: -->
