<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PowerPC Backend

## Purpose
Targets IBM Power/PowerPC 32/64-bit processors. Used in server systems, embedded devices, and legacy computing platforms.

## Key Files
| File | Description |
|------|-------------|
| `PPCTargetMachine.cpp` | TargetMachine implementation |
| `PPCISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `PPCISelLowering.cpp` | Calling convention and custom DAG lowering |
| `PPCAsmPrinter.cpp` | Assembly output |
| `PPCInstrInfo.cpp` | Instruction info implementation |
| `PPCRegisterInfo.cpp` | Register info implementation |
| `PPCFrameLowering.cpp` | Prologue/epilogue |
| `PPC.td` | Main target definition |
| `PPCInstrInfo.td` | Instruction definitions |
| `PPCCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |
| `GISel/` | Global instruction selection |
| `MCTargetDesc/` | MC target description |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- PPCISelLowering.cpp handles calling conventions and custom DAG lowering
- PPCCTRLoops.cpp optimizes loops with count-to-zero register
- PPCBranchCoalescing.cpp merges branch chains
- PPCEarlyReturn.cpp extracts early returns

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
