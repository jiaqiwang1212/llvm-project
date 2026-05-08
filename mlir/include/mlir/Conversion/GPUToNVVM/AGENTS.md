<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToNVVM

## Purpose
Declares the GPU to NVVM lowering pass. Lowers GPU dialect ops to NVVM (NVIDIA Virtual Machine) dialect intrinsics for CUDA GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `GPUToNVVM.h` | Conversion pattern declarations |
| `GPUToNVVMPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `GPUToNVVMPass.h` for pass registration; edit `GPUToNVVM.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/GPUToNVVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/GPU/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (NVVM ops)
- Shared utilities: `GPUCommon/`

<!-- MANUAL: -->
