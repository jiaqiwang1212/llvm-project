<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/PDLL/

## Purpose
PDLL (Pattern Description Language) tooling headers. PDLL is a domain-specific language for writing MLIR rewrite patterns that compiles to either C++ pattern code or PDL dialect IR. This directory contains the complete frontend: AST, parser, code generators, and ODS (Operation Definition Spec) integration.

## Key Files
(none at top level — all content is in subdirectories)

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AST/` | PDLL Abstract Syntax Tree node types (see `AST/AGENTS.md`) |
| `Parser/` | PDLL parser and code-completion (see `Parser/AGENTS.md`) |
| `CodeGen/` | PDLL code generators: C++ and MLIR (PDL dialect) backends (see `CodeGen/AGENTS.md`) |
| `ODS/` | ODS (TableGen op definitions) integration for PDLL type checking (see `ODS/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- PDLL source files (`.pdll`) are parsed to the AST, then either compiled to PDL IR or C++ patterns.
- The LSP server for PDLL (`mlir-pdll-lsp-server`) uses the parser with code-completion enabled.
- ODS integration allows PDLL to understand op argument/result types from TableGen definitions.

### Common Patterns
- PDLL compilation pipeline: source → `Parser` → `AST` → `CodeGen` (CPPGen or MLIRGen).
- ODS context is populated from `.td` files before parsing to enable type checking.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext for MLIR code generation)
- `mlir/Dialect/PDL/` (target dialect for MLIR codegen)

### External
- `llvm/Support/SourceMgr.h`

<!-- MANUAL: -->
