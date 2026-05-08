<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToLLVM

## Purpose
Declares the Arith to LLVM IR lowering pass. Lowers Arith dialect integer and floating-point arithmetic ops to their LLVM dialect equivalents.

## Key Files
| File | Description |
|------|-------------|
| `ArithToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArithToLLVM.h` to add or modify pass declarations and pattern helpers
- The implementation lives under `mlir/lib/Conversion/ArithToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared utilities: `ArithCommon/`

<!-- MANUAL: -->
