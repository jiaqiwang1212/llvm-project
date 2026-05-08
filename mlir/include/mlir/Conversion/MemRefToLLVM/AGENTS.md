<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToLLVM

## Purpose
Declares the MemRef to LLVM IR lowering pass. Lowers MemRef dialect allocation, load, store, and view ops to LLVM dialect pointer and struct ops using the standard memref descriptor layout.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MemRefToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MemRefToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/MemRef/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`
- Shared utilities: `LLVMCommon/` (MemRefBuilder, TypeConverter)

<!-- MANUAL: -->
