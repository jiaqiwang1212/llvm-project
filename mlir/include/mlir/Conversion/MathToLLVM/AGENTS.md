<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToLLVM

## Purpose
Declares the Math to LLVM IR lowering pass. Lowers Math dialect ops to LLVM dialect intrinsics (llvm.intr.sqrt, llvm.intr.fabs, etc.) for native hardware math support.

## Key Files
| File | Description |
|------|-------------|
| `MathToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
