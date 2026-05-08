<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/IR/

## Purpose
Core MLIR IR headers. Defines the fundamental data structures of the MLIR intermediate representation: operations, types, attributes, regions, blocks, values, affine expressions/maps, and the dialect system. Nearly every MLIR component depends on this directory. Also contains the primary TableGen base definitions (`OpBase.td`, `BuiltinOps.td`, etc.) from which all dialects derive.

## Key Files
| File | Description |
|------|-------------|
| `Operation.h` | The `Operation` class — central IR node with opcode, operands, results, regions |
| `Value.h` | `Value` base class for SSA values (block arguments and op results) |
| `Block.h` | `Block` — ordered list of operations with block arguments |
| `Region.h` | `Region` — list of blocks attached to an operation |
| `Types.h` | `Type` base class and type storage infrastructure |
| `Attributes.h` | `Attribute` base class and attribute storage infrastructure |
| `Dialect.h` | `Dialect` registration, hooks, and interfaces |
| `MLIRContext.h` | `MLIRContext` — owns all uniqued types, attributes, and dialect instances |
| `Builders.h` | `OpBuilder` / `Builder` — factory helpers for creating IR |
| `PatternMatch.h` | `RewritePattern`, `PatternRewriter` — pattern-match and rewrite API |
| `SymbolTable.h` | Symbol table for named operations with `@symbol` references |
| `Dominance.h` | Dominance and post-dominance tree computation |
| `AffineExpr.h` | Affine expression AST nodes and manipulation |
| `AffineMap.h` | Affine map representation |
| `IntegerSet.h` | Integer constraint sets for affine loop bounds |
| `Matchers.h` | Value/op pattern matchers for use in rewrites |
| `Verifier.h` | IR verifier entry point |
| `DialectRegistry.h` | `DialectRegistry` for deferred dialect registration |
| `OpDefinition.h` | `Op<>` CRTP base and op trait machinery |
| `BuiltinOps.h` | `ModuleOp`, `FuncOp`-like builtins; generated from `BuiltinOps.td` |
| `BuiltinTypes.h` | Builtin types (integer, float, index, memref, tensor, vector, etc.) |
| `BuiltinAttributes.h` | Builtin attributes (integer, float, string, array, dense, etc.) |
| `BuiltinDialect.h` | The builtin dialect declaration |
| `IRMapping.h` | `IRMapping` — maps old values/blocks/ops to cloned equivalents |
| `TypeUtilities.h` | Helpers for type checking and extraction |
| `Visitors.h` | `walk()` and visitor utilities for IR traversal |
| `OpBase.td` | Master TableGen base file: op, type, attr, interface definitions |
| `BuiltinOps.td` | TableGen definitions for builtin ops |
| `BuiltinTypes.td` | TableGen definitions for builtin types |
| `BuiltinAttributes.td` | TableGen definitions for builtin attributes |
| `AttrTypeBase.td` | Base TableGen classes for attribute/type definitions |
| `Interfaces.td` | Core interface TableGen base classes |
| `Traits.td` | TableGen op trait definitions |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Changes here have the highest blast radius in MLIR — most components include these headers.
- When modifying `.td` files, regenerate `.h.inc`/`.cpp.inc` files via `mlir-tblgen` and update CMakeLists.
- `OpBase.td` is the TableGen root; additions there affect every dialect's generated code.
- New builtin types/attributes must be added to both the `.td` file and the corresponding `.h`.
- Avoid adding heavy dependencies to these headers; they are included by virtually everything.

### Common Patterns
- Op definitions use `def MyOp : Op<MyDialect, "my_op", [Trait1, Trait2]>` in `.td` files.
- Type/attribute storage classes use `TypeStorage`/`AttributeStorage` with a `KeyTy`.
- `OpBuilder` is the standard way to create ops; `rewriter.replaceOpWithNewOp<>()` for rewrites.
- Traits are applied as template arguments to `Op<>` and define verifier/folder behaviors.

## Dependencies

### Internal
- `mlir/Support/` (TypeID, LogicalResult, StorageUniquer, LLVM.h)

### External
- `llvm/ADT/` (SmallVector, StringRef, ArrayRef, DenseMap, etc.)
- `llvm/Support/` (raw_ostream, SourceMgr, etc.)

<!-- MANUAL: -->
