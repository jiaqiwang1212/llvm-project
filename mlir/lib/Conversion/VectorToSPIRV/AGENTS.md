<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToSPIRV

## Purpose
Implements the Vector to SPIR-V lowering pass. Converts `vector` dialect ops (broadcast, extract, insert, shuffle, reduction) into SPIR-V composite and vector ops targeting Vulkan/OpenCL shaders.

## Key Files
| File | Description |
|------|-------------|
| `VectorToSPIRV.cpp` | Core conversion patterns from vector ops to SPIR-V ops |
| `VectorToSPIRVPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToSPIRVPatterns()`
- SPIRVTypeConverter maps `vector<NxT>` to `!spirv.vec<N x T>` (N must be 2, 3, or 4)

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- SPIR-V vectors are limited to 2/3/4 elements; wider vectors require scalarization

## Dependencies
- Headers: `include/mlir/Conversion/VectorToSPIRV/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/SPIRV/`

<!-- MANUAL: -->
