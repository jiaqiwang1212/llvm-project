<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AsmParser

## Purpose

The AsmParser directory contains the LLVM IR text format parser. It parses `.ll` files and text-based IR into LLVM Module objects. The parser handles the full LLVM IR text syntax including functions, basic blocks, instructions, metadata, attributes, and inline assembly.

## Key Files

| File | Description |
|------|-------------|
| `Parser.h` | Main parser interface with `parseAssemblyFile()` and `parseAssemblyString()` entry points |
| `LLParser.h` | Core recursive descent parser implementation |
| `LLLexer.h` | Lexical analyzer (tokenizer) for LLVM IR text |
| `LLToken.h` | Token type definitions for the lexer |
| `AsmParserContext.h` | Parser context holding state during parsing |
| `SlotMapping.h` | Slot number to value mapping for value numbering |
| `NumberedValues.h` | Tracking of numbered values during parsing (e.g., `%0`, `%1`) |
| `FileLoc.h` | Source location tracking for error reporting |

## Subdirectories (if applicable)

No subdirectories; all parser components are headers in this directory.

## For AI Agents

### Working In This Directory

When using or extending the assembly parser:

1. **Use the Parser.h interface** — Call `parseAssemblyFile()` or `parseAssemblyString()` to parse IR text
2. **Error handling** — Parser returns `SMDiagnostic` for errors; check for null Module before using result
3. **Context is required** — Provide an `LLVMContext` to receive parsed globals and metadata
4. **Slot mapping is optional** — Pass `SlotMapping*` if you need mapping of numbered values to Value pointers
5. **Source location tracking** — Parser preserves source locations; use `FileLoc` for error messages

### Common Patterns

- **Parse file or string** — Both entry points work the same; choose based on input source
- **Error reporting** — `SMDiagnostic` includes line number, column, and error message
- **Value numbering** — Values are numbered sequentially (`%0`, `%1`); the parser tracks these in `SlotMapping`
- **Lazy parsing** — Parser fully parses structure and metadata; function bodies are complete
- **Metadata attachment** — Metadata is attached to instructions and globals during parsing

## Dependencies

### Internal

- `llvm/IR/` — IR classes (Module, Function, BasicBlock, Instruction, Value, Type, Metadata)
- `llvm/Support/` — MemoryBuffer, raw_ostream, SourceMgr (source location tracking), error handling
- `llvm/ADT/` — Data structures (DenseMap, SmallVector, StringRef, etc.)

### External

- Input is expected to be valid LLVM IR text format; no external dependencies

<!-- MANUAL: -->
