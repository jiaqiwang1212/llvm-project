<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Analysis

## Purpose

The Analysis directory contains LLVM IR analysis passes that extract properties and relationships from the intermediate representation without modifying the IR. These analyses are used by transformation passes to make informed optimization decisions.

## Key Files

| File | Description |
|------|-------------|
| `AliasAnalysis.h` | Generic alias analysis interface for querying memory aliasing relationships and mod/ref information |
| `BasicAliasAnalysis.h` | Fast, flow-insensitive alias analysis |
| `AliasSetTracker.h` | Alias set tracking for grouping memory locations with potential aliases |
| `ScalarEvolution.h` | Symbolic computation of scalar expressions in loops |
| `DominatorTree.h` | Dominator tree construction and queries |
| `PostDominatorTree.h` | Post-dominator tree for reverse dominance analysis |
| `LoopInfo.h` | Loop detection and hierarchy analysis |
| `MemorySSA.h` | Static single assignment form for memory |
| `DependenceAnalysis.h` | Data and control dependence analysis |
| `CallGraph.h` | Call graph construction and call site analysis |
| `BlockFrequencyInfo.h` | Block execution frequency estimation |
| `BranchProbabilityInfo.h` | Branch probability analysis |
| `ValueTracking.h` | Query functions for constant and value propagation |
| `InstructionSimplify.h` | Instruction-level simplification without IR modification |
| `ConstantFolding.h` | Compile-time evaluation of constant expressions |
| `CFG.h` | Control flow graph utilities and traversal |
| `ProfileSummaryInfo.h` | Profile summary statistics for optimization decisions |
| `DXILResource.h` | DirectX IL resource analysis |

## Subdirectories (if applicable)

No subdirectories; all analyses are headers in this directory.

## For AI Agents

### Working In This Directory

When implementing or extending analyses:

1. **Read the query interface first** — most analyses expose a public interface with query methods rather than exposing internal data structures
2. **Use CRTP (Curiously Recurring Template Pattern)** for analysis implementations — the pattern `class MyAnalysis : public AnalysisInfoMixin<MyAnalysis>` is standard
3. **Register with PassManager** — new analyses must be registered via `AnalysisManager::registerPass()` or similar
4. **Understand dependency chains** — analyses often depend on other analyses; specify these dependencies clearly
5. **Test on real IR** — analyses extract properties; verify your analysis works on diverse IR patterns

### Common Patterns

- **Caching** — Most analyses cache results; invalidation is handled by the PassManager when IR changes
- **Lazy computation** — Results are often computed on-demand, not upfront
- **Query-based interface** — Rather than storing results in per-IR-element data structures, query methods are typical
- **Mod/Ref analysis** — Alias analyses return `AliasResult` (NoAlias, MayAlias, PartialAlias, MustAlias) and Mod/Ref information
- **SSA form** — Analyses assume IR is in SSA form unless otherwise noted

## Dependencies

### Internal

- `llvm/IR/` — Core IR classes (Function, BasicBlock, Instruction, Value)
- `llvm/Pass.h` — Pass infrastructure and registration
- `llvm/ADT/` — Data structures (DenseMap, SmallVector, etc.)
- `llvm/Support/` — Utilities and data structures
- `llvm/Transforms/Utils/` — Some analyses use utility functions

### External

- Some analyses depend on target-specific information (TargetLibraryInfo)
- Profile data may be loaded from external sources

<!-- MANUAL: -->
