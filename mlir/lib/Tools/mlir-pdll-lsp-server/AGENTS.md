<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-pdll-lsp-server

## Purpose
Implements the PDLL (Pattern Description Language) LSP server (`mlir-pdll-lsp-server`). Provides language features for `.pdll` pattern files: diagnostics, hover, go-to-definition, document symbols, and code completion backed by the PDLL compiler's AST.

## Key Files
| File | Description |
|------|-------------|
| `MlirPdllLspServerMain.cpp` | Entry point: `mlir::MlirPdllLspServerMain()` |
| `PDLLServer.cpp` / `PDLLServer.h` | `PDLLServer`: parses `.pdll` files using the PDLL compiler, builds an index, and answers LSP queries |
| `LSPServer.cpp` / `LSPServer.h` | LSP request router for PDLL-specific methods |
| `Protocol.cpp` / `Protocol.h` | PDLL-specific LSP protocol extensions |

## For AI Agents

### Working In This Directory
- Depends on the PDLL compiler in `mlir/lib/Tools/PDLL/`; changes to the PDLL AST/parser may require updates here.
- The server uses the PDLL compiler's `ASTContext` (not MLIR `MLIRContext`) for indexing.

### Common Patterns
- `.pdll` files are parsed by `mlir::pdll::parseFile()` from the PDLL library; the AST is walked to build the source index.

## Dependencies

### Internal
- `mlir/lib/Tools/lsp-server-support/` — JSON-RPC, protocol base
- `mlir/lib/Tools/PDLL/` — PDLL compiler library

### External
- `llvm/lib/Support` — `llvm::json`, `llvm::SourceMgr`

<!-- MANUAL: -->
