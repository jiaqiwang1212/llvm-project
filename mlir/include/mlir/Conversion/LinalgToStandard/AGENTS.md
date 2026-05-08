<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LinalgToStandard

## Purpose
Declares the Linalg to Standard lowering pass. Lowers Linalg named ops and library calls to standard Func dialect calls into the Linalg runtime library.

## Key Files
| File | Description |
|------|-------------|
| `LinalgToStandard.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `LinalgToStandard.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/LinalgToStandard/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Linalg/`
- Target dialect: `include/mlir/Dialect/Func/`

<!-- MANUAL: -->
