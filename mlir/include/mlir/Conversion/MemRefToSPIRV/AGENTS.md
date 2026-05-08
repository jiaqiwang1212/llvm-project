<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToSPIRV

## Purpose
Declares the MemRef to SPIR-V lowering pass. Lowers MemRef dialect memory ops to SPIR-V load/store and variable ops for GPU shader/kernel memory access.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToSPIRV.h` | Conversion pattern declarations |
| `MemRefToSPIRVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `MemRefToSPIRVPass.h` for pass registration; edit `MemRefToSPIRV.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/MemRefToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/MemRef/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
