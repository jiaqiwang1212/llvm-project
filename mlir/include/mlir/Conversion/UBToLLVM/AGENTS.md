<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UBToLLVM

## Purpose
Declares the UB (Undefined Behavior) to LLVM IR lowering pass. Lowers UB dialect ops (poison values) to LLVM dialect poison/undef constants.

## Key Files
| File | Description |
|------|-------------|
| `UBToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `UBToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/UBToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/UB/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
