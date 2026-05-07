<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Hexagon Backend

## Purpose
Targets Qualcomm Hexagon, a VLIW (Very Long Instruction Word) DSP architecture. Used in mobile SoCs and embedded systems for signal processing, audio/video, and compute workloads.

## Key Files
| File | Description |
|------|-------------|
| `HexagonTargetMachine.cpp` | TargetMachine implementation |
| `HexagonISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `HexagonISelLowering.cpp` | Calling convention and custom DAG lowering |
| `HexagonAsmPrinter.cpp` | Assembly output |
| `HexagonInstrInfo.cpp` | Instruction info implementation |
| `HexagonRegisterInfo.cpp` | Register info implementation |
| `HexagonFrameLowering.cpp` | Prologue/epilogue |
| `Hexagon.td` | Main target definition |
| `HexagonCallingConv.td` | Calling convention definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AsmParser/` | Assembly parser implementation |
| `Disassembler/` | Disassembly support |

## For AI Agents

### Working In This Directory
- Edit .td files for instruction/register changes, then rebuild with llvm-tblgen
- HexagonISelLowering.cpp handles VLIW calling conventions and custom lowering
- HexagonPacketizer.cpp handles VLIW packet formation
- BitTracker.cpp provides bit-level data flow analysis

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
