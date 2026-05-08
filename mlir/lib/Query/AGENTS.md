<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Query

## Purpose
Implements the `mlir-query` tool's query evaluation engine. Provides a pattern-matching DSL for querying MLIR IR from the command line or programmatically — analogous to `clang-query` for Clang ASTs.

## Key Files
| File | Description |
|------|-------------|
| `Query.cpp` | `Query` base class and concrete query types (`MatchQuery`, `HelpQuery`, `QuitQuery`); implements `Query::run()` which evaluates a match expression against the IR |
| `QueryParser.cpp` | `QueryParser`: parses the query DSL text into `Query` objects (lexes keywords, matcher names, and string arguments) |
| `QueryParser.h` | Internal parser header |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Matcher/` | Matcher DSL implementations: AST-like matchers for ops, attributes, types, and values (see `Matcher/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Query DSL syntax: `match <matcher-expr>`, `help`, `quit`.
- To add a new matcher, add it in `Matcher/` and register it in `Matcher/RegistryManager.cpp`.
- The tool driver is in `mlir/lib/Tools/mlir-query/`.

### Common Patterns
- `Query::run()` iterates the IR using the matcher result and prints matching ops.
- Queries are stateless; the `QuerySession` holds state across REPL iterations.

## Dependencies

### Internal
- `mlir/lib/IR/` — IR traversal
- `mlir/lib/Query/Matcher/` — matcher implementations

### External
- `llvm/lib/Support` — `llvm::StringRef`, ADT

<!-- MANUAL: -->
