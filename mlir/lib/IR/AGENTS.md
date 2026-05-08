<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/IR

## Purpose
Implements the core MLIR IR data structures. This is the most fundamental library in MLIR — every other subsystem depends on it. It defines the in-memory representation of operations, types, attributes, regions, blocks, values, affine expressions/maps, and the MLIRContext that owns them all.

## Key Files
| File | Description |
|------|-------------|
| `MLIRContext.cpp` | `MLIRContext` implementation: dialect registry, type/attribute storage uniquing, threading model, and global context state |
| `Operation.cpp` | `Operation` class: creation, cloning, result/operand management, successor/region accessors, generic printing |
| `OperationSupport.cpp` | `OperationName`, `OpOperand`, `OpResult`, `OpSuccessor` support classes |
| `Block.cpp` | `Block` class: argument management, predecessor/successor iteration, operation list |
| `Region.cpp` | `Region` class: block list, parent op linkage, cloning |
| `Value.cpp` | `Value` base class and `BlockArgument` implementation |
| `ValueRange.cpp` | `ValueRange` and `MutableValueRange` lightweight views over value lists |
| `TypeRange.cpp` | `TypeRange` lightweight view over type lists |
| `Types.cpp` | `Type` base class, `TypeStorage` uniquing, parametric type infrastructure |
| `Attributes.cpp` | `Attribute` base class, `AttributeStorage` uniquing |
| `BuiltinTypes.cpp` | Built-in types: integer, float, index, tensor, memref, vector, tuple, function, etc. |
| `BuiltinAttributes.cpp` | Built-in attributes: integer, float, string, array, dense elements, dictionary, etc. |
| `BuiltinAttributeInterfaces.cpp` | Interface implementations for built-in attributes (e.g., `ElementsAttr`) |
| `BuiltinTypeInterfaces.cpp` | Interface implementations for built-in types (e.g., `ShapedType`) |
| `BuiltinDialect.cpp` | Registration of the `builtin` dialect, its ops, types, and attributes |
| `BuiltinDialectBytecode.cpp` | Bytecode encoding/decoding for builtin types and attributes |
| `AffineExpr.cpp` | `AffineExpr` hierarchy: dimensions, symbols, constants, add/mul/mod/floor/ceil div |
| `AffineMap.cpp` | `AffineMap`: multi-dimensional affine function composition and simplification |
| `IntegerSet.cpp` | `IntegerSet`: conjunctions of affine constraints |
| `Diagnostics.cpp` | Diagnostic engine: `InFlightDiagnostic`, handlers, notes, source locations |
| `Location.cpp` | `Location` hierarchy: file/line/col, fused, name, call-site locations |
| `Dialect.cpp` | `Dialect` base class, dialect interface registry, op/type/attr registration |
| `ExtensibleDialect.cpp` | Dynamic dialects and dynamically-typed operations |
| `DialectResourceBlobManager.cpp` | Manages large binary blobs attached to dialect resources |
| `PatternMatch.cpp` | Pattern rewriting core: `PatternRewriter`, `RewritePattern`, match/rewrite protocol |
| `PatternLoggingListener.cpp` | Listener that logs pattern application events for debugging |
| `SymbolTable.cpp` | `SymbolTable` and `SymbolTableCollection`: symbol lookup, renaming, dead-symbol analysis |
| `Verifier.cpp` | IR verifier: structural checks, op verification dispatch, region kind checks |
| `Dominance.cpp` | Dominance and post-dominance computation over CFG regions |
| `Builders.cpp` | `OpBuilder` and `ImplicitLocOpBuilder` insertion-point management |
| `AttrTypeSubElements.cpp` | `AttrTypeWalker`/`AttrTypeReplacer` for sub-element traversal and replacement |
| `ODSSupport.cpp` | ODS (Op Definition Spec) runtime support: property encoding helpers |
| `RegionKindInterface.cpp` | Interface implementation distinguishing CFG vs. graph regions |
| `Remarks.cpp` | Diagnostic remark attachment to ops and values |
| `TensorEncoding.cpp` | Attribute verification for `tensor` encoding attributes |
| `Visitors.cpp` | Generic IR visitor utilities (`walk` implementations) |
| `Unit.cpp` | `UnitAttr` singleton implementation |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `PDL/` | Pattern Description Language match/rewrite op implementations used within the IR library (see `PDL/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Many `.cpp` files `#include` generated `.inc` files from `mlir/include/mlir/IR/` (e.g., `BuiltinOps.cpp.inc`, `BuiltinTypes.cpp.inc`). Run `cmake --build <build_dir> --target MLIRBuiltinOpsIncGen` (and similar targets) before editing.
- The `StorageUniquer` in `mlir/lib/Support/StorageUniquer.cpp` is a direct dependency for type/attribute uniquing; changes there affect this directory.
- `Operation.cpp` is performance-critical; avoid adding heap allocations to hot paths.
- `Verifier.cpp` is called on every IR transformation in debug builds; verification logic must be correct and fast.

### Common Patterns
- Types and attributes use a two-layer design: public `Type`/`Attribute` handle + private `TypeStorage`/`AttributeStorage` uniqued via `StorageUniquer`.
- New built-in types/attributes are declared in `.td` files in `mlir/include/mlir/IR/BuiltinTypes.td` / `BuiltinAttributes.td` and the generated `.inc` is included in the corresponding `.cpp`.
- Dialect-specific logic is NOT placed here; it goes under `mlir/lib/Dialect/<DialectName>/`.

### Common Patterns
- `Operation::create()` is the canonical factory; never construct `Operation` directly.
- Pattern rewriting always goes through `PatternRewriter`; direct mutation outside a rewriter is forbidden during pattern application.

## Dependencies

### Internal
- `mlir/lib/Support/` — `StorageUniquer`, `TypeID`, `Timing`
- `mlir/include/mlir/IR/` — public headers and `.inc` files

### External
- `llvm/lib/Support` — ADT, `llvm::raw_ostream`, `llvm::DenseMap`, threading primitives
- `llvm/lib/TableGen` — TableGen record model (for `.td`-driven code generation)

<!-- MANUAL: -->
