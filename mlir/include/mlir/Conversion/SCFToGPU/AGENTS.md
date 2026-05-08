<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToGPU

## Purpose
Declares the SCF to GPU lowering pass. Maps SCF parallel loop constructs to GPU dialect thread/block launch ops, enabling GPU kernel generation from loop nests.

## Key Files
| File | Description |
|------|-------------|
| `SCFToGPU.h` | Conversion pattern declarations |
| `SCFToGPUPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `SCFToGPUPass.h` for pass registration; edit `SCFToGPU.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/SCFToGPU/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SCF/`
- Target dialect: `include/mlir/Dialect/GPU/`

<!-- MANUAL: -->
