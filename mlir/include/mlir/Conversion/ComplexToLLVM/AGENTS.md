<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToLLVM

## Purpose
Declares the Complex to LLVM IR lowering pass. Lowers Complex dialect ops (create, re, im, arithmetic) to LLVM dialect struct operations representing complex numbers as pairs of floats.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ComplexToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ComplexToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared utilities: `ComplexCommon/`

<!-- MANUAL: -->
