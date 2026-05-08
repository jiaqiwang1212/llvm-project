<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/tblgen-lsp-server

## Purpose
Implements the TableGen LSP server (`tblgen-lsp-server`). Provides language features for `.td` (TableGen) files used in MLIR ODS/DRR: diagnostics, hover, go-to-definition, and document symbols backed by the LLVM TableGen parser.

## Key Files
| File | Description |
|------|-------------|
| `TableGenLspServerMain.cpp` | Entry point: `mlir::tblgen::TableGenLspServerMain()` |
| `TableGenServer.cpp` / `TableGenServer.h` | `TableGenServer`: parses `.td` files using the LLVM TableGen parser, indexes record definitions, and answers LSP queries |
| `LSPServer.cpp` / `LSPServer.h` | LSP request router for TableGen methods |

## For AI Agents

### Working In This Directory
- Depends on `llvm/lib/TableGen` for parsing; MLIR-specific knowledge (ODS record types) is layered on top.
- Include paths for `.td` files come from `CompilationDatabase` or the `--include` flags passed to the server.

### Common Patterns
- Record definitions are indexed by name; hover uses the `RecordVal` from the parsed `RecordKeeper`.

## Dependencies

### Internal
- `mlir/lib/Tools/lsp-server-support/` — JSON-RPC, `CompilationDatabase`

### External
- `llvm/lib/TableGen` — `llvm::RecordKeeper`, parser
- `llvm/lib/Support` — `llvm::json`, `llvm::SourceMgr`

<!-- MANUAL: -->
