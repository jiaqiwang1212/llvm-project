<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-04-23 -->

# Disassembler

## Purpose
Decodes BPF binary instruction bytes into `MCInst` objects for `llvm-objdump` and other disassembly consumers. Implements the BPF instruction encoding explicitly using the BPF instruction class/size/mode bit-field layout, rather than relying solely on TableGen-generated decoders, because BPF's encoding uses a compact 8-bit opcode field with overlapping sub-fields.

## Key Files

| File | Description |
|------|-------------|
| `BPFDisassembler.cpp` | Defines `BPFDisassembler : MCDisassembler`; decodes the 8-byte BPF instruction word using `BPF_CLASS`, `BPF_SIZE`, and `BPF_MODE` enums; handles wide (16-byte) instructions for 64-bit immediates |
| `CMakeLists.txt` | Registers this as the `BPFDisassembler` component |

## For AI Agents

### Working In This Directory

- BPF instructions are always 8 bytes (`struct bpf_insn`), except `BPF_LD | BPF_IMM | BPF_DW` which is 16 bytes (two consecutive instruction words). The disassembler must handle this wide-instruction case.
- `BPF_CLASS` (bits 2:0 of opcode), `BPF_SIZE` (bits 4:3), and `BPF_MODE` (bits 7:5) are the primary opcode sub-fields. New instructions added to the ISA must map to these fields correctly.
- Register decode uses `BPFGenDisassemblerTables.inc` (TableGen-generated); raw field extraction is manual.
- Endianness: the `BPFbeDisassembler` variant handles big-endian byte order; both share the same decode logic.

### Testing Requirements

- Test with `llvm-objdump -d --triple=bpfle` on compiled BPF object files.
- `llvm/test/MC/Disassembler/BPF/` holds binary decode tests.

### Common Patterns

- `getInstruction()` returns `MCDisassembler::Success`, `Fail`, or `SoftFail`.
- Use `decodeInstruction()` from `BPFGenDisassemblerTables.inc` for the common path; fall through to manual decoding for instructions not captured by TableGen.

## Dependencies

### Internal
- `../MCTargetDesc/` — register enums, `BPFMCTargetDesc`
- `../TargetInfo/` — target registration

### External
- `llvm/MC/MCDisassembler/MCDisassembler.h` — base class
- `BPFGenDisassemblerTables.inc` — TableGen-generated decode tables; do not edit directly

<!-- MANUAL: -->
