<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MC/MCParser

## Purpose

Assembly language parser — lexer, parser, and assembler for LLVM's assembly syntax and target-specific assembly dialects. Converts `.s` (assembly text) files into MCInst representations for emission or further processing.

## Key Files

| File | Description |
|------|-------------|
| `AsmLexer.cpp` | Assembly lexer (tokenizes .s input) |
| `AsmParser.cpp` | Main LLVM assembly parser (directives, labels, instructions, expressions) |
| `MCAsmParser.cpp` | Abstract assembly parser interface |
| `MCAsmParserExtension.cpp` | Extension mechanism for target-specific parsing |
| `MCTargetAsmParser.cpp` | Abstract target-specific assembly parser base |
| `DarwinAsmParser.cpp` | Darwin/Mach-O assembly dialect (`.section`, `.globl`, etc.) |
| `ELFAsmParser.cpp` | ELF assembly dialect (`.type`, `.size`, `.globl`, etc.) |
| `COFFAsmParser.cpp` | COFF assembly dialect (Windows) |
| `COFFMasmParser.cpp` | MASM (Microsoft Macro Assembler) dialect |
| `XCOFFAsmParser.cpp` | XCOFF assembly dialect (z/OS) |
| `WasmAsmParser.cpp` | WebAssembly assembly dialect |
| `GOFFAsmParser.cpp` | GOFF (Generalized Object File Format) assembly dialect |
| `LFIAsmParser.cpp` | Local Function Identifier assembly support |
| `MasmParser.cpp` | MASM-specific parsing extensions |
| `CMakeLists.txt` | MCParser build configuration |

## For AI Agents

### Working In This Directory

The parser layer converts human-readable assembly into machine code representations. When working here:

1. **Lexer/Parser separation**: AsmLexer tokenizes, AsmParser processes tokens. Keep lexer simple, complex logic in parser.
2. **Directive handling**: Directives like `.section`, `.globl`, `.align` control code emission, not instructions. Handle them specially.
3. **Target extensions**: MCTargetAsmParser subclasses allow targets to add custom syntax (e.g., Intel vs AT&T). Respect the extension mechanism.
4. **Expression parsing**: Assembly expressions (arithmetic, symbols, relocations) parse similarly across dialects. Share logic.
5. **Error recovery**: Parser should report errors with line numbers and context. Avoid cascading errors — stop after first error or recover gracefully.
6. **Dialect handling**: ELF, COFF, Mach-O, Wasm have different directives and syntax. Keep dialect-specific code in respective parsers.

### Common Patterns

- **Token stream**: Lexer produces tokens. Parser consumes them with `Lex()` and `Peek()`.
- **Directive dispatch**: Parse `.` directives, dispatch to target-specific handlers via MCTargetAsmParser.
- **Instruction parsing**: Parse mnemonic + operands. Delegate to target MCTargetAsmParser for encoding.
- **Expression parsing**: Recursive descent or precedence-climbing for arithmetic/symbol expressions.
- **Label definitions**: Track labels, emit MCLabel to streamer.
- **Error reporting**: Use `Error()` or `Warning()` with current token location.

## Dependencies

### Internal
- `llvm/MC/AsmLexer.h` — Lexer interface
- `llvm/MC/MCAsmParser.h` — Parser interface
- `llvm/MC/MCContext.h` — Global state
- `llvm/MC/MCStreamer.h` — Code emission
- `llvm/MC/MCTargetAsmParser.h` — Target extension mechanism
- `llvm/Support/` — Utilities
- Target backends (llvm/Target/) — MCTargetAsmParser implementations

### External
- Standard C++ library

<!-- MANUAL: -->
