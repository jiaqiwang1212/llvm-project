<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Interfaces/Utils

## Purpose
Shared utility implementations used by multiple interface implementations in `mlir/lib/Interfaces/`. Currently provides helpers for integer range inference and memory-slot analysis.

## Key Files
| File | Description |
|------|-------------|
| `InferIntRangeCommon.cpp` | Arithmetic and bitwise integer-range transfer functions shared by `InferIntRangeInterface` implementations across dialects (Arith, Index, etc.) |
| `MemorySlotUtils.cpp` | Helper utilities for Mem2Reg and SROA passes: slot promotion feasibility checks, use-def traversal for memory slots |

## For AI Agents

### Working In This Directory
- `InferIntRangeCommon.cpp` is used by both the `Arith` and `Index` dialect interface implementations; changes must remain consistent with both.
- `MemorySlotUtils.cpp` underpins the `Mem2Reg` pass in `mlir/lib/Transforms/`; API changes here require updating callers there.

### Common Patterns
- Functions here are free functions or methods on plain structs, not interface methods themselves.
- Integer range arithmetic uses `mlir::ConstantIntRanges` from the `InferIntRangeInterface` header.

## Dependencies

### Internal
- `mlir/lib/Interfaces/InferIntRangeInterface.cpp`
- `mlir/lib/Interfaces/MemorySlotInterfaces.cpp`
- `mlir/lib/IR/` — core IR types

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
