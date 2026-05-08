<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToEmitC

## Purpose
Declares the Arith to EmitC lowering pass. Lowers Arith dialect arithmetic and comparison ops to EmitC ops for C/C++ code emission.

## Key Files
| File | Description |
|------|-------------|
| `ArithToEmitC.h` | Conversion pattern declarations |
| `ArithToEmitCPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `ArithToEmitCPass.h` to modify pass registration; edit `ArithToEmitC.h` for pattern helpers
- The implementation lives under `mlir/lib/Conversion/ArithToEmitC/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Target dialect: `include/mlir/Dialect/EmitC/`

<!-- MANUAL: -->
