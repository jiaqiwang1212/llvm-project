<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/tblgen-lsp-server/

## Purpose
Entry-point header for the TableGen LSP language server. Provides IDE features for `.td` (TableGen) files used in both LLVM and MLIR projects, including diagnostics, hover documentation, and go-to-definition.

## Key Files
| File | Description |
|------|-------------|
| `TableGenLspServerMain.h` | `TableGenLspServerMain()` — main entry point for the TableGen language server |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Call `TableGenLspServerMain(argc, argv)` from the tool's `main()`.
- The server understands both LLVM TableGen and MLIR-specific TableGen constructs.
- Uses `lsp-server-support/` for the underlying LSP transport.

## Dependencies

### Internal
- `mlir/Tools/lsp-server-support/`

### External
- `llvm/TableGen/` (TableGen parser)
- `llvm/Support/`

<!-- MANUAL: -->
