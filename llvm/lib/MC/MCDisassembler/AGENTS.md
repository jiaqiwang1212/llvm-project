<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/MC/MCDisassembler

## Purpose

Disassembler framework for converting machine code bytes back to assembly instructions. Provides the abstract MCDisassembler base class and the C API bindings (Disassembler.h/Disassembler.cpp) used by tools like llvm-objdump, debuggers, and instrumentation frameworks.

## Key Files

| File | Description |
|------|-------------|
| `MCDisassembler.cpp` | Abstract disassembler base class (target-independent interface) |
| `Disassembler.cpp` | C API bindings for disassembly (LLVMDisasm* functions) |
| `Disassembler.h` | C API header (public interface for non-C++ code) |
| `MCExternalSymbolizer.cpp` | External symbol resolution (callbacks for symbol lookup during disassembly) |
| `MCRelocationInfo.cpp` | Relocation information queries (for resolving jumps/calls) |
| `MCSymbolizer.cpp` | Symbol resolution during disassembly |
| `CMakeLists.txt` | MCDisassembler build configuration |

## For AI Agents

### Working In This Directory

The disassembler layer decodes binary machine code into human-readable instructions and symbolic references. When working here:

1. **Abstract base**: MCDisassembler is target-independent. Each target backend implements getInstruction() for its ISA.
2. **Opaque decoding**: Disassemble one instruction at a time from a byte stream. Return MCInst and number of bytes consumed.
3. **Symbol resolution**: External symbolizer callbacks allow tools to resolve addresses to symbol names. Implement these for accurate disassembly.
4. **Relocation tracking**: Understand which operands are relocations (need symbol lookup) vs. absolute/relative values.
5. **Error handling**: Invalid bytes should return an error code. Some disassemblers skip invalid bytes; others halt.
6. **C API compatibility**: Disassembler.h must work from C code. Don't break the C API — it's public.

### Common Patterns

- **Disassembly loop**: Get instruction with getInstruction(), print with MCInstPrinter, advance buffer pointer.
- **External symbolizer**: Implement SymbolLookupCallback to resolve branch targets to names.
- **Relocation info**: Query MCRelocationInfo to understand which operands are relocations.
- **Error reporting**: Return error codes from getInstruction() on invalid input.
- **Context passing**: MCDisassembler holds MCContext for consistency with assembler.

## Dependencies

### Internal
- `llvm/MC/MCInst.h` — Instruction representation
- `llvm/MC/MCContext.h` — Global MC context
- `llvm/MC/MCDisassembler.h` — Abstract base class
- `llvm/Support/` — Utilities
- Target backends (llvm/Target/) — Concrete MCDisassembler implementations

### External
- None (self-contained disassembler framework)

<!-- MANUAL: -->
