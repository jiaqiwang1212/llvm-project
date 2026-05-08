<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/AsmParser/

## Purpose
Low-level assembly parser interface headers. Provides the `AsmParser` class used by op `parse()` methods to parse custom assembly syntax, the `AsmParserState` for tracking source locations of parsed entities (used by LSP servers), and the `CodeCompleteContext` for IDE code completion support.

## Key Files
| File | Description |
|------|-------------|
| `AsmParser.h` | `AsmParser` base class with helpers for parsing types, attributes, operands, keywords, punctuation |
| `AsmParserState.h` | `AsmParserState` — maps parsed SSA values/types/ops back to source ranges |
| `CodeComplete.h` | `AsmParserCodeCompleteContext` — provides completions during partial parses |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `AsmParser` is the type used in `Op::parse(AsmParser &parser, OperationState &state)` implementations.
- `OpAsmParser` (in `mlir/IR/OpImplementation.h`) extends `AsmParser` with op-specific methods.
- `AsmParserState` is populated during parse and consumed by the MLIR LSP server for go-to-definition.
- Do not confuse with `mlir/Parser/Parser.h` — that is the high-level file/string entry point.

### Common Patterns
- `parser.parseKeyword("keyword")`, `parser.parseType(type)`, `parser.parseOperand(operand)`
- `parser.emitError(loc, "message")` to report parse errors
- Custom assembly format can be declarative (via `assemblyFormat` in `.td`) or manual via `parse()`/`print()`.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, Type, Attribute, Location)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/SourceMgr.h`

<!-- MANUAL: -->
