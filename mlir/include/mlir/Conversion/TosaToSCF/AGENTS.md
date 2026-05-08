<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToSCF

## Purpose
Declares the TOSA to SCF lowering pass. Lowers TOSA control flow ops (if, while) to SCF dialect structured if/while constructs.

## Key Files
| File | Description |
|------|-------------|
| `TosaToSCF.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `TosaToSCF.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/TosaToSCF/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Tosa/`
- Target dialect: `include/mlir/Dialect/SCF/`

<!-- MANUAL: -->
