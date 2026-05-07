<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Mips Backend

## Purpose

The MIPS backend implements code generation for MIPS processors (32-bit and 64-bit), including support for MIPS ISA variants (MIPS I/II/III/IV/V, MIPS32/MIPS64), microMIPS, and extensions like DSP, MSA, and EVO. This backend serves as the foundation for Cpu0 and similar pedagogical backends.

## Key Files

| File | Description |
|------|-------------|
| `MipsTargetMachine.cpp` | TargetMachine implementation, subtarget creation |
| `MipsTargetMachine.h` | TargetMachine class definition |
| `MipsISelDAGToDAG.cpp` | DAG-to-DAG instruction selection (main ISel) |
| `MipsISelDAGToDAG.h` | Instruction selection class definition |
| `MipsISelLowering.cpp` | Calling conventions, custom lowering, legalization |
| `MipsISelLowering.h` | ISelLowering interface |
| `MipsAsmPrinter.cpp` | Assembly printing, MC emission |
| `MipsAsmPrinter.h` | AsmPrinter implementation |
| `MipsInstrInfo.cpp` | Instruction properties and patterns |
| `MipsInstrInfo.h` | InstrInfo interface |
| `MipsRegisterInfo.cpp` | Register file, frame index elimination |
| `MipsRegisterInfo.h` | RegisterInfo interface |
| `MipsFrameLowering.cpp` | Prologue/epilogue, stack frame layout |
| `MipsFrameLowering.h` | FrameLowering interface |
| `Mips.td` | Top-level TableGen definition |
| `MipsInstrInfo.td` | Instruction definitions (generic MIPS) |
| `MipsRegisterInfo.td` | Register definitions |
| `MipsCallingConv.td` | Calling convention definitions |
| `Mips16InstrInfo.cpp` | MIPS-16 (16-bit compressed ISA) instruction info |
| `Mips16ISelDAGToDAG.cpp` | MIPS-16 instruction selection |
| `Mips16ISelLowering.cpp` | MIPS-16 lowering |
| `MipsSEInstrInfo.cpp` | MIPS Standard Encoding instruction info |
| `MipsSEISelDAGToDAG.cpp` | MIPS Standard Encoding instruction selection |
| `MipsSEISelLowering.cpp` | MIPS Standard Encoding lowering |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | MIPS assembler parser for inline asm and .s files |
| `Disassembler/` | Binary-to-assembly disassembly for MIPS instructions |
| `MCTargetDesc/` | MC layer (Machine Code) target descriptions |
| `TargetInfo/` | Target info registration and initialization |

## For AI Agents

### Working In This Directory

- **Variants**: MIPS backend supports three main ISA variants:
  - **Standard Encoding (SE)**: Full MIPS instructions (MipsSE*)
  - **MIPS-16**: 16-bit compressed encoding (Mips16*)
  - **microMIPS**: Compact variant (MicroMips*)
  
- **Extensions**: Instruction definitions split by extension:
  - `MipsInstrInfo.td`: Base MIPS instructions
  - `Mips32r6InstrInfo.td`, `Mips64r6InstrInfo.td`: Release 6 instructions
  - `MipsMSAInstrInfo.td`: MIPS SIMD Architecture
  - `MipsDSPInstrInfo.td`: Digital Signal Processing extension
  - `MipsMTInstrInfo.td`: Multi-threading extension

- **TableGen Changes**: After modifying `.td` files, the build system automatically regenerates `.inc` files via `llvm-tblgen`. No manual regeneration needed.

- **Calling Conventions**: Defined in `MipsCallingConv.td` and implemented across `MipsISelLowering.cpp` (for lowering) and `MipsRegisterInfo.cpp` (for register allocation).

### Common Patterns

- **Pre-index vs Post-index addressing**: MIPS supports addressing modes that load/store and update the base register simultaneously; see `MipsInstrInfo.td` for patterns.
- **Delay slots**: MIPS has instruction delay slots after branches and loads; the backend inserts NOPs or reorders instructions to fill them.
- **Conditional execution**: MIPS uses branch/condition codes, not predication; instruction selection reflects this in branch generation.
- **Floating-point ABI**: Separate lowering paths for hard-float (FPU instructions) vs soft-float (library calls) in `MipsISelLowering.cpp`.

## Dependencies

### Internal

- `lib/CodeGen/` — Target-independent instruction selection, register allocation, instruction scheduling
- `lib/MC/` — Machine Code layer for binary emission and assembler
- `include/llvm/Target/` — Abstract TargetMachine, TargetInfo, TargetLowering interfaces

### External

- `llvm-tblgen` — TableGen compiler (invoked by build system to generate `.inc` files from `.td` definitions)

<!-- MANUAL: Cpu0 backend is based on MIPS backend structure; refer to this for precedent in multi-ISA-variant design. -->
