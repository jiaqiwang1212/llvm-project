<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Interfaces/Utils/

## Purpose
Utility headers that support multiple interface implementations. Contains shared helper functions used by concrete interface implementations, such as integer range inference utilities and memory slot promotion helpers.

## Key Files
| File | Description |
|------|-------------|
| `InferIntRangeCommon.h` | Shared range inference helpers for `InferIntRangeInterface` implementations (arithmetic op range transfer functions) |
| `MemorySlotUtils.h` | Helper functions for implementing `MemorySlotInterface` (mem2reg-style slot analysis) |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `InferIntRangeCommon.h` provides transfer functions (e.g., `inferResultRanges` for add, mul, shift) used when implementing `InferIntRangeInterface` for arithmetic ops.
- `MemorySlotUtils.h` provides utilities for determining promotability of memory slots used by `MemorySlotInterface`.
- These are internal helpers; prefer using the interface API from `mlir/Interfaces/` directly.

### Common Patterns
- Range inference: `mlir::intrange::inferAdd(lhsRange, rhsRange)` returns result range.
- Memory slot: `isMemorySlotPromotable(slot, users)` checks if a slot can be mem2reg'd.

## Dependencies

### Internal
- `mlir/Interfaces/InferIntRangeInterface.h`
- `mlir/Interfaces/MemorySlotInterfaces.h`
- `mlir/IR/` (Value, Operation)

### External
- `llvm/ADT/`

<!-- MANUAL: -->
