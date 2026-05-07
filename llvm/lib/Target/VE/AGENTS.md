<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Vector Engine Backend

## Purpose
Targets NEC Vector Engine (Aurora), a manycore vector processor for high-performance computing. Used in supercomputing and scientific simulation.

## Key Files
| File | Description |
|------|-------------|
| `VETargetMachine.cpp` | TargetMachine implementation |
| `VEISelDAGToDAG.cpp` | DAG-to-DAG instruction selection |
| `VEISelLowering.cpp` | Calling convention and custom DAG lowering |
| `VEAsmPrinter.cpp` | Assembly output |
| `VEInstrInfo.cpp` | Instruction info implementation |
| `VERegisterInfo.cpp` | Register info implementation |
| `VEFrameLowering.cpp` | Prologue/epilogue |
| `VE.td` | Main target definition |
| `VEInstrFormats.td` | Instruction format definitions |
| `VECallingConv.td` | Calling convention definitions |

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
- VEISelLowering.cpp handles calling conventions and custom DAG lowering
- VECustomDAG.cpp implements custom DAG operations for vectorization
- LVLGen.cpp generates vector length control code
- VEInstrIntrinsicVL.gen.td for vector length intrinsics

## Dependencies

### Internal
- lib/CodeGen/ (target-independent infrastructure)
- lib/MC/ (machine code layer)

<!-- MANUAL: -->
