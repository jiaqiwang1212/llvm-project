<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToControlFlow

## Purpose
Declares the SCF to ControlFlow lowering pass. Lowers SCF dialect structured loops and conditionals (for, while, if) to unstructured cf dialect branch ops.

## Key Files
| File | Description |
|------|-------------|
| `SCFToControlFlow.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `SCFToControlFlow.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/SCFToControlFlow/`

## Dependencies
- Source dialect: `include/mlir/Dialect/SCF/`
- Target dialect: `include/mlir/Dialect/ControlFlow/`

<!-- MANUAL: -->
