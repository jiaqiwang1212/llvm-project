<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToFuncs

## Purpose
Declares the Math to Funcs lowering pass. Expands Math dialect ops that lack direct hardware/library support into sequences of standard Func dialect calls with software emulation.

## Key Files
| File | Description |
|------|-------------|
| `MathToFuncs.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `MathToFuncs.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/MathToFuncs/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Math/`
- Target dialect: `include/mlir/Dialect/Func/`

<!-- MANUAL: -->
