<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg Dialect

## Purpose
Implements the Linalg dialect — MLIR's structured linear algebra abstraction. Provides named ops (matmul, conv, pooling) and the generic `linalg.generic` op with indexing maps, enabling tiling, fusion, vectorization, and bufferization of tensor and memref computations.

## Key Files
| File | Description |
|------|-------------|
| `IR/LinalgDialect.cpp` | Dialect registration and interface population |
| `IR/LinalgOps.cpp` | All named ops (matmul, batch_matmul, conv_*, pooling_*) and `linalg.generic` |
| `IR/LinalgInterfaces.cpp` | `LinalgOp`, `TilingInterface`, `ContractionOpInterface` implementations |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds for linalg op iteration spaces |
| `Transforms/Tiling.cpp` | Loop tiling using `TilingInterface` |
| `Transforms/Vectorization.cpp` | Vectorizes linalg ops to `vector.contract` and transfer ops |
| `Transforms/Fusion.cpp` | Fuses producer-consumer linalg op pairs |
| `Transforms/BufferizableOpInterfaceImpl.cpp` | Bufferization support for tensor linalg ops |
| `Transforms/Loops.cpp` | Lowers linalg generics to loop nests |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Ops, interfaces, indexing map infrastructure |
| `Transforms/` | Tiling, fusion, vectorization, bufferization, specialization |
| `TransformOps/` | Transform dialect extension for linalg (tile, fuse, vectorize, etc.) |
| `Utils/` | Shared utilities for indexing map manipulation and loop generation |

## For AI Agents

### Working In This Directory
- `linalg.generic` is defined by indexing maps (one per operand) and an iterator type list (parallel/reduction).
- Named ops (e.g., `linalg.matmul`) are sugar over `linalg.generic` with fixed indexing maps; they can be generalized via `Generalization.cpp`.
- Tiling produces `scf.for`/`scf.forall` loops wrapping a smaller linalg op; the `TilingInterface` drives this.
- Vectorization detects contraction patterns and generates `vector.contract`; then `vector.*` lowering handles the rest.
- `ElementwiseOpFusion.cpp` fuses elementwise producers into consumer linalg ops when no data reuse is lost.
- `ConvertConv2DToImg2Col.cpp` rewrites conv2d as a matmul after im2col transformation.

### Common Patterns
- Adding a new named op: define in TableGen with `Linalg_StructuredOp_Base`, specify indexing maps and iterator types.
- `LinalgOp::getIndexingMapsArray()` returns the affine maps binding operands to loop indices.
- Fusion: `fuseProducerOfTensor()` identifies a producer op for a given tensor operand and fuses it.

## Dependencies
- `mlir/Dialect/Affine`, `mlir/Dialect/Arith`, `mlir/Dialect/MemRef`, `mlir/Dialect/Tensor`, `mlir/Dialect/Vector`, `mlir/Dialect/SCF`

<!-- MANUAL: -->
