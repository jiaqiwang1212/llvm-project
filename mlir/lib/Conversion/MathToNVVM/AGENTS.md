<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToNVVM Conversion

## Purpose
Lowers Math dialect ops to NVVM dialect (NVIDIA GPU intrinsics). Converts transcendental math ops to their NVVM hardware intrinsic equivalents for CUDA GPU code generation.

## Key Files
| File | Description |
|------|-------------|
| `MathToNVVM.cpp` | Patterns mapping math ops to nvvm intrinsics |

## For AI Agents

### Working In This Directory
- `math.exp f32` → `nvvm.ex2.approx.ftz.f` (fast approximate exponential).
- `math.sin`/`math.cos` → `nvvm.sin.approx.f`/`nvvm.cos.approx.f`.
- `math.sqrt` → `nvvm.sqrt.approx.f` or `nvvm.sqrt.rn.f` depending on precision mode.
- Uses `OpToFuncCallLowering` template from `GPUCommon` for function-call style intrinsics.
- NVVM math intrinsics are approximate — verify precision requirements before using.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/NVVM`
- Utility: `mlir/Conversion/GPUCommon`

<!-- MANUAL: -->
