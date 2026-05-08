<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRef Dialect

## Purpose
Provides operations for working with memory references (buffers): allocation/deallocation, loads, stores, casts, reshapes, subviews, copies, and atomic operations on `memref<...>` types.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and interface definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | MemRef utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `memref.alloc`/`memref.dealloc` are the standard buffer allocation ops
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- `memref.subview` creates a strided subview with static/dynamic offsets, sizes, strides
- `memref.cast` / `memref.reinterpret_cast` handle type-compatible memref conversions
- Op names follow `memref.*` convention

## Dependencies
- Depends on: Arith dialect (for index computation), LLVMIR dialect (for lowering)

<!-- MANUAL: -->
