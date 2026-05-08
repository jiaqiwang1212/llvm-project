<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToLLVM

## Purpose
Declares the ControlFlow to LLVM IR lowering pass. Lowers cf dialect ops (br, cond_br, switch, assert) to LLVM dialect branch and conditional branch ops.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ControlFlowToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ControlFlowToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ControlFlow/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
