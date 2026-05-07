<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# C-SKY Backend

## Purpose
Targets C-SKY (China Standard Instruction Set Architecture), a 32-bit RISC ISA used in embedded processors from T-Head/Alibaba. Used for embedded systems, microcontrollers, and IoT devices.

## Key Files
| File | Description |
|------|-------------|
| `CSKYTargetMachine.cpp` | TargetMachine implementation |
| `CSKYISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `CSKYISelLowering.cpp` | Calling convention and custom DAG lowering |
| `CSKYAsmPrinter.cpp` | Assembly output |
| `CSKYInstrInfo.cpp` | Instruction info implementation |
| `CSKYRegisterInfo.cpp` | Register info implementation |
| `CSKYFrameLowering.cpp` | Prologue/epilogue |
| `CSKY.td` | Main target definition |
| `CSKYInstrFormats.td` | Instruction format definitions |
| `CSKYCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- CSKYISelLowering.cpp handles calling conventions and custom DAG lowering
- CSKYConstantIslandPass.cpp optimizes constant pool access

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
