<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IndexToLLVM

## Purpose
Declares the Index to LLVM IR lowering pass. Lowers Index dialect ops (arithmetic on index-typed values) to LLVM dialect integer ops with pointer-width integers.

## Key Files
| File | Description |
|------|-------------|
| `IndexToLLVM.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `IndexToLLVM.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/IndexToLLVM/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Index/`
- Target dialect: `include/mlir/Dialect/LLVMIR/`

<!-- MANUAL: -->
