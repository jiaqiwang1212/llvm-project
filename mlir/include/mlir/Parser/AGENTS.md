<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Parser/

## Purpose
High-level entry point for parsing MLIR text format. Provides the `parseSourceFile` and `parseSourceString` functions that convert MLIR assembly text into in-memory IR. This is a thin public API header; the actual parsing machinery lives in `mlir/lib/Parser/` and the lower-level assembly parser interface is in `mlir/AsmParser/`.

## Key Files
| File | Description |
|------|-------------|
| `Parser.h` | `parseSourceFile<OpTy>()`, `parseSourceString<OpTy>()`, `parseAttribute()`, `parseType()` entry points |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- This directory contains only one header; prefer including `mlir/Parser/Parser.h` for all high-level parse needs.
- For lower-level assembly parsing (custom op parsers, code completion), use `mlir/AsmParser/AsmParser.h` instead.
- Template parameter to `parseSourceFile<ModuleOp>()` specifies the expected top-level op type.

### Common Patterns
- `auto module = parseSourceFile<ModuleOp>("file.mlir", &context);`
- `auto module = parseSourceString<ModuleOp>(irString, &context);`
- Error diagnostics are routed through the `MLIRContext`'s diagnostic engine.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, OwningOpRef)
- `mlir/AsmParser/` (underlying parser implementation interface)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/SourceMgr.h` (source file management)

<!-- MANUAL: -->
