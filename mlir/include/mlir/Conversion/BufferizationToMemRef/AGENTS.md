<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# BufferizationToMemRef

## Purpose
Declares the Bufferization to MemRef lowering pass. Lowers bufferization dialect ops (alloc_tensor, dealloc) to MemRef allocation and deallocation ops.

## Key Files
| File | Description |
|------|-------------|
| `BufferizationToMemRef.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `BufferizationToMemRef.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/BufferizationToMemRef/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Bufferization/`
- Target dialect: `include/mlir/Dialect/MemRef/`

<!-- MANUAL: -->
