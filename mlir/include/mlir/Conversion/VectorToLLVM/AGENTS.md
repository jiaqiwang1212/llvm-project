<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToLLVM

## Purpose
Declares the Vector to LLVM IR lowering pass. Lowers Vector dialect ops (transfer_read/write, shuffles, reductions, contractions) to LLVM dialect vector ops and intrinsics.

## Key Files
| File | Description |
|------|-------------|
| `ConvertVectorToLLVM.h` | Conversion pattern declarations |
| `ConvertVectorToLLVMPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `ConvertVectorToLLVMPass.h` for pass registration; edit `ConvertVectorToLLVM.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/VectorToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared utilities: `LLVMCommon/` (VectorPattern)

<!-- MANUAL: -->
