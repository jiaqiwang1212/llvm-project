<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToSPIRV

## Purpose
Declares the Vector to SPIR-V lowering pass. Lowers Vector dialect ops to SPIR-V composite and vector ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `VectorToSPIRV.h` | Conversion pattern declarations |
| `VectorToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `VectorToSPIRVPass.h` for pass registration; edit `VectorToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/VectorToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
