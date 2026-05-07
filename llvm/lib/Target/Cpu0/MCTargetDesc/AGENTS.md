<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-04-23 | Updated: 2026-05-07 -->

# MCTargetDesc

## Purpose
MC-layer (Machine Code layer) target description for Cpu0. Exposes register and instruction enumerations to the rest of the backend, defines the ABI calling-convention variants, and provides assembler metadata. This layer sits below CodeGen and must not depend on `SelectionDAG` or `MachineFunction`.

## Key Files

| File | Description |
|------|-------------|
| `Cpu0MCTargetDesc.h` | Pulls in `Cpu0GenRegisterInfo.inc`, `Cpu0GenInstrInfo.inc`, and `Cpu0GenSubtargetInfo.inc` to expose register/instruction/subtarget enums to the whole backend |
| `Cpu0MCTargetDesc.cpp` | Registers MC components (assembler, disassembler, inst printer, streamer) with the LLVM `TargetRegistry` |
| `Cpu0ABIInfo.h/.cpp` | `Cpu0ABIInfo` — holds the active ABI variant (`O32` or `S32`), provides `computeTargetABI()`, and maps calling-convention IDs to register lists |
| `Cpu0BaseInfo.h` | Instruction encoding constants and field masks shared across TableGen and C++ code |
| `Cpu0MCAsmInfo.h/.cpp` | `Cpu0MCAsmInfo` — configures the assembler dialect (comment chars, directives, ELF flags) for Cpu0 assembly syntax |
| `CMakeLists.txt` | Build rules; outputs the `Cpu0Desc` library |

## For AI Agents

### Working In This Directory

- **No CodeGen dependencies**: files here must only include `llvm/MC/`, `llvm/Support/`, and `llvm/ADT/` headers. Importing `llvm/CodeGen/` breaks the layering and will cause link errors in standalone MC tools.
- **ABI variants**: `Cpu0ABIInfo::O32()` is the standard 32-bit ABI; `S32` is a tutorial-specific simplified variant. `computeTargetABI()` reads the triple/CPU string to pick one.
- **Register/instruction enums** are generated into `Cpu0GenRegisterInfo.inc` and `Cpu0GenInstrInfo.inc`. `Cpu0MCTargetDesc.h` is the canonical include for these enums; do not re-include the `.inc` files elsewhere.

### Testing Requirements

- Changes to `Cpu0MCAsmInfo` affect all assembly output — verify with `llvm-mc -triple=cpu0el`.
- ABI changes require updating calling-convention tests in the tutorial test suite.

### Common Patterns

- MC component registration follows the pattern: `RegisterMCAsmInfo<Cpu0MCAsmInfo> X(T)` inside `LLVMInitializeCpu0TargetMC()`.
- `Cpu0BaseInfo.h` constants use bitmask + shift pairs (e.g. `Cpu0II::MO_ABS_HI`) consumed by both `Cpu0MCInstLower` and TableGen encoding classes.

## Dependencies

### Internal
- `../TargetInfo/` — `getTheCpu0Target()` / `getTheCpu0elTarget()` used during registration

### External
- `llvm/MC/MCAsmInfo.h`, `llvm/MC/MCTargetDescription.h`
- `Cpu0GenRegisterInfo.inc`, `Cpu0GenInstrInfo.inc`, `Cpu0GenSubtargetInfo.inc` (TableGen-generated; do not edit)

<!-- MANUAL: -->
