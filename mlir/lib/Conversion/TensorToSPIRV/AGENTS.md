<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TensorToSPIRV

## Purpose
Implements the Tensor to SPIR-V lowering pass. Converts `tensor` dialect ops (extract, insert) into SPIR-V composite object access ops targeting Vulkan/OpenCL shader execution.

## Key Files
| File | Description |
|------|-------------|
| `TensorToSPIRV.cpp` | Core conversion patterns from tensor ops to SPIR-V ops |
| `TensorToSPIRVPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTensorToSPIRVPatterns()`
- SPIRVTypeConverter maps tensor element types to SPIR-V scalar/vector types

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `tensor.extract` → `spirv.CompositeExtract` for statically-shaped tensors

## Dependencies
- Headers: `include/mlir/Conversion/TensorToSPIRV/`
- Source dialect: `lib/Dialect/Tensor/`
- Target dialect: `lib/Dialect/SPIRV/`

<!-- MANUAL: -->
