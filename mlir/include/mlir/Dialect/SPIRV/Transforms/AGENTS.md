<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV Transforms

## Purpose
Transformation passes for the SPIRV dialect: ABI lowering, canonicalization, WebGPU compatibility transforms, and type conversion infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `SPIRVConversion.h` | SPIR-V type converter and conversion target |
| `SPIRVWebGPUTransforms.h` | WebGPU-specific SPIRV transformation patterns |

## For AI Agents

### Working In This Directory
- `SPIRVConversion.h` provides `SPIRVTypeConverter` used in dialect-to-SPIRV conversion passes
- `SPIRVConversionTarget` sets up the conversion target for SPIR-V dialect

## Dependencies
- Depends on: SPIRV IR, MLIR conversion infrastructure

<!-- MANUAL: -->
