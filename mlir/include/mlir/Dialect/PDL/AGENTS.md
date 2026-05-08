<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDL Dialect

## Purpose
Pattern Description Language dialect. Provides a DSL for writing rewrite patterns directly in MLIR IR: op matching, constraint checking, and rewriting expressed as `pdl.pattern` ops. Patterns are compiled to PDLInterp bytecode for efficient execution.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, and dialect definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- PDL patterns compile to PDLInterp ops via the PDLToPDLInterp conversion
- Use PDL when patterns are too complex to express with C++ RewritePatterns directly

### Common Patterns
- `pdl.pattern` wraps `pdl.operation` (matcher) + `pdl.rewrite` (replacement)
- Types are `pdl.type`, `pdl.value`, `pdl.operation`, `pdl.attribute`

## Dependencies
- Depends on: MLIR IR core, PDLInterp dialect (after compilation)

<!-- MANUAL: -->
