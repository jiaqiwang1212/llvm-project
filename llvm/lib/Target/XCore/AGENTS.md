<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XCore Backend

## Purpose
Targets XMOS XCore communication processor, a multi-core embedded processor. Used in real-time systems, audio processing, and IoT applications.

## Key Files
| File | Description |
|------|-------------|
| `XCoreTargetMachine.cpp` | TargetMachine implementation |
| `XCoreISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `XCoreISelLowering.cpp` | Calling convention and custom DAG lowering |
| `XCoreAsmPrinter.cpp` | Assembly output |
| `XCoreInstrInfo.cpp` | Instruction info implementation |
| `XCoreRegisterInfo.cpp` | Register info implementation |
| `XCoreFrameLowering.cpp` | Prologue/epilogue |
| `XCore.td` | Main target definition |
| `XCoreInstrInfo.td` | Instruction definitions |
| `XCoreCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Disassembler/` | Disassembly support |
| `MCTargetDesc/` | MC target description |
| `TargetInfo/` | Target information |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- XCoreISelLowering.cpp handles calling conventions and custom DAG lowering
- XCoreLowerThreadLocal.cpp lowers thread-local storage
- XCoreFrameToArgsOffsetElim.cpp optimizes frame addressing
- Real-time and multi-core specific considerations

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
