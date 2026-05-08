<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToOpenMP

## Purpose
Declares the SCF to OpenMP lowering pass. Maps SCF parallel loop constructs to OpenMP dialect parallel region and loop ops for CPU multi-threaded execution.

## Key Files
| File | Description |
|------|-------------|
| `SCFToOpenMP.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `SCFToOpenMP.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/SCFToOpenMP/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SCF/`
- Target dialect: `include/mlir/Dialect/OpenMP/`

<!-- MANUAL: -->
