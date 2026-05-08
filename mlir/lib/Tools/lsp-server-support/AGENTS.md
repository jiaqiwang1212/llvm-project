<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/lsp-server-support

## Purpose
Provides shared infrastructure for all MLIR LSP (Language Server Protocol) servers. Implements the JSON-RPC transport layer, LSP protocol message types, source manager integration, and compilation database support. All three MLIR LSP servers (`mlir-lsp-server`, `mlir-pdll-lsp-server`, `tblgen-lsp-server`) build on this library.

## Key Files
| File | Description |
|------|-------------|
| `CompilationDatabase.cpp` | `CompilationDatabase`: reads `compile_commands.json` to find per-file compiler flags; used by LSP servers to replicate the correct include paths and defines when parsing files |
| `SourceMgrUtils.cpp` | Utilities bridging `llvm::SourceMgr` (used by MLIR parsers) and LSP source location types (`lsp::Position`, `lsp::Range`, `lsp::Location`) |

## For AI Agents

### Working In This Directory
- The JSON-RPC transport and LSP protocol types are defined here; do not duplicate them in individual server subdirectories.
- When adding a new LSP capability (e.g., code actions, semantic tokens), add the protocol types and transport here first.
- LSP position/range types use 0-based UTF-16 offsets per the LSP spec; `SourceMgrUtils.cpp` handles the conversion from `llvm::SMLoc`.

### Common Patterns
- LSP servers inherit from `lsp::JSONTransport` and implement request/notification handlers.
- `CompilationDatabase::getCompileCommand(file)` returns the flags to use when parsing a given file.

## Dependencies

### Internal
- `mlir/lib/IR/` — `MLIRContext`, source manager integration

### External
- `llvm/lib/Support` — `llvm::SourceMgr`, JSON support (`llvm::json`), ADT

<!-- MANUAL: -->
