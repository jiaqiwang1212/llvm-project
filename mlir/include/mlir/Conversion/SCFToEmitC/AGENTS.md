<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToEmitC

## Purpose
Declares the SCF to EmitC lowering pass. Lowers SCF dialect structured loops and conditionals to EmitC for/if ops for C/C++ control flow emission.

## Key Files
| File | Description |
|------|-------------|
| `SCFToEmitC.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `SCFToEmitC.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/SCFToEmitC/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SCF/`
- Target dialect: `include/mlir/Dialect/EmitC/`

<!-- MANUAL: -->
