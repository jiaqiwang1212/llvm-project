<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDLInterp Dialect

## Purpose
Implements the PDL Interpreter dialect — the compilation target for PDL patterns. PDLInterp ops form a bytecode for a decision-tree pattern-matching virtual machine. PDL patterns are compiled into PDLInterp programs that efficiently match and rewrite MLIR IR at runtime.

## Key Files
| File | Description |
|------|-------------|
| `IR/PDLInterp.cpp` | Dialect registration and all interpreter op implementations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Interpreter bytecode ops |

## For AI Agents

### Working In This Directory
- PDLInterp is a compilation target only — do not write PDLInterp ops by hand; generate them from PDL via `PDLToPDLInterp`.
- The VM has two op categories: matchers (check, get_attribute, get_operand, is_not_null, etc.) and rewriters (create_operation, replace, erase, etc.).
- `pdl_interp.func` defines a matcher/rewriter function; control flow uses `pdl_interp.branch` and `pdl_interp.goto_rewriter`.
- The interpreter is executed by `PatternApplicator` which calls into the generated PDLInterp program.

### Common Patterns
- Generated code: always produced by `PDLToPDLInterp`; the decision tree structure minimizes redundant checks.
- External functions: `pdl_interp.call_constraint` / `pdl_interp.call_rewrite` invoke registered C++ callbacks.

## Dependencies
- `mlir/IR`, `mlir/Dialect/PDL`, `mlir/Rewrite/PatternApplicator`

<!-- MANUAL: -->
