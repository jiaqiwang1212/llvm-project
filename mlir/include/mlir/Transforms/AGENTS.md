<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Transforms/

## Purpose
Dialect-independent transformation pass headers. Contains the canonical set of general-purpose MLIR passes (CSE, inlining, canonicalization, mem2reg, SROA, etc.) and supporting utilities for dialect conversion, pattern rewriting drivers, control flow restructuring, and loop transformations. These passes are available to all dialects without introducing dialect-specific dependencies.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Registration and accessor functions for all Transforms passes |
| `Passes.td` | TableGen definitions for Transforms passes (options, descriptions) |
| `DialectConversion.h` | `ConversionTarget`, `TypeConverter`, `ConversionPatternRewriter` for dialect lowering |
| `DialectConversion.pdll` | PDLL patterns for dialect conversion |
| `GreedyPatternRewriteDriver.h` | `applyPatternsGreedily()` and greedy rewrite driver config |
| `WalkPatternRewriteDriver.h` | Walk-based pattern rewrite driver |
| `CSE.h` | Common Subexpression Elimination pass |
| `Inliner.h` | Inliner pass and configuration |
| `InliningUtils.h` | Utilities for implementing inlining (op cloning, argument mapping) |
| `DialectInlinerInterface.td` | TableGen interface for dialect-specific inlining hooks |
| `FoldUtils.h` | `OperationFolder` for folding ops with constants |
| `Mem2Reg.h` | Memory-to-register promotion pass |
| `SROA.h` | Scalar Replacement of Aggregates pass |
| `RegionUtils.h` | Region manipulation: merging, cloning, erasing unused values |
| `ControlFlowSinkUtils.h` | Utilities for sinking ops into control flow regions |
| `CFGToSCF.h` | Convert CFG-style control flow to structured control flow |
| `LoopInvariantCodeMotionUtils.h` | LICM utility functions |
| `LocationSnapshot.h` | Pass to snapshot op locations to external file |
| `ViewOpGraph.h` | Emit graphviz of op dependency graph |
| `EndomorphismSimplification.h` | Simplification of endomorphism patterns |
| `HomomorphismSimplification.h` | Simplification of homomorphism patterns |
| `CommutativityUtils.h` | Utilities for commutative op canonicalization |
| `BubbleDownMemorySpaceCasts.h` | Bubble memory-space cast ops downward |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- New passes must be added to both `Passes.td` (for options/description) and `Passes.h` (for the `create*Pass()` factory).
- `DialectConversion.h` is the primary entry point for lowering passes; use `ConversionTarget` to specify legal/illegal ops.
- Pattern rewrite drivers (`GreedyPatternRewriteDriver.h`) are used by canonicalization and most optimization passes.
- The `DialectInlinerInterface.td` must be implemented by any dialect that wants its ops to be inlinable.

### Common Patterns
- Pass options are declared in `.td` with `Option<>` and `ListOption<>` fields.
- Dialect conversions use `populateXxxToYyyConversionPatterns(patterns, typeConverter)` convention.
- `applyPatternsGreedily(op, patterns)` is the standard canonicalization driver.
- Inlining hooks: implement `DialectInlinerInterface` and override `isLegalToInline`.

## Dependencies

### Internal
- `mlir/IR/` (PatternMatch, Operation, Region)
- `mlir/Pass/` (Pass base class)
- `mlir/Interfaces/` (CallInterfaces, ControlFlowInterfaces, SideEffectInterfaces)
- `mlir/Analysis/` (for LICM, slice utilities)

### External
- `llvm/ADT/` (SmallVector, DenseSet)

<!-- MANUAL: -->
