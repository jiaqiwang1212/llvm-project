<!-- Generated: 2026-05-07 -->

# Bitcode — LLVM Bitcode I/O

## Purpose

Provides reading and writing of LLVM's compact binary format (.bc files). Bitcode is a serialized form of IR, allowing efficient storage and transmission of modules. This directory contains the top-level infrastructure; actual encoding/decoding logic is in `Reader/` and `Writer/` subdirectories.

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `Reader/` | Bitcode deserialization (read .bc → IR) — see `Reader/AGENTS.md` |
| `Writer/` | Bitcode serialization (IR → write .bc) — see `Writer/AGENTS.md` |

## For AI Agents

### Working In This Directory

1. **Two-stage process**: Bitcode is split between Reader (deserialization) and Writer (serialization). Changes to the format must be coordinated between both.

2. **Backward compatibility**: LLVM bitcode must be forward and backward compatible. Older LLVM versions should read newer .bc files (gracefully) and newer versions should read older files.

3. **Format versioning**: Bitcode includes version information. Check `Reader/` and `Writer/` for version handling.

4. **IR mapping**: Understand how IR objects (Module, Function, BasicBlock, Instruction) map to bitcode records. This is in the reader/writer implementations.

5. **Performance**: Bitcode is used for link-time optimization (LTO). Read/write performance is critical.

## Dependencies

### Internal

- **llvm/lib/IR/** — IR objects being serialized/deserialized
- **llvm/lib/Bitstream/Reader/** — Low-level bitstream I/O (see Bitstream/AGENTS.md)
- **llvm/lib/Support/** — String and data utilities

### External

None.

### Dependents

- **llvm/lib/Bitcode/Reader/** — Bitcode deserialization
- **llvm/lib/Bitcode/Writer/** — Bitcode serialization
- **llvm/tools/llvm-dis** — Disassemble .bc to .ll
- **llvm/tools/llvm-as** — Assemble .ll to .bc
- **Link-time optimization (LTO)** — Uses bitcode for lazy loading

## Notes for Developers

- **See subdirectories**: Most of the implementation is in `Reader/` and `Writer/`. Start there.
- **Bitstream format**: Bitcode is built on top of the bitstream format. Understand bitstream records before diving into bitcode.

<!-- MANUAL: -->
