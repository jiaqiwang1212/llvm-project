<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SystemZ Backend

## Purpose
Targets IBM System Z (s390x) mainframe architecture. Used in enterprise data centers and mission-critical systems.

## Key Files
| File | Description |
|------|-------------|
| `SystemZTargetMachine.cpp` | TargetMachine implementation |
| `SystemZISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `SystemZISelLowering.cpp` | Calling convention and custom DAG lowering |
| `SystemZAsmPrinter.cpp` | Assembly output |
| `SystemZInstrInfo.cpp` | Instruction info implementation |
| `SystemZRegisterInfo.cpp` | Register info implementation |
| `SystemZFrameLowering.cpp` | Prologue/epilogue |
| `SystemZ.td` | Main target definition |
| `SystemZInstrFormats.td` | Instruction format definitions |
| `SystemZCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- SystemZISelLowering.cpp handles calling conventions and custom DAG lowering
- SystemZElimCompare.cpp eliminates redundant comparisons
- SystemZHazardRecognizer.cpp handles CPU hazards
- Mainframe-specific memory model and instruction scheduling

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
