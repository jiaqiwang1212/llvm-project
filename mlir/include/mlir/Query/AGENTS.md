<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Query/

## Purpose
Headers for the `mlir-query` tool, which provides a Clang-query-style interface for inspecting MLIR IR. Defines the `Query` and `QuerySession` types that represent parsed queries and their execution context, plus a matcher framework for matching operations by structural and type criteria.

## Key Files
| File | Description |
|------|-------------|
| `Query.h` | `Query` — a parsed query object with kind, matcher, and output options |
| `QuerySession.h` | `QuerySession` — holds context (module, output stream) for query execution |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Matcher/` | Matcher framework for operation matching (see `Matcher/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- This is the public API for `mlir-query`; the tool main is in `mlir/Tools/mlir-query/`.
- Queries are parsed from command-line strings and matched against a loaded MLIR module.
- The matcher system mirrors Clang's AST matcher design (typed matchers, combinators).

### Common Patterns
- `QuerySession` holds the root module and drives query execution.
- Matcher combinators: `hasOpName("func.func")`, `isPublic()`, combined with `allOf()`/`anyOf()`.

## Dependencies

### Internal
- `mlir/IR/` (Operation, MLIRContext)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (StringRef)
- `llvm/Support/` (raw_ostream)

<!-- MANUAL: -->
