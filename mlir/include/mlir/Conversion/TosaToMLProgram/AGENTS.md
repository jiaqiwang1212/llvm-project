<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToMLProgram

## Purpose
Declares the TOSA to MLProgram lowering pass. Lowers TOSA variable ops to MLProgram dialect global variable ops for ML model weight representation.

## Key Files
| File | Description |
|------|-------------|
| `TosaToMLProgram.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `TosaToMLProgram.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/TosaToMLProgram/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tosa/`
- Target dialect: `include/mlir/Dialect/MLProgram/`

<!-- MANUAL: -->
