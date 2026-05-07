<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPARC Backend

## Purpose
Targets SPARC V8/V9 architecture, historically used in Sun Microsystems workstations and servers. Used in embedded systems and legacy computing.

## Key Files
| File | Description |
|------|-------------|
| `SparcTargetMachine.cpp` | TargetMachine implementation |
| `SparcISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `SparcISelLowering.cpp` | Calling convention and custom DAG lowering |
| `SparcAsmPrinter.cpp` | Assembly output |
| `SparcInstrInfo.cpp` | Instruction info implementation |
| `SparcRegisterInfo.cpp` | Register info implementation |
| `SparcFrameLowering.cpp` | Prologue/epilogue |
| `Sparc.td` | Main target definition |
| `SparcInstrFormats.td` | Instruction format definitions |
| `SparcCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- SparcISelLowering.cpp handles calling conventions and custom DAG lowering
- DelaySlotFiller.cpp fills SPARC delay slots
- LeonPasses.cpp implements Leon-specific optimizations
- SparcInstr64Bit.td handles 64-bit extensions

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
