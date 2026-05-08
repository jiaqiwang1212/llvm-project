<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToStandard

## Purpose
Declares the Complex to Standard lowering pass. Lowers Complex dialect ops to standard Arith and Math dialect ops by expanding complex arithmetic into scalar real/imaginary components.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToStandard.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ComplexToStandard.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ComplexToStandard/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Target dialects: `include/mlir/Dialect/Arith/`, `include/mlir/Dialect/Math/`
- Shared utilities: `ComplexCommon/`

<!-- MANUAL: -->
