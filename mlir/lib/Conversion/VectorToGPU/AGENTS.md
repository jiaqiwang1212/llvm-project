<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToGPU

## Purpose
Implements the Vector to GPU lowering pass. Converts `vector` dialect warp-level matrix ops (targeting WMMA/MMA fragments) into `gpu` dialect matrix ops for NVIDIA and AMD GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `VectorToGPU.cpp` | Conversion patterns from vector ops to gpu.subgroup_mma_* ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToGPUConversionPatterns()`
- Vector shapes must correspond to supported WMMA fragment sizes (e.g., 16x16x16 for f16)

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `vector.contract` with matching shapes → `gpu.subgroup_mma_compute`

## Dependencies
- Headers: `include/mlir/Conversion/VectorToGPU/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/GPU/`

<!-- MANUAL: -->
