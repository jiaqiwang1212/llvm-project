<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XeVMToLLVM

## Purpose
Declares the XeVM to LLVM IR lowering pass. Lowers XeVM (Intel Xe Virtual Machine) dialect intrinsics to LLVM dialect ops, completing the Intel GPU code generation pipeline.

## Key Files
| File | Description |
|------|-------------|
| `XeVMToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `XeVMToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/XeVMToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/XeVM/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
