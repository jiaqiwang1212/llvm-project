<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlowToSPIRV

## Purpose
Declares the ControlFlow to SPIR-V lowering pass. Lowers cf dialect branch ops to SPIR-V branch and merge ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `ControlFlowToSPIRV.h` | Conversion pattern declarations |
| `ControlFlowToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `ControlFlowToSPIRVPass.h` for pass registration; edit `ControlFlowToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/ControlFlowToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ControlFlow/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
