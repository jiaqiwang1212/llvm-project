<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenMPToLLVM

## Purpose
Declares the OpenMP to LLVM IR lowering pass. Lowers OpenMP dialect ops to LLVM dialect calls into the OpenMP runtime library for CPU/GPU parallel execution.

## Key Files
| File | Description |
|------|-------------|
| `ConvertOpenMPToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ConvertOpenMPToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/OpenMPToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/OpenMP/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
