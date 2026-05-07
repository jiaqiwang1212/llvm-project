<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MSP430 Backend

## Purpose
Targets Texas Instruments MSP430, a 16-bit low-power microcontroller family. Used in battery-powered embedded systems, IoT sensors, and wearable devices.

## Key Files
| File | Description |
|------|-------------|
| `MSP430TargetMachine.cpp` | TargetMachine implementation |
| `MSP430ISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `MSP430ISelLowering.cpp` | Calling convention and custom DAG lowering |
| `MSP430AsmPrinter.cpp` | Assembly output |
| `MSP430InstrInfo.cpp` | Instruction info implementation |
| `MSP430RegisterInfo.cpp` | Register info implementation |
| `MSP430FrameLowering.cpp` | Prologue/epilogue |
| `MSP430.td` | Main target definition |
| `MSP430InstrFormats.td` | Instruction format definitions |
| `MSP430CallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- MSP430ISelLowering.cpp handles calling conventions and custom DAG lowering
- MSP430BranchSelector.cpp manages branch ranges
- Focus on 16-bit operations and low code size

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
