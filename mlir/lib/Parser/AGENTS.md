<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Parser

## Purpose
Top-level entry point for parsing textual MLIR assembly. This thin wrapper ties together the lower-level `AsmParser` lexer/parser machinery and exposes the public `mlir::parseSourceFile()` / `mlir::parseSourceString()` API used by tools and tests.

## Key Files
| File | Description |
|------|-------------|
| `Parser.cpp` | Implements `mlir::parseSourceFile()`, `mlir::parseSourceString()`, and `mlir::parseAsmSourceFile()`. Sets up the `ParserState`, drives the top-level module parse loop, and handles source manager integration |

## For AI Agents

### Working In This Directory
- The actual grammar rules live in `mlir/lib/AsmParser/`; this directory is purely the public entry point.
- Source locations are anchored to an `llvm::SourceMgr` passed in at parse time.
- Round-trip fidelity (print then parse yields same IR) is tested extensively in `mlir/test/`.

### Common Patterns
- Tools call `parseSourceFile(sourceMgrRef, &block, &context)` to get ops into an existing block.
- `parseSourceString` is the in-memory variant without a file path.

## Dependencies

### Internal
- `mlir/lib/AsmParser/` — all grammar and lexer implementation
- `mlir/lib/IR/` — `MLIRContext`, `Block`, `Operation`

### External
- `llvm/lib/Support` — `llvm::SourceMgr`, `llvm::MemoryBuffer`

<!-- MANUAL: -->
