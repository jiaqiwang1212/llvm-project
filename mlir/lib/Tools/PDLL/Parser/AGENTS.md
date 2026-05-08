<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/PDLL/Parser

## Purpose
Implements the PDLL lexer and recursive-descent parser. Converts `.pdll` source text into an `ASTContext`-owned AST. Also provides code completion support for the PDLL LSP server.

## Key Files
| File | Description |
|------|-------------|
| `Lexer.cpp` / `Lexer.h` | `Lexer`: tokenizes PDLL source into keywords, identifiers, operators, and literals |
| `Parser.cpp` | `Parser`: recursive-descent parser producing PDLL AST nodes; performs name resolution and basic type checking during parse |
| `CodeComplete.cpp` | `CodeCompleter`: provides LSP code completion suggestions by tracking the cursor position during a partial parse |

## For AI Agents

### Working In This Directory
- The parser performs single-pass name resolution; forward references to pattern names are not supported.
- Code completion works by injecting a sentinel token at the cursor position and catching the resulting parse state.
- Grammar is defined implicitly in `Parser.cpp`; there is no separate grammar file.

### Common Patterns
- Parser methods return `FailureOr<ASTNodeType*>` and emit diagnostics on failure.
- The `Lexer` is driven by the parser; the parser calls `lexer.lexToken()` on demand.

## Dependencies

### Internal
- `mlir/lib/Tools/PDLL/AST/` — AST node types
- `mlir/lib/Tools/PDLL/ODS/` — op/constraint lookups during parse

### External
- `llvm/lib/Support` — `llvm::SourceMgr`, `llvm::SMLoc`

<!-- MANUAL: -->
