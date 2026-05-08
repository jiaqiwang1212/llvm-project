<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Query/Matcher/

## Purpose
Matcher framework for the `mlir-query` tool. Provides a typed matcher system for matching MLIR operations by structural and semantic criteria, modeled after Clang's AST matcher design. Includes the matcher registry, variant value type, internal matcher machinery, and slice-based matchers.

## Key Files
| File | Description |
|------|-------------|
| `ErrorBuilder.h` | Error accumulation and reporting for matcher parse errors |
| `Marshallers.h` | Marshalling between dynamic matcher arguments and typed C++ matchers |
| `MatchersInternal.h` | Internal matcher machinery: `Matcher<T>`, `MatcherInterface<T>`, combinators |
| `MatchFinder.h` | `MatchFinder` — traverses IR and invokes callbacks on matched ops |
| `Registry.h` | `MatcherRegistry` — maps string names to matcher factory functions |
| `SliceMatchers.h` | Matchers for forward/backward slices of operations |
| `VariantValue.h` | `VariantValue` — type-erased value for dynamic matcher argument passing |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Matchers are strongly typed at the C++ level but accessed dynamically via the registry by name.
- The registry maps `"hasOpName"` → a factory that creates a `Matcher<Operation*>`.
- `VariantValue` enables dynamic invocation from parsed query strings.
- `MatchFinder` drives IR traversal and calls registered `MatchCallback`s on each match.

### Common Patterns
- Query string: `match hasOpName("func.func")` parsed and dispatched via `MatcherRegistry`.
- Combinator: `allOf(hasOpName("arith.addi"), isPublic())`.
- Callback: implement `MatchCallback::run(MatchResult &result)`.

## Dependencies

### Internal
- `mlir/IR/` (Operation, MLIRContext)
- `mlir/Analysis/SliceAnalysis.h`
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (StringRef, SmallVector)

<!-- MANUAL: -->
