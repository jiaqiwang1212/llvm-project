<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRReader

## Purpose

The IRReader directory provides unified IR reading functions that automatically detect and parse LLVM IR in either bitcode or text format. The reader determines the input format and delegates to the appropriate parser (AsmParser or BitcodeReader).

## Key Files

| File | Description |
|------|-------------|
| `IRReader.h` | Main unified reader interface with auto-detection of bitcode vs. text format |

## Subdirectories (if applicable)

No subdirectories; single header file contains all reader definitions.

## For AI Agents

### Working In This Directory

When reading LLVM IR files:

1. **Use IRReader for mixed input** — If you don't know whether input is bitcode or text, use `getIRFileModule()` or `getLazyIRFileModule()`
2. **Lazy vs. eager loading** — Use `getLazyIR*` for large modules to defer function body deserialization; use `getIR*` for complete modules
3. **Lazy metadata loading** — Set `ShouldLazyLoadMetadata = true` to defer metadata deserialization
4. **MemoryBuffer or filename** — Both `MemoryBuffer` and file path interfaces are provided
5. **Error handling** — Check return value and `SMDiagnostic` for parse errors

### Common Patterns

- **Format auto-detection** — IRReader checks first bytes to determine bitcode vs. text; no manual format specification needed
- **Lazy deserialization** — Bitcode reader supports lazy loading; text parser is always eager
- **Error recovery** — On parse error, error message indicates format and location
- **Context ownership** — Caller provides `LLVMContext`; Module ownership is transferred to caller
- **Pipeline integration** — IRReader is typical entry point for `opt`, `llc`, and analysis tools

## Dependencies

### Internal

- `llvm/AsmParser/` — Text format parsing (Parser.h, AsmParserContext.h)
- `llvm/Bitcode/` — Bitcode format reading (BitcodeReader.h)
- `llvm/IR/` — IR classes (Module, LLVMContext)
- `llvm/Support/` — MemoryBuffer, raw_ostream, error handling
- `llvm/ADT/` — Data structures

### External

- Input file must be valid LLVM IR (either bitcode or text format)

<!-- MANUAL: -->
