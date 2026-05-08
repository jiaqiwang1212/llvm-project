<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToSPIRV

## Purpose
Declares the GPU to SPIR-V lowering pass. Lowers GPU dialect ops (launch, thread/block IDs, barriers) to SPIR-V ops for Vulkan or OpenCL GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `GPUToSPIRV.h` | Conversion pattern declarations |
| `GPUToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `GPUToSPIRVPass.h` for pass registration; edit `GPUToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/GPUToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/GPU/`
- Target dialect: `include/mlir/Dialect/SPIRV/`
- Shared utilities: `GPUCommon/`

<!-- MANUAL: -->
