<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-lsp-server

## Purpose
Implements the MLIR assembly LSP server (`mlir-lsp-server`). Provides IDE language features (hover, go-to-definition, diagnostics, document symbols, code completion) for `.mlir` text files by parsing them incrementally and querying the `AsmParserState`.

## Key Files
| File | Description |
|------|-------------|
| `MlirLspServerMain.cpp` | Entry point: `mlir::MlirLspServerMain()` — initializes the server and starts the JSON-RPC dispatch loop |
| `MLIRServer.cpp` / `MLIRServer.h` | `MLIRServer`: the server implementation; handles LSP requests by parsing `.mlir` files, indexing definitions/uses via `AsmParserState`, and answering hover/definition/completion queries |
| `LSPServer.cpp` / `LSPServer.h` | LSP request router: maps JSON-RPC method names to `MLIRServer` handlers |
| `Protocol.cpp` / `Protocol.h` | MLIR-specific LSP protocol extensions (beyond the base protocol in `lsp-server-support/`) |

## For AI Agents

### Working In This Directory
- `MLIRServer` re-parses the file on every `textDocument/didChange`; parsing uses `AsmParserState` to record source spans for all ops, values, types, and attributes.
- Go-to-definition works by looking up the `AsmParserState` definition entry for the hovered source position.
- New LSP capabilities require: (1) adding protocol types to `Protocol.h`, (2) adding a handler in `LSPServer.cpp`, (3) implementing the query in `MLIRServer.cpp`.

### Common Patterns
- Source positions are `lsp::Position` (0-based line/character); converted to `llvm::SMLoc` via `lsp-server-support/SourceMgrUtils.cpp`.
- The server uses `mlir::parseSourceFile()` with an `AsmParserState` observer to build the symbol index.

## Dependencies

### Internal
- `mlir/lib/Tools/lsp-server-support/` — JSON-RPC transport, `CompilationDatabase`
- `mlir/lib/AsmParser/AsmParserState.cpp` — source-level index
- `mlir/lib/IR/` — `MLIRContext`

### External
- `llvm/lib/Support` — `llvm::json`, `llvm::SourceMgr`

<!-- MANUAL: -->
