<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToSPIRV

## Purpose
Declares the SCF to SPIR-V lowering pass. Lowers SCF dialect structured loops and conditionals to SPIR-V loop and selection merge constructs for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `SCFToSPIRV.h` | Conversion pattern declarations |
| `SCFToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `SCFToSPIRVPass.h` for pass registration; edit `SCFToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/SCFToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SCF/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
