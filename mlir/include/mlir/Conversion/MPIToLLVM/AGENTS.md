<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MPIToLLVM

## Purpose
Declares the MPI to LLVM IR lowering pass. Lowers MPI dialect ops to LLVM dialect calls into the MPI C runtime library (e.g., MPI_Send, MPI_Recv).

## Key Files
| File | Description |
|------|-------------|
| `MPIToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MPIToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MPIToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/MPI/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
