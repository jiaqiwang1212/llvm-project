<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LoongArch Backend

## Purpose
Targets LoongArch, a 32/64-bit RISC ISA from Loongson/Alibaba based on MIPS principles. Used in Chinese consumer and embedded systems.

## Key Files
| File | Description |
|------|-------------|
| `LoongArchTargetMachine.cpp` | TargetMachine implementation |
| `LoongArchISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `LoongArchISelLowering.cpp` | Calling convention and custom DAG lowering |
| `LoongArchAsmPrinter.cpp` | Assembly output |
| `LoongArchInstrInfo.cpp` | Instruction info implementation |
| `LoongArchRegisterInfo.cpp` | Register info implementation |
| `LoongArchFrameLowering.cpp` | Prologue/epilogue |
| `LoongArch.td` | Main target definition |
| `LoongArchInstrInfo.td` | Instruction definitions |
| `LoongArchCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- LoongArchISelLowering.cpp handles calling conventions and custom DAG lowering
- LoongArchExpandPseudoInsts.cpp expands pseudo-instructions
- Float instruction handling in Float32/64InstrInfo.td

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
