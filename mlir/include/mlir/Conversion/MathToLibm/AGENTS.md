<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToLibm

## Purpose
Declares the Math to Libm lowering pass. Lowers Math dialect transcendental ops (sin, cos, exp, log, etc.) to calls into the C standard math library (libm).

## Key Files
| File | Description |
|------|-------------|
| `MathToLibm.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToLibm.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToLibm/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target: Func dialect calls to libm symbols

<!-- MANUAL: -->
