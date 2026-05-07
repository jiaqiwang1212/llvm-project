<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AArch64 Backend

## Purpose

The AArch64 backend implements code generation for ARMv8-A and later 64-bit ARM architectures. It supports the full ARMv8 ISA, Scalable Vector Extensions (SVE/SVE2), Scalable Matrix Extensions (SME), and numerous microarchitecture-specific optimizations for processors from Cortex-A series and others.

## Key Files

| File | Description |
|------|-------------|
| `AArch64TargetMachine.cpp` | TargetMachine implementation, subtarget creation |
| `AArch64TargetMachine.h` | TargetMachine class definition |
| `AArch64ISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `AArch64ISelDAGToDAG.h` | Instruction selection class definition |
| `AArch64ISelLowering.cpp` | Calling conventions, custom lowering, legalization |
| `AArch64ISelLowering.h` | ISelLowering interface |
| `AArch64AsmPrinter.cpp` | Assembly printing, MC emission |
| `AArch64AsmPrinter.h` | AsmPrinter implementation |
| `AArch64InstrInfo.cpp` | Instruction properties and patterns |
| `AArch64InstrInfo.h` | InstrInfo interface |
| `AArch64RegisterInfo.cpp` | Register file, frame index elimination |
| `AArch64RegisterInfo.h` | RegisterInfo interface |
| `AArch64FrameLowering.cpp` | Prologue/epilogue, stack frame layout |
| `AArch64FrameLowering.h` | FrameLowering interface |
| `AArch64.td` | Top-level TableGen definition |
| `AArch64InstrInfo.td` | Instruction definitions |
| `AArch64RegisterInfo.td` | Register definitions |
| `AArch64CallingConvention.td` | ARM EABI calling conventions |
| `AArch64Features.td` | Feature and predicate definitions |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | AArch64 assembler parser for inline asm and .s files |
| `Disassembler/` | Binary-to-assembly disassembly for AArch64 instructions |
| `GISel/` | GlobalISel infrastructure (machine-independent instruction selection) |
| `MCTargetDesc/` | MC layer (Machine Code) target descriptions |
| `TargetInfo/` | Target info registration and initialization |
| `Utils/` | Utility classes (e.g., condition code predicates, instruction utilities) |

## For AI Agents

### Working In This Directory

- **ISA Variants**:
  - **ARMv8-A Base**: Integer and FP instructions (AArch64InstrInfo.td)
  - **SVE (Scalable Vector Extension)**: Variable-length vectors up to 2048 bits
  - **SME (Scalable Matrix Extension)**: Matrix operations on scalable registers
  - Predicates in `AArch64Features.td` control availability of each extension

- **Instruction Selection Paths**:
  - **DAG-to-DAG (ISelDAGToDAG.cpp)**: Traditional pattern matching on DAG nodes
  - **GlobalISel (GISel/)**: Machine-independent selection for better code quality and maintainability

- **Calling Conventions**: ARM EABI defined in `AArch64CallingConvention.td`; implemented in `AArch64ISelLowering.cpp` (lowering) and `AArch64RegisterInfo.cpp` (register allocation).

- **TableGen Changes**: After modifying `.td` files, the build system automatically regenerates `.inc` files via `llvm-tblgen`. No manual regeneration needed.

### Common Patterns

- **Conditional execution**: AArch64 uses conditional branches and conditional selects (CSEL), not predicated instructions; see `AArch64InstrInfo.td` for CSEL patterns.
- **Load-pair/Store-pair instructions**: LDP/STP combine two loads or stores with a single addressing mode; handled by peephole patterns.
- **Addressing modes**: Immediate offset, post-index, pre-index, and register offset modes; patterns in `AArch64InstrInfo.td`.
- **SVE predicates**: Each SVE instruction has an associated predicate register; GISel handles predicate logic more cleanly than DAG ISel.

## Dependencies

### Internal

- `lib/CodeGen/` — Target-independent instruction selection, register allocation, instruction scheduling
- `lib/CodeGen/GlobalISel/` — Machine-independent instruction selection framework (used by GISel/)
- `lib/MC/` — Machine Code layer for binary emission and assembler
- `include/llvm/Target/` — Abstract TargetMachine, TargetInfo, TargetLowering interfaces

### External

- `llvm-tblgen` — TableGen compiler (invoked by build system to generate `.inc` files from `.td` definitions)

<!-- MANUAL: AArch64 is one of the most mature and heavily optimized backends; use as reference for best practices in large-scale ISA support. -->
