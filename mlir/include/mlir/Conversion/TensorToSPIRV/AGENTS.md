<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TensorToSPIRV

## Purpose
Declares the Tensor to SPIR-V lowering pass. Lowers Tensor dialect ops to SPIR-V array and composite ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `TensorToSPIRV.h` | Conversion pattern declarations |
| `TensorToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `TensorToSPIRVPass.h` for pass registration; edit `TensorToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/TensorToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tensor/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
