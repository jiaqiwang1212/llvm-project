<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Dialect Utils

## Purpose
Provides shared utility implementations used across multiple dialects. Contains indexing utilities, reshape op utilities, static value helpers, structured op utilities, and verification helpers that are common infrastructure rather than belonging to a specific dialect.

## Key Files
| File | Description |
|------|-------------|
| `IndexingUtils.cpp` | Utilities for indexing maps, permutations, and multi-dimensional index arithmetic |
| `ReshapeOpsUtils.cpp` | Utilities for reshape op handling: reassociation indices, collapse/expand shape logic |
| `StaticValueUtils.cpp` | Helpers for mixed static/dynamic value lists (used by subview, slice, etc.) |
| `StructuredOpsUtils.cpp` | Utilities for structured ops (linalg generics): iterator type queries, indexing map analysis |
| `VerificationUtils.cpp` | Common op verifier helper functions |

## For AI Agents

### Working In This Directory
- `StaticValueUtils.cpp` provides `dispatchIndexOpFoldResults()`, `getValueOrCreateConstantIndexOp()` — used everywhere that mixed static/dynamic offsets appear (subview, slice, extract_slice).
- `ReshapeOpsUtils.cpp` provides `ReassociationIndices` / `ReassociationExprs` for `tensor.collapse_shape` / `tensor.expand_shape`.
- `IndexingUtils.cpp` provides `invertPermutationVector()`, `applyPermutation()` used by linalg and vector transforms.
- `StructuredOpsUtils.cpp` provides `isParallelIterator()`, `isReductionIterator()` for linalg iterator type queries.
- These utilities have no dialect-specific dependencies — they are pure helpers over MLIR IR types.

### Common Patterns
- Mixed static/dynamic: `SmallVector<OpFoldResult>` holds either `IntegerAttr` (static) or `Value` (dynamic); use `dispatchIndexOpFoldResults()` to split.
- Reassociation: `getReassociationIndicesForReshape()` computes the reassociation map given src/dst shapes.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Arith`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
