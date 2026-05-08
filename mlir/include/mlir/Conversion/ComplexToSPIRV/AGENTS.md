<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToSPIRV

## Purpose
Declares the Complex to SPIR-V lowering pass. Lowers Complex dialect ops to SPIR-V ops for GPU shader/kernel compilation targeting Vulkan or OpenCL.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToSPIRV.h` | Conversion pattern declarations |
| `ComplexToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `ComplexToSPIRVPass.h` for pass registration; edit `ComplexToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/ComplexToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
