<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor Dialect

## Purpose
Implements the Tensor dialect — MLIR's primary immutable tensor abstraction. Provides ops for extracting/inserting slices (`tensor.extract_slice`, `tensor.insert_slice`), reshaping, concatenating, padding, and constructing tensors. Forms the foundation for linalg and bufferization pipelines.

## Key Files
| File | Description |
|------|-------------|
| `IR/TensorOps.cpp` | All tensor op implementations: `tensor.empty`, `tensor.extract_slice`, `tensor.insert_slice`, `tensor.reshape`, `tensor.pad`, `tensor.concat`, `tensor.unpack`, `tensor.pack` |
| `IR/TensorDialect.cpp` | Dialect registration |
| `IR/TensorTilingInterfaceImpl.cpp` | `TilingInterface` implementations for tensor ops |
| `IR/TensorInferTypeOpInterfaceImpl.cpp` | Type inference for tensor ops |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds for tensor dimension ops |
| `Transforms/BufferizableOpInterfaceImpl.cpp` | Bufferization for all tensor ops |
| `Transforms/EmptyOpPatterns.cpp` | Patterns for folding `tensor.empty` through reshapes |
| `Transforms/FoldTensorSubsetOps.cpp` | Folds chains of slice/insert ops |
| `Transforms/ReshapePatterns.cpp` | Reshape canonicalization and folding |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core tensor ops, tiling/type inference interfaces |
| `Transforms/` | Bufferization, reshape folding, concat patterns, subset folding |
| `Extensions/` | External `BufferizableOpInterface` implementations |
| `TransformOps/` | Transform dialect extension for tensor transformations |
| `Utils/` | Shared utilities for slice/offset arithmetic |

## For AI Agents

### Working In This Directory
- `tensor.empty` creates an uninitialized tensor — it carries shape but no buffer; use it to initialize linalg destination-passing style ops.
- `tensor.extract_slice` / `tensor.insert_slice` use mixed static/dynamic offsets, sizes, strides — same pattern as `memref.subview`.
- `tensor.pack` / `tensor.unpack` implement tiled/blocked tensor layouts (NHWC → NHWCnhwc blocking) for cache optimization.
- Bufferization: most tensor ops have straightforward buffer mappings in `BufferizableOpInterfaceImpl.cpp`.
- `FoldTensorSubsetOps.cpp` is critical for performance — folds `extract_slice(insert_slice(...))` chains.

### Common Patterns
- Builders: `tensor::EmptyOp::create(loc, resultType, dynamicSizes)`.
- Tiling: implement `TilingInterface` to participate in `SCF` tiling; `TensorTilingInterfaceImpl` provides this for tensor ops.
- Reshape folding: `ReshapePatterns` uses `ReassociationIndices` to compose/decompose reshape chains.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/MemRef`, `mlir/Dialect/Linalg`, `mlir/Dialect/Bufferization`

<!-- MANUAL: -->
