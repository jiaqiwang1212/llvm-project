<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVVMToLLVM

## Purpose
Declares the NVVM to LLVM IR lowering pass. Lowers NVVM dialect intrinsics to LLVM dialect ops, completing the CUDA GPU code generation pipeline.

## Key Files
| File | Description |
|------|-------------|
| `NVVMToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `NVVMToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/NVVMToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/LLVMIR/` (NVVM ops)
- Target dialect: `include/mlir/Dialect/LLVMIR/` (plain LLVM ops)

<!-- MANUAL: -->
