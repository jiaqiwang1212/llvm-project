<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# AsmParser

## Purpose
Parses BPF assembly text into `MCInst` objects for the integrated assembler. Implements `MCTargetAsmParser` with BPF-specific syntax conventions: `=` is not an assignment operator (it is used in BPF asm for register assignment), and `*` at the start of a token signals a memory dereference rather than multiplication.

## Key Files

| File | Description |
|------|-------------|
| `BPFAsmParser.cpp` | Defines `BPFAsmParser : MCTargetAsmParser`; implements `parseInstruction`, `parseRegister`, `matchAndEmitInstruction`, and `PreMatchCheck`; `BPFOperand` models register, immediate, and memory operands |
| `CMakeLists.txt` | Registers this as the `BPFAsmParser` component |

## For AI Agents

### Working In This Directory

- `equalIsAsmAssignment()` returns `false` — do not change this; `=` is part of BPF asm instruction syntax.
- `tokenIsStartOfStatement()` returns `true` for `AsmToken::Star` — memory dereference operands start with `*` and are treated as the beginning of a new statement by the lexer.
- Most instruction matching is auto-generated via `BPFGenAsmMatcher.inc` (from `#include "BPFGenAsmMatcher.inc"` inside the class). Custom pre-match validation goes in `PreMatchCheck`.
- Adding a new instruction requires updating `BPFInstrInfo.td` with a correct `AsmString`; the parser will handle it automatically through the generated matcher.

### Testing Requirements

- Test with `llvm-mc -triple=bpfle -filetype=obj` on hand-written `.s` files.
- Parser round-trip: assemble then disassemble and compare.

### Common Patterns

- `BPFOperand` stores a union of `{Register, Imm, Token, Mem}` variants with `isReg()`, `isImm()`, `isMem()` predicates used by the generated matcher.

## Dependencies

### Internal
- `../MCTargetDesc/` — `BPFMCAsmInfo`, `BPFMCTargetDesc` (register enums, instruction info)
- `../TargetInfo/` — `getTheBPFleTarget()` / `getTheBPFbeTarget()` for registration

### External
- `llvm/MC/MCParser/MCTargetAsmParser.h` — base class
- `BPFGenAsmMatcher.inc` — TableGen-generated; do not edit directly

<!-- MANUAL: -->
