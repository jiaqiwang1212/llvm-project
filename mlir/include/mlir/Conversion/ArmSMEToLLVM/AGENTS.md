<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSMEToLLVM

## Purpose
Declares the ArmSME to LLVM IR lowering pass. Lowers Arm Scalable Matrix Extension (SME) dialect ops to LLVM dialect intrinsics for native SME hardware execution.

## Key Files
| File | Description |
|------|-------------|
| `ArmSMEToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArmSMEToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArmSMEToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ArmSME/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
