<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToNVVM

## Purpose
Declares the Math to NVVM lowering pass. Lowers Math dialect ops to NVVM (NVIDIA Virtual Machine) intrinsics for optimized CUDA GPU math execution.

## Key Files
| File | Description |
|------|-------------|
| `MathToNVVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToNVVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToNVVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (NVVM ops)

<!-- MANUAL: -->
