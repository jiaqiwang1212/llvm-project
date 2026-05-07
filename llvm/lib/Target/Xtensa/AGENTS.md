<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Xtensa Backend

## Purpose
Targets Tensilica Xtensa, a configurable RISC ISA from Cadence. Used in ESP32 microcontrollers and other embedded systems with customizable processor extensions.

## Key Files
| File | Description |
|------|-------------|
| `XtensaTargetMachine.cpp` | TargetMachine implementation |
| `XtensaISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `XtensaISelLowering.cpp` | Calling convention and custom DAG lowering |
| `XtensaAsmPrinter.cpp` | Assembly output |
| `XtensaInstrInfo.cpp` | Instruction info implementation |
| `XtensaRegisterInfo.cpp` | Register info implementation |
| `XtensaFrameLowering.cpp` | Prologue/epilogue |
| `Xtensa.td` | Main target definition |
| `XtensaInstrFormats.td` | Instruction format definitions |
| `XtensaCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `MCTargetDesc/` | MC target description |
| `TargetInfo/` | Target information |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- XtensaISelLowering.cpp handles calling conventions and custom DAG lowering
- XtensaConstantPoolValue.cpp manages constant pool entries
- XtensaDSPInstrInfo.td for DSP instruction extensions
- Support for configurable processor variants (ESP32, etc.)

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
