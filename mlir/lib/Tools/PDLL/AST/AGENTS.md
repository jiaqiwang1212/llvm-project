<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/PDLL/AST

## Purpose
Defines the PDLL Abstract Syntax Tree node hierarchy, the PDLL type system, and diagnostic utilities for the PDLL compiler. This is the intermediate representation used between parsing and code generation.

## Key Files
| File | Description |
|------|-------------|
| `Nodes.cpp` | All AST node implementations: `PatternDecl`, `VariableDecl`, `OpExpr`, `TypeConstraintDecl`, `ReplaceStmt`, etc. |
| `Types.cpp` | PDLL type system: `OpType`, `ValueType`, `TypeType`, `AttributeType`, `ConstraintType`, and type compatibility rules |
| `Context.cpp` | `ASTContext`: owns all AST node allocations (bump-pointer allocator) and type uniquing tables |
| `Diagnostic.cpp` | PDLL-specific diagnostics: wraps `llvm::SMDiagnostic` with PDLL source locations |
| `NodePrinter.cpp` | AST pretty-printer for debugging and the LSP hover display |

## For AI Agents

### Working In This Directory
- All AST nodes are allocated from `ASTContext`'s bump allocator; never `new`/`delete` nodes directly.
- Adding a new AST node: add the class in `Nodes.cpp` (and its header), add a `visit()` case to the visitor, update `NodePrinter.cpp`.

### Common Patterns
- AST nodes use `static T* create(ASTContext&, ...)` factory methods.
- `ASTContext` is passed by reference throughout the compiler pipeline.

## Dependencies

### Internal
- No mlir/lib/ runtime deps; self-contained AST library

### External
- `llvm/lib/Support` — `llvm::SMLoc`, `llvm::BumpPtrAllocator`, ADT

<!-- MANUAL: -->
