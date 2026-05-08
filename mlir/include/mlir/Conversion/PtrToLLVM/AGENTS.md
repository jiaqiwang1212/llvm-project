<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PtrToLLVM

## Purpose
Declares the Ptr to LLVM IR lowering pass. Lowers Ptr dialect pointer ops to LLVM dialect pointer ops, bridging the typed pointer abstraction to LLVM's opaque pointer model.

## Key Files
| File | Description |
|------|-------------|
| `PtrToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `PtrToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/PtrToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Ptr/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
