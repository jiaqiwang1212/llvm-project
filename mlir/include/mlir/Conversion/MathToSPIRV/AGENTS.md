<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToSPIRV

## Purpose
Declares the Math to SPIR-V lowering pass. Lowers Math dialect transcendental and math ops to SPIR-V GLSL/OpenCL extended instruction set ops for GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `MathToSPIRV.h` | Conversion pattern declarations |
| `MathToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `MathToSPIRVPass.h` for pass registration; edit `MathToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/MathToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
