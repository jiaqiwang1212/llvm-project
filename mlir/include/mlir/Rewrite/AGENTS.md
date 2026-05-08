<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Rewrite/

## Purpose
Low-level pattern rewrite engine primitives. Provides `FrozenRewritePatternSet` (an immutable, compiled pattern set for repeated application), `PatternApplicator` (the core engine that matches and applies patterns), and TableGen utilities for pass-level rewrite configuration. This layer sits below the higher-level drivers in `mlir/Transforms/`.

## Key Files
| File | Description |
|------|-------------|
| `FrozenRewritePatternSet.h` | `FrozenRewritePatternSet` — immutable pattern set built from a `RewritePatternSet`; thread-safe for concurrent application |
| `PatternApplicator.h` | `PatternApplicator` — matches patterns against ops and applies the best match |
| `PassUtil.td` | TableGen utilities for declaring pass-level pattern options |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `FrozenRewritePatternSet` is the type passed to `applyPatternsGreedily()` and dialect conversion drivers.
- Freeze a `RewritePatternSet` once after all patterns are added to amortize sorting/indexing cost.
- `PatternApplicator` requires a `RewriterBase` (e.g., `IRRewriter`) to apply matched patterns.
- `PassUtil.td` provides the `Statistic` and `RewritePatternSet` option infrastructure for passes.

### Common Patterns
- `RewritePatternSet patterns(&ctx); populateMyPatterns(patterns); FrozenRewritePatternSet frozen(std::move(patterns));`
- `PatternApplicator applicator(frozen); applicator.applyDefaultCostModel(); applicator.matchAndRewrite(op, rewriter);`

## Dependencies

### Internal
- `mlir/IR/` (PatternMatch, Operation, MLIRContext)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (SmallVector, DenseMap)

<!-- MANUAL: -->
