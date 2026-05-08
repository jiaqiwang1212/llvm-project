<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToROCDLLibraryCalls

## Purpose
Declares the Complex to ROCDL library calls lowering pass. Lowers Complex dialect math ops to AMD ROCm device library function calls for GPU execution.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToROCDLLibraryCalls.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ComplexToROCDLLibraryCalls.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ComplexToROCDLLibraryCalls/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Target: ROCDL library calls via `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
