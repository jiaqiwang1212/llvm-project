<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor Dialect

## Purpose
Provides tensor operations (extract_slice, insert_slice, reshape, pad, concatenate, empty, etc.) for manipulating immutable tensor values. Represents tensor-level computations before bufferization to MemRef.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Extensions/` | Sharding and other dialect extensions (see `Extensions/AGENTS.md`) |
| `IR/` | Op definitions and interface impls (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Tensor utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `tensor.empty` creates an uninitialized tensor (buffer allocation at tensor level)
- `tensor.extract_slice` / `tensor.insert_slice` implement subset extraction/insertion
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names follow `tensor.*` convention
- Implements `TilingInterface`, `SubsetInsertionOpInterface` on relevant ops

## Dependencies
- Depends on: Arith dialect, MemRef dialect (post-bufferization)

<!-- MANUAL: -->
