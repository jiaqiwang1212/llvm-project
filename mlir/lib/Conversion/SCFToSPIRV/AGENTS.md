<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToSPIRV

## Purpose
Implements the SCF to SPIR-V lowering pass. Converts `scf.for`, `scf.if`, and `scf.while` ops into SPIR-V structured control flow ops (`spirv.mlir.loop`, `spirv.mlir.selection`) targeting the Vulkan/OpenCL execution model.

## Key Files
| File | Description |
|------|-------------|
| `SCFToSPIRV.cpp` | Core conversion patterns for SCF ops to SPIR-V control flow |
| `SCFToSPIRVPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateSCFToSPIRVPatterns()`
- SPIRVTypeConverter handles loop-carried value type lowering

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- SPIR-V structured control flow requires explicit `spirv.mlir.merge` blocks

## Dependencies
- Headers: `include/mlir/Conversion/SCFToSPIRV/`
- Source dialect: `lib/Dialect/SCF/`
- Target dialect: `lib/Dialect/SPIRV/`

<!-- MANUAL: -->
