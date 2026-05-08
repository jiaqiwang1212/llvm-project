<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Vector Dialect

## Purpose
Implements the Vector dialect — MLIR's primary SIMD/vector abstraction. Provides multi-dimensional fixed-width and scalable vector ops: `vector.contract`, `vector.transfer_read/write`, `vector.broadcast`, `vector.shuffle`, `vector.reduction`, `vector.transpose`, `vector.mask`, etc. Used as the intermediate between linalg vectorization and hardware-specific lowering.

## Key Files
| File | Description |
|------|-------------|
| `IR/VectorOps.cpp` | All vector op implementations and verifiers |
| `IR/ScalableValueBoundsConstraintSet.cpp` | Value bounds for scalable vector operations |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds interface implementations |
| `Transforms/VectorTransforms.cpp` | Core vector transformation infrastructure |
| `Transforms/LowerVectorContract.cpp` | Lowers `vector.contract` to `vector.outerproduct` or matrix ops |
| `Transforms/LowerVectorTransfer.cpp` | Lowers `vector.transfer_read/write` to loads/stores with masking |
| `Transforms/LowerVectorTranspose.cpp` | Lowers `vector.transpose` using shuffles |
| `Transforms/VectorUnroll.cpp` | Unrolls vector ops to smaller vector sizes |
| `Transforms/VectorDistribute.cpp` | Distributes vector ops across threads (warp-level) |
| `Transforms/LowerVectorMask.cpp` | Lowers `vector.mask` to predicated ops |
| `Transforms/VectorEmulateNarrowType.cpp` | Emulates narrow vector types (i1, i2, i4) |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Vector ops, scalable bounds |
| `Transforms/` | Comprehensive lowering of all vector ops to simpler forms |
| `TransformOps/` | Transform dialect extension for vector transformations |
| `Interfaces/` | Vector op interfaces |
| `Utils/` | Vector utility functions |

## For AI Agents

### Working In This Directory
- `vector.contract` is the key high-level op — maps to matrix multiply, dot product, or outer product depending on indexing maps and iterator types.
- `vector.transfer_read/write` abstract over masked/unmasked memory accesses with permutation maps; they lower to `vector.load`/`vector.store` or masked variants.
- Scalable vectors (`vector<[4]xi32>`) use `vscale` as a runtime multiplier; not all lowering paths support them.
- `VectorUnroll.cpp` decomposes large vectors into smaller ones matching hardware register widths.
- `VectorDistribute.cpp` rewrites vector ops into warp-lane-level ops for GPU subgroup code generation.
- `LowerVectorMask.cpp` handles `vector.mask` regions, generating either predicated instructions or select patterns.

### Common Patterns
- Contract lowering: detect `vector.contract` indexing map shape to choose outerproduct vs. dot vs. matvec path.
- Transfer legalization: `VectorTransferFullPartialRewriter` splits transfers that may go out of bounds.
- Unrolling: `UnrollVectorOptions` specifies target shape; the unroller splits and reassembles vectors.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/MemRef`, `mlir/Dialect/SCF`, `mlir/Dialect/AffineToStandard`

<!-- MANUAL: -->
