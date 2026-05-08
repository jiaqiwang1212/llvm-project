<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# BufferizationToMemRef Conversion

## Purpose
Lowers Bufferization dialect ops to MemRef dialect ops. Primarily converts `bufferization.dealloc` to `memref.dealloc` sequences, handling the ownership-based deallocation model's conditional free logic.

## Key Files
| File | Description |
|------|-------------|
| `BufferizationToMemRef.cpp` | Patterns converting bufferization dealloc ops to memref dealloc |

## For AI Agents

### Working In This Directory
- `bufferization.dealloc` carries a list of buffers and conditions (ownership flags) — it is lowered to a series of conditional `memref.dealloc` ops.
- `bufferization.to_tensor` / `bufferization.to_memref` are also lowered here if they remain after bufferization.
- This conversion runs after One-Shot Bufferization and the ownership-based deallocation pass.

## Dependencies
- Source: `mlir/Dialect/Bufferization`
- Target: `mlir/Dialect/MemRef`, `mlir/Dialect/Arith`, `mlir/Dialect/SCF`

<!-- MANUAL: -->
