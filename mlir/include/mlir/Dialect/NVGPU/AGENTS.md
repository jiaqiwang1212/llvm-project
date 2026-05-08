<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU Dialect

## Purpose
Represents NVIDIA GPU-specific operations above NVVM level: warp-level matrix multiply (wgmma), Tensor Memory Accelerator (TMA) operations, async copy, and other CUDA-specific ops. Bridges the gap between generic GPU dialect and NVVM intrinsics.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, and dialect definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | MMA utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files
- TMA ops require Hopper (SM90+) hardware; WGMMA requires Hopper+

### Common Patterns
- Op names follow `nvgpu.*` convention
- Lowered to NVVM intrinsics via NVGPUToNVVM conversion

## Dependencies
- Depends on: GPU dialect, NVVM dialect, Vector dialect

<!-- MANUAL: -->
