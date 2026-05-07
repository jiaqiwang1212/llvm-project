<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# M68k Backend

## Purpose
Targets Motorola 68000 series processors, a historical CISC architecture. Used for retro computing, embedded systems, and automotive applications.

## Key Files
| File | Description |
|------|-------------|
| `M68kTargetMachine.cpp` | TargetMachine implementation |
| `M68kISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `M68kISelLowering.cpp` | Calling convention and custom DAG lowering |
| `M68kAsmPrinter.cpp` | Assembly output |
| `M68kInstrInfo.cpp` | Instruction info implementation |
| `M68kRegisterInfo.cpp` | Register info implementation |
| `M68kFrameLowering.cpp` | Prologue/epilogue |
| `M68k.td` | Main target definition |
| `M68kInstrArithmetic.td` | Arithmetic instructions |
| `M68kCallingConv.h` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `GISel/` | Global instruction selection (if present) |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- M68kISelLowering.cpp handles calling conventions and custom DAG lowering
- M68kExpandPseudo.cpp expands pseudo-instructions
- M68kCollapseMOVEMPass.cpp optimizes move-multiple instructions

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
