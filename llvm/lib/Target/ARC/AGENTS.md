<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ARC Backend

## Purpose
Compiles LLVM IR to ARC (Synopsys ARC) processor machine code. ARC is a configurable 32-bit embedded RISC architecture used in IoT, automotive, and industrial microcontrollers.

## Key Files
| File | Description |
|------|-------------|
| `ARC.td` | Architecture features and instruction definitions |
| `ARCISelLowering.cpp` | DAG lowering and custom operations |
| `ARCISelDAGToDAG.cpp` | Instruction selection patterns |
| `ArcInstrInfo.cpp` | Instruction metadata and pseudoinstructions |
| `ARCFrameLowering.cpp` | Stack frame and prologue/epilogue handling |
| `ARCAsmPrinter.cpp` | Assembly output |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `MCTargetDesc/` | MC layer: encoding, register definitions |
| `TargetInfo/` | Target registration |
| `Disassembler/` | Binary machine code to assembly conversion |

## For AI Agents

### Working In This Directory
- Edit `.td` files for instruction/register changes, rebuild with `llvm-tblgen`
- Modify `ARCISelLowering.cpp` for calling conventions and custom lowering
- Update `ARCFrameLowering.cpp` for stack layout changes
- Compiler intrinsics in MCTargetDesc/ for encoding/decoding

## Dependencies

### Internal
- `lib/CodeGen/` — Standard LLVM code generation infrastructure
- `lib/MC/` — Machine code layer

### External
- ARC ISA documentation (Synopsys)
- ARC GNU toolchain reference

