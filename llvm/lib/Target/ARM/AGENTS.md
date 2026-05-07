<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ARM Backend

## Purpose

The ARM backend implements code generation for 32-bit ARM processors, supporting ARM, Thumb, and Thumb-2 instruction sets. It covers a wide range of ARM architectures from ARMv4 through ARMv8-32, with special handling for conditional execution, variable instruction encoding, and both software and hardware floating-point ABIs.

## Key Files

| File | Description |
|------|-------------|
| `ARMTargetMachine.cpp` | TargetMachine implementation, subtarget creation |
| `ARMTargetMachine.h` | TargetMachine class definition |
| `ARMISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `ARMISelDAGToDAG.h` | Instruction selection class definition |
| `ARMISelLowering.cpp` | Calling conventions, custom lowering, legalization |
| `ARMISelLowering.h` | ISelLowering interface |
| `ARMAsmPrinter.cpp` | Assembly printing, MC emission |
| `ARMAsmPrinter.h` | AsmPrinter implementation |
| `ARMInstrInfo.cpp` | Instruction properties and patterns |
| `ARMInstrInfo.h` | InstrInfo interface |
| `ARMRegisterInfo.cpp` | Register file, frame index elimination |
| `ARMRegisterInfo.h` | RegisterInfo interface |
| `ARMFrameLowering.cpp` | Prologue/epilogue, stack frame layout |
| `ARMFrameLowering.h` | FrameLowering interface |
| `ARM.td` | Top-level TableGen definition |
| `ARMInstrInfo.td` | Instruction definitions (base ARM, Thumb, Thumb-2) |
| `ARMRegisterInfo.td` | Register definitions |
| `ARMCallingConv.td` | EABI and other calling convention definitions |
| `ARMFeatures.td` | Feature and predicate definitions |
| `ARMArchitectures.td` | ARM architecture version definitions |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | ARM assembler parser for inline asm and .s files |
| `Disassembler/` | Binary-to-assembly disassembly for ARM instructions |
| `MCTargetDesc/` | MC layer (Machine Code) target descriptions |
| `TargetInfo/` | Target info registration and initialization |
| `Utils/` | Utility classes and predicates |

## For AI Agents

### Working In This Directory

- **Instruction Set Variants**:
  - **ARM mode**: Full 32-bit instruction set with conditional execution
  - **Thumb mode**: 16-bit compressed instructions (subset of ARM)
  - **Thumb-2 mode**: 16/32-bit mixed instructions with wider capabilities than original Thumb
  - Mode selection controlled by subtarget features and function attributes

- **Conditional Execution**: ARM instructions have condition codes (AL, EQ, NE, etc.) that execute conditionally without branching. This is different from other ISAs and requires special handling in ISel and FrameLowering.
  - See `ARMInstrInfo.td` for conditional instruction definitions
  - Predication logic in `ARMISelDAGToDAG.cpp`

- **Instruction Encoding Complexity**: Different modes (ARM vs Thumb-2) encode the same semantics differently:
  - `ARMInstrInfo.td` contains mode-specific instruction patterns
  - `ARMAsmPrinter.cpp` selects the correct encoding at code emission time

- **TableGen Changes**: After modifying `.td` files, the build system automatically regenerates `.inc` files via `llvm-tblgen`. No manual regeneration needed.

- **Calling Conventions**: ARM EABI defined in `ARMCallingConv.td`; implemented in `ARMISelLowering.cpp` (lowering) and `ARMRegisterInfo.cpp` (register allocation).

### Common Patterns

- **Conditional moves (MOVCC)**: Used extensively to avoid branches; patterns in `ARMInstrInfo.td`.
- **Load/Store multiple**: LDM/STM instructions load/store multiple registers in a single instruction; handled by peephole optimizer.
- **Thumb IT (If-Then) blocks**: Multiple instructions under one condition code; handled specially in `ARMInstrInfo.cpp`.
- **VFP (Vector Floating-Point)**: Separate register file for floating-point operations; defined in `ARMRegisterInfo.td` with FPR classes.
- **NEON**: SIMD extension; instruction definitions in `ARMInstrInfo.td` with NEON predicates.

## Dependencies

### Internal

- `lib/CodeGen/` — Target-independent instruction selection, register allocation, instruction scheduling
- `lib/MC/` — Machine Code layer for binary emission and assembler
- `include/llvm/Target/` — Abstract TargetMachine, TargetInfo, TargetLowering interfaces

### External

- `llvm-tblgen` — TableGen compiler (invoked by build system to generate `.inc` files from `.td` definitions)

<!-- MANUAL: ARM backend's conditional execution model is unusual; study carefully before modifying predication logic or introducing new instruction patterns. -->
