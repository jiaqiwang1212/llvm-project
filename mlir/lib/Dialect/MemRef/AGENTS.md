<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRef Dialect

## Purpose
Implements the MemRef dialect — MLIR's primary buffer abstraction. Provides ops for allocating, deallocating, loading, storing, casting, and reshaping memory references (`memref<NxMxf32>`). The dialect sits between high-level tensor ops and low-level pointer/load/store in LLVM IR.

## Key Files
| File | Description |
|------|-------------|
| `IR/MemRefOps.cpp` | All memref ops: `memref.alloc`, `memref.load`, `memref.store`, `memref.cast`, `memref.subview`, `memref.reshape`, etc. |
| `IR/MemRefDialect.cpp` | Dialect registration |
| `IR/MemRefMemorySlot.cpp` | Memory slot analysis for `memref.alloca` (enables SROA) |
| `IR/ValueBoundsOpInterfaceImpl.cpp` | Value bounds for memref dimension ops |
| `Transforms/ExpandStridedMetadata.cpp` | Expands `memref.subview` to explicit stride/offset arithmetic |
| `Transforms/FoldMemRefAliasOps.cpp` | Folds chains of subview/cast ops |
| `Transforms/NormalizeMemRefs.cpp` | Normalizes memref types to canonical (identity map) form |
| `Transforms/EmulateNarrowType.cpp` | Emulates narrow element types (i1, i2, i4) via wider types |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core ops, dialect, memory slot analysis |
| `Transforms/` | Normalization, stride expansion, alias folding, type emulation |
| `TransformOps/` | Transform dialect extension |
| `Utils/` | Shared utilities for stride/offset computation |

## For AI Agents

### Working In This Directory
- `memref.subview` carries `offsets`, `sizes`, `strides` as mixed static/dynamic operands — verifiers use `ShapedType` utilities to validate.
- `ExpandStridedMetadata.cpp` is required before `MemRefToLLVM` if subviews are present; it makes all stride/offset arithmetic explicit.
- `memref.alloca` vs `memref.alloc`: `alloca` is stack-allocated (function-scoped lifetime), `alloc` is heap-allocated (explicit dealloc needed).
- `NormalizeMemRefs.cpp` rewrites non-identity affine map memrefs to canonical form — needed by some lowering passes.
- `MemRefMemorySlot.cpp` enables mem2reg-style optimization over `memref.alloca` + `memref.load`/`memref.store` patterns.

### Common Patterns
- Builders: `MemRefType::get(shape, elementType, layout, memSpace)`.
- Stride computation: use `getStridesAndOffset()` from `mlir/Dialect/MemRef/Utils/MemRefUtils.h`.
- Type folding: `memref.cast` folds when static info is compatible with the source type.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Arith`, `mlir/Dialect/Affine` (for affine map layouts)

<!-- MANUAL: -->
