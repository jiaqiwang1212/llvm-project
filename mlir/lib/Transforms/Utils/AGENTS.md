<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Transforms/Utils

## Purpose
Shared transformation utilities used by passes throughout the MLIR ecosystem. This is the most-depended-upon utility layer for IR transformation work, providing the dialect conversion framework, inliner, greedy and walk-based pattern rewrite drivers, and miscellaneous IR manipulation helpers.

## Key Files
| File | Description |
|------|-------------|
| `DialectConversion.cpp` | `DialectConversionTarget`, `TypeConverter`, `ConversionPatternRewriter`: the framework for progressive dialect lowering; supports legal/illegal op marking, type conversion, and rollback on failure |
| `GreedyPatternRewriteDriver.cpp` | `applyPatternsGreedily()`: the greedy (worklist-driven) pattern application engine used by canonicalization and most rewrite passes |
| `WalkPatternRewriteDriver.cpp` | `applyPatternsAndFoldGreedily()` walk-based variant: applies patterns via IR walk rather than worklist (useful for order-sensitive rewrites) |
| `Inliner.cpp` | `Inliner` class: performs call-site inlining using `CallOpInterface` / `CallableOpInterface`; handles argument/result materialization and cost modeling |
| `InliningUtils.cpp` | Low-level inlining helpers: region cloning into call site, SSA operand remapping, result replacement |
| `RegionUtils.cpp` | Region-level utilities: `mergeBlocks`, `simplifyRegions`, `makeRegionIsolatedFromAbove`, dead block elimination |
| `LoopInvariantCodeMotionUtils.cpp` | LICM implementation shared between `LoopInvariantCodeMotion.cpp` pass and dialect-specific loops |
| `ControlFlowSinkUtils.cpp` | Control-flow sinking utility shared by `ControlFlowSink.cpp` pass |
| `CommutativityUtils.cpp` | Utilities for generating commutativity-exploiting canonicalization patterns |
| `FoldUtils.cpp` | `OperationFolder`: fold ops in-place; deduplicates constant-like results with the folder's constant cache |
| `CFGToSCF.cpp` | Converts unstructured CFG regions to structured control flow (`scf.if`, `scf.while`); used during SCF-to-CFG reverse lowering |

## For AI Agents

### Working In This Directory
- `DialectConversion.cpp` is large and complex; understand the conversion target / type converter protocol before modifying.
- `GreedyPatternRewriteDriver.cpp` implements the core rewrite loop; performance changes here affect all passes using canonicalization.
- `Inliner.cpp` interacts with `CallOpInterface`; new callable/call op types must implement those interfaces to participate in inlining.

### Common Patterns
- Dialect conversion passes call `applyPartialConversion()` or `applyFullConversion()`, not `applyPatternsGreedily()`.
- `FoldUtils` / `OperationFolder` is used internally by the greedy driver; it caches constants to avoid duplication.
- CFG-to-SCF conversion is a prerequisite for certain structured control flow analyses.

## Dependencies

### Internal
- `mlir/lib/IR/` — core IR
- `mlir/lib/Analysis/` — dominator tree, CFG loop info
- `mlir/lib/Interfaces/` — `CallInterfaces`, `ControlFlowInterfaces`, `SideEffectInterfaces`

### External
- `llvm/lib/Support` — ADT, `llvm::DominatorTree`

<!-- MANUAL: -->
