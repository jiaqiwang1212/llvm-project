<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToSPIRV

## Purpose
Declares the Func to SPIR-V lowering pass. Lowers Func dialect function definitions and calls to SPIR-V function ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `FuncToSPIRV.h` | Conversion pattern declarations |
| `FuncToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `FuncToSPIRVPass.h` for pass registration; edit `FuncToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/FuncToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Func/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
