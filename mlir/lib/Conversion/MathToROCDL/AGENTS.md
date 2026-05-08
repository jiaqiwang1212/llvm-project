<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToROCDL Conversion

## Purpose
Lowers Math dialect ops to ROCDL dialect / OCML library calls for AMD GPU targets. Converts transcendental math ops to AMD's OpenCL Math Library (OCML) device function calls.

## Key Files
| File | Description |
|------|-------------|
| `MathToROCDL.cpp` | Patterns mapping math ops to OCML function calls or ROCDL intrinsics |

## For AI Agents

### Working In This Directory
- `math.exp` → `__ocml_exp_f32`, `math.sin` → `__ocml_sin_f32`, etc. (OCML naming convention).
- Uses `OpToFuncCallLowering` template from `GPUCommon` for systematic function call generation.
- OCML functions are provided by AMD's ROCm device libraries — they must be linked at compile time.
- For f16 variants, uses packed f16 intrinsics when available on the target chipset.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/ROCDL`, OCML library (AMD ROCm)
- Utility: `mlir/Conversion/GPUCommon`

<!-- MANUAL: -->
