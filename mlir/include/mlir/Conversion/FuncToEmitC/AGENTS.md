<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToEmitC

## Purpose
Declares the Func to EmitC lowering pass. Lowers Func dialect function definitions and calls to EmitC ops for C/C++ function emission.

## Key Files
| File | Description |
|------|-------------|
| `FuncToEmitC.h` | Conversion pattern declarations |
| `FuncToEmitCPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `FuncToEmitCPass.h` for pass registration; edit `FuncToEmitC.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/FuncToEmitC/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Func/`
- Target dialect: `include/mlir/Dialect/EmitC/`

<!-- MANUAL: -->
