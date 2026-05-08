<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AsyncToLLVM

## Purpose
Declares the Async to LLVM IR lowering pass. Lowers Async dialect ops (async.execute, async.await, tokens, values) to LLVM dialect calls into the MLIR async runtime library.

## Key Files
| File | Description |
|------|-------------|
| `AsyncToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `AsyncToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/AsyncToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Async/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
