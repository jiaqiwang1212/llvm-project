<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Rewrite

## Purpose
Implements MLIR's pattern rewrite engine internals. Provides the PDL (Pattern Description Language) bytecode interpreter, the frozen pattern set (immutable compiled pattern database), and the pattern applicator that drives match-and-rewrite cycles.

## Key Files
| File | Description |
|------|-------------|
| `ByteCode.cpp` | PDL bytecode interpreter: executes compiled PDL match/rewrite programs (`.pdl` patterns compiled by `mlir-pdll`); dispatches to C++ callback hooks for external constraint and rewrite functions |
| `ByteCode.h` | Internal header for the PDL bytecode format constants and interpreter state |
| `FrozenRewritePatternSet.cpp` | `FrozenRewritePatternSet`: immutable, compiled set of `RewritePattern`s; created from a `RewritePatternSet` and used as input to `PatternApplicator` |
| `PatternApplicator.cpp` | `PatternApplicator`: applies a `FrozenRewritePatternSet` to ops; manages pattern benefit ordering, match failure recovery, and recursive application depth |

## For AI Agents

### Working In This Directory
- The PDL bytecode format is defined in `mlir/include/mlir/Rewrite/ByteCode.h`; the compiler is in `mlir/lib/Dialect/PDLInterp/`.
- `FrozenRewritePatternSet` must be constructed once and reused across many `PatternApplicator` runs; constructing it is expensive (sorts and compiles patterns).
- The greedy driver (`mlir/lib/Transforms/Utils/GreedyPatternRewriteDriver.cpp`) is the main consumer of `PatternApplicator`.

### Common Patterns
- External PDL constraint/rewrite functions are registered with `PDLPatternModule::registerConstraintFunction()` / `registerRewriteFunction()`.
- Pattern benefit is declared at pattern construction; higher benefit patterns are tried first.
- `PatternApplicator::applyDefaultCostModel()` uses the declared benefit ordering.

## Dependencies

### Internal
- `mlir/lib/IR/PatternMatch.cpp` — `RewritePattern`, `PatternRewriter`
- `mlir/lib/IR/PDL/PDLPatternMatch.cpp` — PDL pattern module
- `mlir/lib/Dialect/PDLInterp/` — PDL bytecode compiler

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
