<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDLToPDLInterp

## Purpose
Declares the PDL to PDLInterp lowering pass. Compiles PDL (Pattern Description Language) rewrite rules into PDLInterp bytecode ops for efficient pattern matching execution.

## Key Files
| File | Description |
|------|-------------|
| `PDLToPDLInterp.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `PDLToPDLInterp.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/PDLToPDLInterp/`

## Dependencies
- Source dialect: `include/mlir/Dialect/PDL/`
- Target dialect: `include/mlir/Dialect/PDLInterp/`

<!-- MANUAL: -->
