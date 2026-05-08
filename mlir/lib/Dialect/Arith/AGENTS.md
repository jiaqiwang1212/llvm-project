<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Arith Dialect

## Purpose
Implements the Arith dialect — the core arithmetic and integer/float operation set for MLIR. Provides integer and floating-point arithmetic, bitwise operations, comparison, casting, and constant ops that are the building blocks for higher-level dialects.

## Key Files
| File | Description |
|------|-------------|
| `IR/ArithDialect.cpp` | Dialect registration |
| `IR/ArithOps.cpp` | All op implementations: `arith.addi`, `arith.mulf`, `arith.cmpi`, `arith.constant`, etc. |
| `IR/ArithCanonicalization.td` | TableGen canonicalization patterns (constant folding, algebraic simplifications) |
| `IR/InferIntRangeInterfaceImpls.cpp` | Integer range inference implementations for all arith ops |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds interface implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core ops, canonicalization patterns, interface implementations |
| `Transforms/` | Type emulation (narrow int, wide int, float), int range optimizations, bufferization |
| `Utils/` | Shared utility functions |

## For AI Agents

### Working In This Directory
- `ArithCanonicalization.td` is tablegen-driven — pattern changes there regenerate `.inc` files; matching `.cpp` includes them.
- `InferIntRangeInterfaceImpls.cpp` must be updated whenever new integer ops are added so integer range analysis remains complete.
- `EmulateNarrowType.cpp` and `EmulateWideInt.cpp` in Transforms handle hw that lacks native i1/i2 or i128 support.
- `BufferizableOpInterfaceImpl.cpp` is needed because `arith.constant` can produce tensors.

### Common Patterns
- Constant folding: implement `fold()` on the op class, returning a new attribute.
- Canonicalizers: in `ArithCanonicalization.td` using `Pat` records.
- Integer range: implement `inferResultRanges()` to participate in `IntRangeAnalysis`.

## Dependencies
- `mlir/IR`, `mlir/Interfaces/SideEffectInterfaces`, `mlir/Interfaces/InferIntRangeInterface`

<!-- MANUAL: -->
