<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# MCTargetDesc

## Purpose
MC-layer (Machine Code layer) target description for BPF. Unlike many LLVM backends, BPF places its instruction printer (`BPFInstPrinter`) here rather than in a separate `InstPrinter/` directory. This directory provides all components needed for binary emission: code emitter, asm backend, ELF object writer, fixup kinds, assembler info, and the inst printer.

## Key Files

| File | Description |
|------|-------------|
| `BPFMCTargetDesc.h/.cpp` | Exports register/instruction/subtarget enums; registers MC components (code emitters, asm backends, inst printer, disassembler) for all three triples via `LLVMInitializeBPFTargetMC()` |
| `BPFInstPrinter.h/.cpp` | `BPFInstPrinter : MCInstPrinter`; prints registers (`r0`–`r10`), immediates, memory operands (`*(u64*)(r1 + 8)`), branch targets, and 64-bit immediates; `getMnemonic`/`printInstruction`/`getRegisterName` are TableGen-generated |
| `BPFMCAsmInfo.h` | `BPFMCAsmInfo` — comment character (`#`), directive style, ELF flags for BPF |
| `BPFMCCodeEmitter.cpp` | Emits little-endian BPF instruction bytes; `createBPFMCCodeEmitter` (LE) and `createBPFbeMCCodeEmitter` (BE) |
| `BPFAsmBackend.cpp` | `BPFAsmBackend` — handles relaxation and fixup application; `createBPFAsmBackend` (LE) and `createBPFbeAsmBackend` (BE) |
| `BPFELFObjectWriter.cpp` | Generates ELF relocations via `createBPFELFObjectWriter` |
| `BPFMCFixups.h` | Defines `BPF::FK_BPF_PCRel_4` — the single BPF-specific fixup kind (4-byte PC-relative for jump offsets) |
| `CMakeLists.txt` | Outputs the `BPFDesc` library |

## For AI Agents

### Working In This Directory

- **No CodeGen dependencies**: must only include `llvm/MC/`, `llvm/Support/`, `llvm/ADT/`. Importing `llvm/CodeGen/` breaks the layering and causes link failures in standalone MC tools.
- **InstPrinter is here**: unlike most backends, BPF's inst printer lives in `MCTargetDesc/` — not in a separate `InstPrinter/`. When modifying printed operand syntax, edit `BPFInstPrinter.cpp`.
- **Endianness variants**: LE and BE code emitters/backends are separate factory functions but share most logic. When fixing an emission bug, check both paths.
- **Fixups**: `FK_BPF_PCRel_4` encodes the jump offset in the 16-bit `offset` field of the BPF instruction. The offset is in units of instructions (not bytes), so the backend divides byte offsets by 8.
- `BPFMCCodeEmitter` accesses the `.imm` field of wide (16-byte) `BPF_LD | BPF_IMM | BPF_DW` instructions specially.

### Testing Requirements

- `llvm-mc -triple=bpfle -show-encoding` to inspect emitted bytes.
- `llvm-mc -triple=bpfle -filetype=obj` then `llvm-readelf -r` to inspect relocations.

### Common Patterns

- MC component registration: `RegisterMCAsmInfo<BPFMCAsmInfo> X(T)` style inside `LLVMInitializeBPFTargetMC`.
- `BPFGenInstrInfo.inc`, `BPFGenRegisterInfo.inc`, `BPFGenSubtargetInfo.inc` are pulled in by `BPFMCTargetDesc.h` for enum exports.

## Dependencies

### Internal
- `../TargetInfo/` — `getTheBPF{,le,be}Target()` for MC registration

### External
- `llvm/MC/MCAsmInfo.h`, `llvm/MC/MCCodeEmitter.h`, `llvm/MC/MCAsmBackend.h`
- `BPFGenAsmWriter.inc` — TableGen-generated inst printer; do not edit directly
- `BPFGenInstrInfo.inc`, `BPFGenRegisterInfo.inc`, `BPFGenSubtargetInfo.inc` — TableGen-generated enums

<!-- MANUAL: -->
