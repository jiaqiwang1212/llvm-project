<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToXeVM

## Purpose
Declares the Math to XeVM lowering pass. Lowers Math dialect ops to XeVM (Intel Xe GPU Virtual Machine) intrinsics for Intel GPU math execution.

## Key Files
| File | Description |
|------|-------------|
| `MathToXeVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToXeVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToXeVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/XeVM/` or LLVM dialect with XeVM intrinsics

<!-- MANUAL: -->
