<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToLLVM

## Purpose
Declares the Func to LLVM IR lowering pass. Lowers Func dialect function definitions, calls, and returns to LLVM dialect equivalents including ABI and calling convention handling.

## Key Files
| File | Description |
|------|-------------|
| `ConvertFuncToLLVM.h` | Conversion pattern declarations |
| `ConvertFuncToLLVMPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `ConvertFuncToLLVMPass.h` for pass registration; edit `ConvertFuncToLLVM.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/FuncToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Func/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared type converter: `LLVMCommon/`

<!-- MANUAL: -->
