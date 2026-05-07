<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# RISCV Backend

## Purpose

The RISC-V backend implements code generation for RISC-V processors, a free and open-source instruction set architecture. It supports RISC-V 32-bit (RV32) and 64-bit (RV64) variants with modular ISA extensions (I, M, F, D, A, C, V, etc.). The backend is actively growing with comprehensive support for emerging extensions and optimizations for production processors.

## Key Files

| File | Description |
|------|-------------|
| `RISCVTargetMachine.cpp` | TargetMachine implementation, subtarget creation |
| `RISCVTargetMachine.h` | TargetMachine class definition |
| `RISCVISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `RISCVISelDAGToDAG.h` | Instruction selection class definition |
| `RISCVISelLowering.cpp` | Calling conventions, custom lowering, legalization |
| `RISCVISelLowering.h` | ISelLowering interface |
| `RISCVAsmPrinter.cpp` | Assembly printing, MC emission |
| `RISCVAsmPrinter.h` | AsmPrinter implementation |
| `RISCVInstrInfo.cpp` | Instruction properties and patterns |
| `RISCVInstrInfo.h` | InstrInfo interface |
| `RISCVRegisterInfo.cpp` | Register file, frame index elimination |
| `RISCVRegisterInfo.h` | RegisterInfo interface |
| `RISCVFrameLowering.cpp` | Prologue/epilogue, stack frame layout |
| `RISCVFrameLowering.h` | FrameLowering interface |
| `RISCV.td` | Top-level TableGen definition |
| `RISCVInstrInfo.td` | Instruction definitions (modular by extension) |
| `RISCVRegisterInfo.td` | Register definitions |
| `RISCVCallingConv.td` | Calling convention definitions (RISC-V ABI) |
| `RISCVFeatures.td` | Feature and extension definitions |
| `RISCVProfiles.td` | Profile definitions (IMAC, IMAFD, etc.) |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | RISC-V assembler parser for inline asm and .s files |
| `Disassembler/` | Binary-to-assembly disassembly for RISC-V instructions |
| `GISel/` | GlobalISel infrastructure and register bank definitions |
| `MCA/` | Machine Code Analyzer for performance modeling |
| `MCTargetDesc/` | MC layer (Machine Code) target descriptions |
| `TargetInfo/` | Target info registration and initialization |

## For AI Agents

### Working In This Directory

- **ISA Base + Modular Extensions**:
  - **RV32I/RV64I**: Base integer ISA (required)
  - **M**: Multiply/Divide extension
  - **A**: Atomic extension
  - **F/D**: Single/Double floating-point extensions
  - **C**: Compressed instruction extension (16-bit instructions)
  - **V**: Vector extension (scalable, variable-length vectors)
  - **B**: Bit manipulation extension
  - **H**, **S**, **U**: Hypervisor, Supervisor, User modes
  - Features controlled by predicates in `RISCVFeatures.td`; profiles in `RISCVProfiles.td` bundle common combinations (e.g., IMAC = I+M+A+C)

- **Instruction Definitions Structure**:
  - `RISCVInstrInfo.td`: Organizes instructions by extension; modular design allows easy addition of new extensions
  - Split into logical sections (Integer, Float, Vector, Atomic, Compressed, etc.)
  - Each extension can be independently enabled/disabled via subtarget features

- **Calling Convention**: RISC-V ABI defined in `RISCVCallingConv.td`; implemented in `RISCVISelLowering.cpp` (lowering) and `RISCVRegisterInfo.cpp` (register allocation).

- **GlobalISel Support**: RISC-V has growing GlobalISel infrastructure in `GISel/` for machine-independent instruction selection, particularly valuable as new extensions are added.

- **TableGen Changes**: After modifying `.td` files, the build system automatically regenerates `.inc` files via `llvm-tblgen`. No manual regeneration needed.

### Common Patterns

- **Immediate encoding**: RISC-V has 12-bit immediate encodings; larger immediates require LUI (load upper immediate) + secondary instruction. See `RISCVInstrInfo.td` for patterns.
- **Compressed instructions (RV32C/RV64C)**: 16-bit aliases for common operations; defined separately in `RISCVInstrInfo.td` with C_ prefix.
- **Atomic operations (RV32A/RV64A)**: LR/SC (load-reserved/store-conditional) pairs for lock-free synchronization; patterns in `RISCVInstrInfo.td`.
- **Vector operations (RV32V/RV64V)**: Scalable vectors with vl (vector length) and vtype CSRs; complex patterns in `RISCVInstrInfo.td` with vector-specific predicates.
- **Floating-point operations**: Separate register file for floats (F extension); calling convention handles float-in-GPR for soft-float ABIs.
- **Bit manipulation (RV32B/RV64B)**: Rotate, shift-and-add, bit-field operations; recent extension with patterns in `RISCVInstrInfo.td`.

## Dependencies

### Internal

- `lib/CodeGen/` — Target-independent instruction selection, register allocation, instruction scheduling
- `lib/CodeGen/GlobalISel/` — Machine-independent instruction selection framework (used by GISel/)
- `lib/MC/` — Machine Code layer for binary emission and assembler
- `lib/TargetParser/` — ISA parsing and version management
- `include/llvm/Target/` — Abstract TargetMachine, TargetInfo, TargetLowering interfaces

### External

- `llvm-tblgen` — TableGen compiler (invoked by build system to generate `.inc` files from `.td` definitions)

<!-- MANUAL: RISC-V backend exemplifies modular ISA extension design; use as reference when adding new instruction sets. The vector extension (V) is complex; study RISCVInstrInfo.td vector patterns and GISel/ support before implementing V-dependent optimizations. -->
