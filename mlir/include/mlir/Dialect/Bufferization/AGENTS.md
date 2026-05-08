<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization Dialect

## Purpose
Provides the One-Shot Bufferization framework and supporting ops/interfaces for converting tensor-level IR to buffer (memref) IR. Central to the tensor-to-memref lowering pipeline in MLIR.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Extensions/` | Sharding and other dialect extensions (see `Extensions/AGENTS.md`) |
| `IR/` | Op and interface definitions (see `IR/AGENTS.md`) |
| `Pipelines/` | Pre-built bufferization pipeline passes (see `Pipelines/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Bufferization analysis and pass declarations (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- One-Shot Bufferization analyses all ops at once; register `BufferizableOpInterface` for new ops
- Ops must implement `BufferizableOpInterface` to participate in bufferization

### Common Patterns
- `bufferization.alloc_tensor` is the canonical way to allocate tensor storage
- `bufferization.to_memref` / `bufferization.to_tensor` bridge tensor and buffer worlds

## Dependencies
- Depends on: MemRef dialect, Tensor dialect, Func dialect

<!-- MANUAL: -->
