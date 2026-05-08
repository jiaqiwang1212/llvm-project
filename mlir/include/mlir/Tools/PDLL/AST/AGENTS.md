<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/PDLL/AST/

## Purpose
PDLL Abstract Syntax Tree node definitions. Contains the full type hierarchy for the PDLL AST including all expression, statement, declaration, and constraint node types, plus the AST context and diagnostic infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `Nodes.h` | All PDLL AST node types: `PatternDecl`, `OpExpr`, `ConstraintDecl`, `ReplaceStmt`, etc. |
| `Types.h` | PDLL type system: `OperationType`, `ValueType`, `TypeType`, `AttributeType`, etc. |
| `Context.h` | `Context` — owns and allocates all AST nodes (arena allocator) |
| `Diagnostic.h` | PDLL-specific diagnostic emission into the AST context |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- All AST nodes are arena-allocated through `Context`; do not `delete` them manually.
- `Nodes.h` uses LLVM RTTI (`isa<>`, `cast<>`, `dyn_cast<>`) for node type dispatch.
- Types in `Types.h` are PDLL source-level types, not MLIR IR types.
- Diagnostics attach to source locations from the `SourceMgr`.

### Common Patterns
- Node creation: `PatternDecl::create(ctx, loc, name, body)`.
- Type check: `if (auto *opType = dyn_cast<OperationType>(type)) { ... }`.

## Dependencies

### Internal
- `mlir/Support/` (LLVM.h for RTTI)

### External
- `llvm/Support/SourceMgr.h` (source locations)
- `llvm/ADT/` (StringRef, SmallVector)

<!-- MANUAL: -->
