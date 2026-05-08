<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToEmitC

## Purpose
Declares the Math to EmitC lowering pass. Lowers Math dialect transcendental and math ops to EmitC ops that map to C standard math functions for C/C++ code emission.

## Key Files
| File | Description |
|------|-------------|
| `MathToEmitC.h` | Conversion pattern declarations |
| `MathToEmitCPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `MathToEmitCPass.h` for pass registration; edit `MathToEmitC.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/MathToEmitC/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/EmitC/`

<!-- MANUAL: -->
