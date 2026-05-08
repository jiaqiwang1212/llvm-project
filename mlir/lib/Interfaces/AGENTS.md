<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Interfaces

## Purpose
Implements MLIR's built-in op/type/attribute interfaces. Interfaces are abstract protocols (similar to virtual dispatch) that operations, types, and attributes can conform to, enabling dialect-independent analyses and transformations. Each `.cpp` file here provides the out-of-line method bodies and any non-trivial default implementations declared in the corresponding TableGen-generated headers.

## Key Files
| File | Description |
|------|-------------|
| `CallInterfaces.cpp` | `CallOpInterface` and `CallableOpInterface`: abstractions over call/return ops and callable regions |
| `CastInterfaces.cpp` | `CastOpInterface`: cast-like operations that fold away when source/dest types match |
| `ControlFlowInterfaces.cpp` | `BranchOpInterface`, `RegionBranchOpInterface`, `RegionBranchTerminatorOpInterface`: CFG and region control flow |
| `DataLayoutInterfaces.cpp` | `DataLayoutOpInterface`, `DataLayoutTypeInterface`: target data layout queries (sizes, alignments, endianness) |
| `DerivedAttributeOpInterface.cpp` | `DerivedAttributeOpInterface`: attributes derived from op properties at print time |
| `DestinationStyleOpInterface.cpp` | `DestinationStyleOpInterface`: ops that write into pre-allocated output buffers (used by Linalg-style ops) |
| `FunctionImplementation.cpp` | Shared utilities for implementing function-like ops (argument/result parsing, printing) |
| `FunctionInterfaces.cpp` | `FunctionOpInterface`: abstractions over function-like ops (symbol, type, body region) |
| `IndexingMapOpInterface.cpp` | `IndexingMapOpInterface`: ops that expose affine indexing maps (e.g., Linalg generics) |
| `InferIntRangeInterface.cpp` | `InferIntRangeInterface`: propagate integer value ranges through ops |
| `InferStridedMetadataInterface.cpp` | `InferStridedMetadataInterface`: infer memref strides/offsets/sizes from operands |
| `InferTypeOpInterface.cpp` | `InferTypeOpInterface`, `ReifyRankedShapedTypeOpInterface`: result-type inference for ops |
| `LoopLikeInterface.cpp` | `LoopLikeOpInterface`: loop abstraction (bounds, step, induction variable, iter-args) |
| `MemOpInterfaces.cpp` | `MemoryReadOpInterface`, `MemoryWriteOpInterface`: memory effect side-channel |
| `MemorySlotInterfaces.cpp` | `MemorySlot` and related interfaces for Mem2Reg/SROA transformations |
| `ParallelCombiningOpInterface.cpp` | `ParallelCombiningOpInterface`: ops that combine parallel sub-computations |
| `RuntimeVerifiableOpInterface.cpp` | `RuntimeVerifiableOpInterface`: generate runtime assertion ops for dynamic invariants |
| `ShapedOpInterfaces.cpp` | Shared utilities for ops operating on shaped types |
| `SideEffectInterfaces.cpp` | `MemoryEffectOpInterface`, effect sets, resource/effect modeling |
| `SubsetOpInterface.cpp` | `SubsetOpInterface` / `SubsetExtractionOpInterface`: ops selecting subsets of tensors/buffers |
| `TilingInterface.cpp` | `TilingInterface`: ops that can be tiled, fused, and padded (used by structured op tiling) |
| `ValueBoundsOpInterface.cpp` | `ValueBoundsOpInterface`: query upper/lower bounds of SSA values |
| `VectorInterfaces.cpp` | `VectorUnrollOpInterface`: ops that can be unrolled over vector dimensions |
| `ViewLikeInterface.cpp` | `ViewLikeOpInterface`, `OffsetSizeAndStrideOpInterface`: memref view/subview ops |
| `AlignmentAttrInterface.cpp` | `AlignmentAttrInterface`: attributes that carry alignment information |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Utils/` | Shared utility implementations used by multiple interface files (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Interface `.cpp` files include generated `.inc` files from `mlir/include/mlir/Interfaces/` (e.g., `CallInterfaces.cpp.inc`). Run the corresponding `*IncGen` CMake target before editing.
- Default method implementations in `.cpp` files must be consistent with the contract described in the corresponding `.td` file in `mlir/include/mlir/Interfaces/`.
- When adding a new interface, add the `.td` file to `mlir/include/mlir/Interfaces/`, generate the `.h`/`.cpp` inc files, and add the implementation `.cpp` here.

### Common Patterns
- Interfaces are declared with `def MyInterface : OpInterface<"MyInterface">` in `.td`.
- Out-of-line implementations go in `.cpp` files named after the interface group.
- `SideEffectInterfaces.cpp` uses the `EffectOpInterface` model; new memory resources are registered as subtypes of `Resource`.

## Dependencies

### Internal
- `mlir/lib/IR/` — `Operation`, `Value`, `Type`, `Attribute` base classes
- `mlir/include/mlir/Interfaces/` — TableGen-generated interface headers

### External
- `llvm/lib/Support` — ADT, `llvm::DenseMap`

<!-- MANUAL: -->
