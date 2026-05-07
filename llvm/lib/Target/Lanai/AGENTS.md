<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Lanai Backend

## Purpose
Targets Google's Lanai, an educational RISC ISA designed for compiler teaching and research. Used for learning compiler design and testing code generation algorithms.

## Key Files
| File | Description |
|------|-------------|
| `LanaiTargetMachine.cpp` | TargetMachine implementation |
| `LanaiISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `LanaiISelLowering.cpp` | Calling convention and custom DAG lowering |
| `LanaiAsmPrinter.cpp` | Assembly output |
| `LanaiInstrInfo.cpp` | Instruction info implementation |
| `LanaiRegisterInfo.cpp` | Register info implementation |
| `LanaiFrameLowering.cpp` | Prologue/epilogue |
| `Lanai.td` | Main target definition |
| `LanaiInstrInfo.td` | Instruction definitions |
| `LanaiCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- LanaiISelLowering.cpp handles calling conventions and custom DAG lowering
- LanaiDelaySlotFiller.cpp manages delay slot scheduling
- LanaiCondCode.h defines condition codes

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
