<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToEmitC

## Purpose
Declares the MemRef to EmitC lowering pass. Lowers MemRef dialect memory operations to EmitC ops for C/C++ array and pointer emission.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToEmitC.h` | Conversion pattern declarations |
| `MemRefToEmitCPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `MemRefToEmitCPass.h` for pass registration; edit `MemRefToEmitC.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/MemRefToEmitC/`

## Dependencies
- Source dialect: `include/mlir/Dialect/MemRef/`
- Target dialect: `include/mlir/Dialect/EmitC/`

<!-- MANUAL: -->
