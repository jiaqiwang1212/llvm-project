<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Query/Matcher

## Purpose
Implements the matcher DSL for `mlir-query`. Provides composable matcher objects that test predicates on MLIR operations, attributes, types, and values — modelled after `clang-query`'s AST matcher infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `MatchFinder.cpp` | `MatchFinder`: walks the IR and collects all ops satisfying a given matcher expression |
| `MatchersInternal.cpp` | Internal matcher combinators: `allOf`, `anyOf`, `unless`, `hasOperand`, etc. |
| `VariantValue.cpp` | `VariantValue`: a tagged union holding a matcher, string, integer, or other value as parsed from the DSL |
| `RegistryManager.cpp` | `RegistryManager`: maps matcher names (strings) to matcher factory functions; the lookup table powering the query parser |
| `Diagnostics.cpp` | Error diagnostics for invalid matcher expressions |
| `ErrorBuilder.cpp` | Utilities for building structured matcher error messages |
| `Parser.cpp` | Low-level matcher expression parser (sub-parser called from `mlir/lib/Query/QueryParser.cpp`) |
| `Parser.h` / `RegistryManager.h` / `Diagnostics.h` | Internal headers |

## For AI Agents

### Working In This Directory
- To add a new matcher, implement a factory function in `MatchersInternal.cpp` and register it in `RegistryManager.cpp` with a string name.
- `VariantValue` must be updated if new matcher argument types are needed.
- Modelled closely after the `clang/lib/ASTMatchers/` implementation.

### Common Patterns
- Matchers return `bool` when applied to an `Operation*`.
- Combinators (`allOf`, `anyOf`) compose matchers using `&&` / `||` semantics.

## Dependencies

### Internal
- `mlir/lib/IR/` — `Operation`, `Attribute`, `Type`
- `mlir/lib/Query/Query.cpp`

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
