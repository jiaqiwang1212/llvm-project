<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/AsmParser

## Purpose
Implements the full MLIR textual assembly parser. Converts `.mlir` source text into in-memory `Operation` / `Block` / `Region` trees. The parser is hand-written recursive descent, covering the MLIR generic assembly format as well as custom op syntax registered by dialects.

## Key Files
| File | Description |
|------|-------------|
| `Lexer.cpp` | `Lexer`: tokenizes raw source text into a stream of `Token` objects; handles keywords, identifiers, literals, punctuation, and comments |
| `Token.cpp` | `Token`: token kinds, spelling accessors, and source location attachment |
| `TokenKinds.def` | X-macro table listing all token kind names (included by `Token.h` and `Lexer.cpp`) |
| `Parser.cpp` | Top-level parser: module/function/block/op parsing, SSA value definition/use resolution, region and block argument parsing |
| `AffineParser.cpp` | Parses affine expressions, affine maps, and integer sets within `affine_map<...>` / `affine_set<...>` syntax |
| `AttributeParser.cpp` | Parses built-in and dialect-custom attribute syntax (dense elements, integer, float, string, array, …) |
| `TypeParser.cpp` | Parses built-in and dialect-custom type syntax (integer, float, index, memref, tensor, vector, …) |
| `LocationParser.cpp` | Parses location attributes (`loc(...)` syntax, `callsite`, `fused`, `name`) |
| `DialectSymbolParser.cpp` | Handles `#dialect.attr<...>` and `!dialect.type<...>` dispatch to dialect-registered custom parsers |
| `AsmParserState.cpp` | `AsmParserState`: tracks source spans of ops, values, and types for LSP hover/goto-definition support |
| `AsmParserImpl.h` | Internal `AsmParserImpl` class used as the concrete `AsmParser` passed to dialect `parseAttribute`/`parseType` hooks |
| `Parser.h` | Internal parser class declaration |
| `ParserState.h` | `ParserState`: carries the SSA value table, block forward references, and parse context across recursive calls |

## For AI Agents

### Working In This Directory
- All public parser headers are in `mlir/include/mlir/AsmParser/`; this directory is implementation-only.
- Custom op/attribute/type syntax hooks are called via `Dialect::parseAttribute()`, `Dialect::parseType()`, and op `parse()` static methods — all dispatched from `DialectSymbolParser.cpp` and `Parser.cpp`.
- `AsmParserState` must be updated whenever new syntactic constructs are added so that LSP servers provide correct source locations.
- `TokenKinds.def` must be updated when new keyword tokens are introduced.

### Common Patterns
- Parser methods return `ParseResult` (success/failure); errors emit diagnostics via `Parser::emitError()`.
- SSA value uses before definition are tracked as forward references resolved at block/region end.
- Affine parsing is driven by `AffineParser` as a sub-parser instantiated from the main `Parser`.

## Dependencies

### Internal
- `mlir/lib/IR/` — all core IR types and `MLIRContext`
- `mlir/lib/Parser/` — entry-point wrapper

### External
- `llvm/lib/Support` — `llvm::SourceMgr`, `llvm::SMLoc`, ADT

<!-- MANUAL: -->
