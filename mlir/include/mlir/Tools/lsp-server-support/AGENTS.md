<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/lsp-server-support/

## Purpose
Shared LSP (Language Server Protocol) protocol implementation used by all MLIR LSP servers (MLIR, PDLL, TableGen). Provides the JSON-RPC transport, LSP message types, compilation database support, and source manager utilities that language-specific servers build on top of.

## Key Files
| File | Description |
|------|-------------|
| `CompilationDatabase.h` | `CompilationDatabase` — loads compile commands (flags, include paths) for LSP diagnostics |
| `SourceMgrUtils.h` | Utilities for mapping LSP `Position`/`Range` to LLVM `SourceMgr` locations |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- This is a shared library header; individual LSP servers (`mlir-lsp-server/`, `mlir-pdll-lsp-server/`, etc.) depend on it.
- `CompilationDatabase` reads `compile_commands.json` to provide accurate include paths for the LSP.
- `SourceMgrUtils.h` bridges between LSP line/column positions and LLVM `SMLoc` values.
- The JSON-RPC transport is implemented in `mlir/lib/Tools/lsp-server-support/`.

### Common Patterns
- Load compile database: `CompilationDatabase db; db.loadFromDirectory(workspaceDir);`
- Position conversion: `mlir::lsp::convertToSMLoc(position, sourceMgr)`.

## Dependencies

### Internal
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (SourceMgr, JSON, raw_ostream)

<!-- MANUAL: -->
