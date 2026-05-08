<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Analysis/AliasAnalysis

## Purpose
Implements the local alias analysis backend for MLIR. Provides conservative may/must alias queries for SSA values within a single function or region, based on the operation semantics exposed through `ViewLikeOpInterface` and similar interfaces.

## Key Files
| File | Description |
|------|-------------|
| `LocalAliasAnalysis.cpp` | `LocalAliasAnalysis`: the primary local alias analysis implementation. Tracks alloc-like ops as distinct memory resources and follows view-like ops (subview, reshape, etc.) to determine whether two values alias, with support for `ViewLikeOpInterface` and `RegionBranchOpInterface` |

## For AI Agents

### Working In This Directory
- Register a custom backend by calling `aliasAnalysis.addAnalysisImplementation<LocalAliasAnalysis>(...)` on the `AliasAnalysis` object from `mlir/lib/Analysis/AliasAnalysis.cpp`.
- New memory-producing ops should implement `AllocOpInterface` or `ViewLikeOpInterface` to be handled correctly.

### Common Patterns
- Returns `AliasResult` (`NoAlias`, `MayAlias`, `MustAlias`) and `ModRefResult`.
- Conservative: unknown ops return `MayAlias`.

## Dependencies

### Internal
- `mlir/lib/Analysis/AliasAnalysis.cpp` — dispatcher
- `mlir/lib/IR/` — `Value`, `Operation`
- `mlir/lib/Interfaces/` — `ViewLikeInterface`, `SideEffectInterfaces`

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
