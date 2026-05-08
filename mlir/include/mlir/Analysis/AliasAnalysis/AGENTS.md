<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Analysis/AliasAnalysis/

## Purpose
Local alias analysis implementation headers. Provides `LocalAliasAnalysis`, an interprocedural alias analysis that reasons about memory effects locally within a function/region using side-effect interfaces and type information.

## Key Files
| File | Description |
|------|-------------|
| `LocalAliasAnalysis.h` | `LocalAliasAnalysis` — queries alias relationships between two `Value` objects using local information |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `LocalAliasAnalysis` implements the `AliasAnalysis` interface declared in `mlir/Analysis/AliasAnalysis.h`.
- It uses `MemoryEffectOpInterface` to determine which ops read/write which values.
- For dataflow-based alias analysis, use the framework in `mlir/Analysis/DataFlow/` instead.

### Common Patterns
- `LocalAliasAnalysis analysis(op); auto result = analysis.alias(v1, v2);`
- Result values: `AliasResult::NoAlias`, `MayAlias`, `MustAlias`.

## Dependencies

### Internal
- `mlir/Analysis/AliasAnalysis.h` (base AliasAnalysis interface)
- `mlir/Interfaces/SideEffectInterfaces.h`
- `mlir/IR/` (Value, Operation)

### External
- `llvm/ADT/`

<!-- MANUAL: -->
