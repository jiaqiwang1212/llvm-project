<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToROCDL

## Purpose
Declares the GPU to ROCDL lowering pass. Lowers GPU dialect ops to ROCDL (ROCm Device Library) intrinsics for AMD GPU execution via ROCm/HIP.

## Key Files
| File | Description |
|------|-------------|
| `GPUToROCDLPass.h` | Pass pipeline entry-points and option structs |
| `Runtimes.h` | Enumerations for supported ROCDL runtime variants |

## For AI Agents

### Working In This Directory
- Edit `GPUToROCDLPass.h` to modify pass declarations; edit `Runtimes.h` to add new runtime variants
- The implementation lives under `mlir/lib/Conversion/GPUToROCDL/`

## Dependencies
- Source dialect: `include/mlir/Dialect/GPU/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` (ROCDL ops)
- Shared utilities: `GPUCommon/`

<!-- MANUAL: -->
