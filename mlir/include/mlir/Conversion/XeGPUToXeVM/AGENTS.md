<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XeGPUToXeVM

## Purpose
Declares the XeGPU to XeVM lowering pass. Lowers XeGPU (Intel Xe GPU) dialect ops to XeVM (Intel Xe Virtual Machine) intrinsics for Intel GPU code generation.

## Key Files
| File | Description |
|------|-------------|
| `XeGPUToXeVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `XeGPUToXeVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/XeGPUToXeVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/XeGPU/`
- Target dialect: `include/mlir/Dialect/XeVM/` or LLVM dialect with XeVM intrinsics

<!-- MANUAL: -->
