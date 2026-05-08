<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Interfaces/

## Purpose
Declarations for MLIR's built-in op, type, and attribute interfaces. Each interface is defined in a paired `.td` (TableGen) and `.h` (C++ header) file. TableGen generates the interface class boilerplate; the `.h` files provide any hand-written utilities, extra methods, and documentation. Interfaces here are dialect-independent and reused across many dialects.

## Key Files
| File | Description |
|------|-------------|
| `CallInterfaces.h` / `.td` | `CallOpInterface`, `CallableOpInterface` for call/return ops |
| `CastInterfaces.h` / `.td` | `CastOpInterface` for type cast operations |
| `ControlFlowInterfaces.h` / `.td` | `BranchOpInterface`, `RegionBranchOpInterface`, `RegionSuccessor` |
| `DataLayoutInterfaces.h` / `.td` | `DataLayoutOpInterface`, `DataLayoutTypeInterface` for target data layout |
| `DestinationStyleOpInterface.h` / `.td` | Interface for ops writing into pre-allocated output buffers |
| `FunctionInterfaces.h` / `.td` | `FunctionOpInterface` for function-like ops |
| `FunctionImplementation.h` | Helpers for implementing function-like ops |
| `InferTypeOpInterface.h` / `.td` | `InferTypeOpInterface`, `ReifyRankedShapedTypeOpExtents` |
| `InferIntRangeInterface.h` / `.td` | Interface for integer range inference |
| `InferStridedMetadataInterface.h` / `.td` | Interface for inferring memref strided metadata |
| `LoopLikeInterface.h` / `.td` | `LoopLikeOpInterface` for loop-structured ops |
| `MemOpInterfaces.h` / `.td` | `LoadOpInterface`, `StoreOpInterface` for memory ops |
| `MemorySlotInterfaces.h` / `.td` | Interfaces for mem2reg-style memory slot promotion |
| `SideEffectInterfaces.h` / `.td` | `MemoryEffectOpInterface`, effect modeling |
| `SideEffectInterfaceBase.td` | Base TableGen classes for side-effect interfaces |
| `ShapedOpInterfaces.h` / `.td` | Interfaces for shaped-type operands |
| `SubsetOpInterface.h` / `.td` | Interface for subset insertion/extraction ops |
| `TilingInterface.h` / `.td` | `TilingInterface` for ops that can be tiled |
| `ValueBoundsOpInterface.h` / `.td` | Interface for querying value bounds |
| `VectorInterfaces.h` / `.td` | Interfaces for vector ops |
| `ViewLikeInterface.h` / `.td` | `ViewLikeOpInterface` for memref view ops |
| `RuntimeVerifiableOpInterface.h` / `.td` | Interface for runtime assertion insertion |
| `IndexingMapOpInterface.h` / `.td` | Interface for ops with indexing maps (used in Linalg) |
| `ParallelCombiningOpInterface.h` / `.td` | Interface for ops combining parallel results |
| `AlignmentAttrInterface.h` / `.td` | Interface for attributes carrying alignment info |
| `DerivedAttributeOpInterface.h` / `.td` | Interface for ops with derived attributes |
| `FoldInterfaces.h` | `DialectFoldInterface` for dialect-level folding hooks |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Utils/` | Helper utilities for interface implementations (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Each interface requires both a `.td` definition and typically a `.h` with `#include "mlir/Interfaces/FooInterface.h.inc"`.
- Register interfaces with their dialect via `addInterfaces<>()` in the dialect constructor.
- When adding a new interface, also update `CMakeLists.txt` to add a `mlir_tablegen()` call.
- Interface methods that are not pure-virtual should provide a default implementation in the `.td` extraClassDeclaration.

### Common Patterns
- TableGen: `def MyInterface : OpInterface<"MyInterface"> { let methods = [...]; }`
- C++ usage: `if (auto iface = op->dyn_cast<MyInterface>()) iface.myMethod();`
- Implementations use `DeclareOpInterfaceMethods<MyInterface>` in op definitions.

## Dependencies

### Internal
- `mlir/IR/` (Operation, Type, Attribute base classes)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (SmallVector, StringRef)

<!-- MANUAL: -->
