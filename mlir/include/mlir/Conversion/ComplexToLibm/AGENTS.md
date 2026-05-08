<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToLibm

## Purpose
Declares the Complex to Libm lowering pass. Lowers Complex dialect math ops to calls into the C standard math library (libm) for complex number operations.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToLibm.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ComplexToLibm.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ComplexToLibm/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Target: Func dialect calls to libm symbols

<!-- MANUAL: -->
