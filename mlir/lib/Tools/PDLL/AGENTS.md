<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/PDLL

## Purpose
Implements the PDLL (Pattern Description Language) compiler library. PDLL is a domain-specific language for expressing MLIR pattern rewrites that compiles to either PDL dialect ops (for interpretation) or C++ pattern code (for static compilation). This library provides the full PDLL compiler pipeline: lexing, parsing, semantic analysis, ODS integration, and code generation.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AST/` | PDLL Abstract Syntax Tree: node types, type system, diagnostic handling, AST printer (see `AST/AGENTS.md`) |
| `Parser/` | PDLL lexer and recursive-descent parser; produces an AST from `.pdll` source text (see `Parser/AGENTS.md`) |
| `ODS/` | ODS (Op Definition Spec) integration: loads op/dialect/constraint definitions from TableGen-generated ODS data so PDLL patterns can reference them (see `ODS/AGENTS.md`) |
| `CodeGen/` | Code generation backends: C++ pattern codegen (`CPPGen.cpp`) and PDL dialect codegen (`MLIRGen.cpp`) (see `CodeGen/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- The PDLL compiler pipeline: `Parser` → `AST` → (ODS enrichment) → `CodeGen`.
- The PDLL LSP server (`mlir/lib/Tools/mlir-pdll-lsp-server/`) uses the `Parser` and `AST` directly for language features.
- PDLL `.pdll` files are typically compiled to PDL dialect ops by `mlir-pdll`, which is then embedded in a module and interpreted by the PDL bytecode interpreter at runtime.

### Common Patterns
- `mlir::pdll::parseFile(sourceMgr, context, odsContext)` is the main entry point into the compiler.
- `CodeGen::generateCPP(ast, os)` and `CodeGen::generateMLIR(ast, context)` are the two backends.

## Dependencies

### Internal
- `mlir/lib/IR/` — `MLIRContext`, PDL dialect ops
- `mlir/lib/TableGen/` — ODS record access (for `ODS/` layer)

### External
- `llvm/lib/Support` — `llvm::SourceMgr`, ADT

<!-- MANUAL: -->
