<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine Dialect

## Purpose
Implements the Affine dialect, which provides a polyhedral abstraction over loops and memory accesses. Affine maps and integer set constraints are used to express loop bounds, memory subscripts, and conditions in a mathematically precise form that enables powerful analysis and transformation.

## Key Files
| File | Description |
|------|-------------|
| `IR/AffineOps.cpp` | Core op implementations: `affine.for`, `affine.if`, `affine.load`, `affine.store`, `affine.apply`, etc. |
| `IR/AffineValueMap.cpp` | AffineValueMap — binds an AffineMap to SSA values for composable affine expressions |
| `IR/AffineMemoryOpInterfaces.cpp` | Memory interface implementations for affine load/store/DMA ops |
| `IR/InferIntRangeInterfaceImpls.cpp` | Integer range inference for affine ops |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds interface implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core IR: ops, affine value maps, memory interfaces |
| `Analysis/` | Affine analysis utilities (dependence analysis, loop analysis) |
| `Transforms/` | Loop transformations: tiling, unrolling, fusion, vectorization, data copy generation |
| `TransformOps/` | Transform dialect extension ops for affine transformations |
| `Utils/` | Shared utilities for affine lowering and analysis |

## For AI Agents

### Working In This Directory
- Affine ops carry `AffineMapAttr` / `IntegerSetAttr` — verifiers enforce that operand counts match map dimensions/symbols.
- Loop bounds must be affine expressions; non-affine accesses require `memref.load`/`memref.store` instead.
- TableGen codegen produces `AffineOps.h.inc` / `AffineOps.cpp.inc`; never hand-edit those files.
- `AffineValueMap` is the canonical way to compose affine expressions with SSA values in analysis code.

### Common Patterns
- Op verifiers: `AffineForOp::verify()`, `AffineIfOp::verify()` check map arity vs. operand count.
- Canonicalizers: registered via `getCanonicalizationPatterns()`, simplify affine maps using `simplifyAffineMap`.
- Loop nest utilities live in `Analysis/` and `Utils/`; prefer them over reimplementing.

## Dependencies
- `mlir/IR`, `mlir/Analysis/Presburger` (integer set math), `mlir/Dialect/MemRef`, `mlir/Dialect/Arith`

<!-- MANUAL: -->
