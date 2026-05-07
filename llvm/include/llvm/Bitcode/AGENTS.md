<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bitcode

## Purpose

LLVM bitcode (.bc) file format reader and writer. Serializes and deserializes LLVM IR (functions, basic blocks, instructions, metadata, type info) to/from compact binary representation. Core to LTO, offline compilation, and IR distribution.

## Key Files

| File | Description |
|------|-------------|
| `BitcodeReader.h` | Bitcode deserializer; lazy-loads IR modules and function bodies (13 KB) |
| `BitcodeWriter.h` | Bitcode serializer; writes IR to .bc format (7 KB) |
| `BitcodeWriterPass.h` | Pass wrapper for bitcode writing (2 KB) |
| `BitcodeAnalyzer.h` | Bitcode format analyzer; inspects structure and metadata (3 KB) |
| `BitcodeCommon.h` | Shared types (error codes, module info) (1 KB) |
| `BitcodeConvenience.h` | Convenience functions for bitcode I/O (18 KB) |
| `LLVMBitCodes.h` | Bitcode opcode and record format definitions (34 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding bitcode format structure (opcodes, record codes, type encoding)
- Analyzing IR serialization strategies
- Reviewing format versioning and compatibility

**Implementation tasks:**
- Adding support for new IR constructs (new instruction types, metadata kinds)
- Optimizing bitcode size or read performance
- Implementing bitcode versioning for IR evolution

### Common Patterns

1. **Lazy parsing**: BitcodeReader defers function body parsing until requested (streaming model)
2. **Record codes**: LLVMBitCodes.h defines all record types; readers interpret via opcode dispatch
3. **Type encoding**: Complex LLVM types (function, struct, vector) encoded as type indices
4. **Function abbreviations**: Repeating patterns (e.g., instruction sequences) use abbreviations to reduce size
5. **Metadata blocks**: Separate block for non-structural metadata (debug info, annotations)

### Important Notes

- **Opcode stability**: Bitcode opcodes must remain stable across LLVM versions; new constructs use reserved opcodes
- **Version negotiation**: BitcodeReader detects version and adapts parsing (backward compatibility critical)
- **Streaming capable**: Designed for streaming readers; enables incremental linking and just-in-time compilation
- **LTO dependency**: ThinLTO and FullLTO rely on bitcode; changes here impact linker performance

## Dependencies

### Internal
- `llvm/IR/` — Module, Function, Instruction, Type, Metadata
- `llvm/Bitstream/` — Low-level bitstream I/O (BitstreamReader, BitstreamWriter)
- `llvm/Support/` — Error, Endianness, DataExtractor

### External
- None

<!-- MANUAL: -->
