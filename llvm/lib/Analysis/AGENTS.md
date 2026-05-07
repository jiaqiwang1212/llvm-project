<!-- Generated: 2026-05-07 -->

# Analysis — LLVM Analysis Passes

## Purpose

Implementation of analysis passes that compute properties of IR without modifying it. Analyses provide information used by transformation passes: data flow, control flow, loop structure, memory behavior, cost models, and profiling data. Analyses are typically "read-only" and can be reused across multiple passes.

## Key Files

| File | Description |
|------|-------------|
| `AliasAnalysis.cpp` | Base class for alias analysis (memory dependency analysis) |
| `BasicAliasAnalysis.cpp` | Precise alias analysis using simple rules (most common analysis) |
| `GlobalsModRef.cpp` | Alias analysis for global variable memory references |
| `ScopedNoAliasAA.cpp` | Alias analysis using `noalias` metadata |
| `TypeBasedAliasAnalysis.cpp` | Alias analysis based on type information |
| `ObjCARCAliasAnalysis.cpp` | Objective-C reference counting alias analysis |
| `ObjCARCAnalysisUtils.cpp` | Objective-C ARC analysis utilities |
| `ObjCARCInstKind.cpp` | Objective-C ARC instruction classification |
| `DominatorTree.cpp` (in IR, not here) | — Dominator tree (see IR/) |
| `LoopInfo.cpp` | Loop detection and analysis (builds loop tree) |
| `LoopAnalysisManager.cpp` | Pass manager for loop-level analyses |
| `LoopPass.cpp` | Base class for loop transformation passes |
| `LoopAccessAnalysis.cpp` | Memory access patterns in loops |
| `LoopCacheAnalysis.cpp` | Loop cache locality analysis |
| `LoopNestAnalysis.cpp` | Nested loop analysis |
| `LoopUnrollAnalyzer.cpp` | Heuristics for loop unrolling |
| `ScalarEvolution.cpp` | Symbolic expression analysis for induction variables |
| `ScalarEvolutionAliasAnalysis.cpp` | Alias analysis using scalar evolution |
| `ScalarEvolutionDivision.cpp` | Division and modulo analysis for scalar evolution |
| `ScalarEvolutionNormalization.cpp` | Normalization of scalar evolution expressions |
| `IVDescriptors.cpp` | Induction variable descriptor analysis |
| `IVUsers.cpp` | Users of induction variables |
| `CallGraph.cpp` | Call graph construction and analysis |
| `CallGraphSCCPass.cpp` | Pass manager for strongly connected components of call graph |
| `LazyCallGraph.cpp` | Lazy (on-demand) call graph construction |
| `DependenceAnalysis.cpp` | Loop dependence analysis (array subscript dependencies) |
| `DependenceGraphBuilder.cpp` | Data dependence graph construction |
| `DDG.cpp` | Fine-grained dependence graph |
| `DominanceFrontier.cpp` | Dominance frontier computation |
| `PostDominators.cpp` | Post-dominator tree (domination from exit) |
| `RegionInfo.cpp` | Region analysis (generalization of loops) |
| `RegionPass.cpp` | Base class for region-level passes |
| `CFG.cpp` | Control flow graph analysis |
| `BlockFrequencyInfo.cpp` | Block execution frequency estimation |
| `BlockFrequencyInfoImpl.cpp` | Block frequency implementation |
| `LazyBlockFrequencyInfo.cpp` | Lazy block frequency analysis |
| `BranchProbabilityInfo.cpp` | Branch prediction probability estimation |
| `LazyBranchProbabilityInfo.cpp` | Lazy branch probability analysis |
| `MemorySSA.cpp` | Memory SSA form (virtual use-def for memory) |
| `MemorySSAUpdater.cpp` | Incremental update for memory SSA |
| `MemoryDependenceAnalysis.cpp` | Memory dependency (def-use) analysis |
| `MemoryLocation.cpp` | Memory location representation |
| `MemoryBuiltins.cpp` | Analysis of memory allocation/deallocation |
| `MemoryProfileInfo.cpp` | Memory profiling data analysis |
| `MemDerefPrinter.cpp` | Printable analysis for memory dereferences |
| `Loads.cpp` | Load instruction analysis |
| `MustExecute.cpp` | Definitely-executed instruction analysis |
| `ValueTracking.cpp` | Value property tracking (known bits, ranges, etc.) |
| `ValueLattice.cpp` | Lattice-based value analysis |
| `ValueLatticeUtils.cpp` | Value lattice utilities |
| `InstructionSimplify.cpp` | Instruction simplification without building new IR |
| `LazyValueInfo.cpp` | Lazy value constraint analysis |
| `TargetLibraryInfo.cpp` | Target-specific library function attributes |
| `TargetTransformInfo.cpp` | Target cost modeling (instruction costs, etc.) |
| `CostModel.cpp` | Abstract cost model for vectorization decisions |
| `VectorUtils.cpp` | Vector-related analysis utilities |
| `AssumptionCache.cpp` | Caching of assume intrinsics |
| `AssumeBundleQueries.cpp` | Queries on assume bundles |
| `GuardUtils.cpp` | Guard intrinsic utilities |
| `CaptureTracking.cpp` | Pointer capture analysis (is a pointer escaping?) |
| `PHITransAddr.cpp` | PHI node translation of addresses |
| `PhiValues.cpp` | PHI value analysis |
| `StackLifetime.cpp` | Stack object lifetime analysis |
| `StackSafetyAnalysis.cpp` | Stack memory safety analysis |
| `PtrUseVisitor.cpp` | Visitor for pointer uses |
| `InstCount.cpp` | Instruction count statistics |
| `CodeMetrics.cpp` | Code size and complexity metrics |
| `CmpInstAnalysis.cpp` | Comparison instruction analysis |
| `ConstantFolding.cpp` | Compile-time constant folding (analysis version) |
| `Delinearization.cpp` | Delinearization of array subscripts |
| `DemandedBits.cpp` | Which bits of a value are actually used? |
| `EphemeralValuesCache.cpp` | Caching of ephemeral value detection |
| `FloatingPointPredicateUtils.cpp` | Floating-point predicate analysis |
| `ConstraintSystem.cpp` | Linear constraint solver |
| `FunctionPropertiesAnalysis.cpp` | Function characteristics analysis |
| `DomConditionCache.cpp` | Caching of dominator-based conditions |
| `DomTreeUpdater.cpp` | Incremental dominator tree updates |
| `DXILMetadataAnalysis.cpp` | DirectX IL metadata analysis |
| `DXILResource.cpp` | DirectX IL resource analysis |
| `IndirectCallPromotionAnalysis.cpp` | Indirect call devirtualization analysis |
| `HashRecognize.cpp` | Hash function pattern recognition |
| `HeatUtils.cpp` | Heat (frequency) utilities |
| `KernelInfo.cpp` | Kernel (GPU) information analysis |
| `Lint.cpp` | IR linting (error detection) |
| `Trace.cpp` | Execution trace analysis |
| `ModuleDebugInfoPrinter.cpp` | Debug info printing pass |
| `ModuleSummaryAnalysis.cpp` | Whole-program module summary |
| `OptimizationRemarkEmitter.cpp` | Emits optimization remarks (for diagnostics) |
| `OverflowInstAnalysis.cpp` | Overflow instruction pattern detection |
| `ProfileSummaryInfo.cpp` | Profile summary and hotness queries |
| `RuntimeLibcallInfo.cpp` | Runtime library call information |
| `SyntheticCountsUtils.cpp` | Synthetic block count generation |
| `TypeMetadataUtils.cpp` | Type metadata utilities |
| `UniformityAnalysis.cpp` | Uniform (convergence-safe) value analysis |
| `CycleAnalysis.cpp` | Cycle (generalized loop) analysis |
| `CGSCCPassManager.cpp` | Pass manager for call graph SCC traversal |
| `CallPrinter.cpp` | Call graph printing pass |
| `CFGPrinter.cpp` | CFG printing pass |
| `CFGSCCPrinter.cpp` | CFG SCC printing pass |
| `DDGPrinter.cpp` | Dependence graph printing pass |
| `DomPrinter.cpp` | Dominator tree printing pass |
| `RegionPrinter.cpp` | Region printing pass |
| `Analysis.cpp` | Analysis registry and utilities |
| `CtxProfAnalysis.cpp` | Context profile analysis |
| `LastRunTrackingAnalysis.cpp` | Last-run state tracking for incremental analysis |
| `IR2Vec.cpp` | IR-to-vector representation (machine learning) |
| `IRSimilarityIdentifier.cpp` | Identifies similar IR code (for code deduplication) |
| `InlineAdvisor.cpp` | Inlining heuristics and decision-making |
| `DevelopmentModeInlineAdvisor.cpp` | Development mode inline advisor |
| `MLInlineAdvisor.cpp` | Machine learning-based inline advisor |
| `ReplayInlineAdvisor.cpp` | Replay advisor (for regression testing) |
| `InteractiveModelRunner.cpp` | Interactive ML model runner |
| `NoInferenceModelRunner.cpp` | Stub model runner (no ML) |
| `ModelUnderTrainingRunner.cpp` | Model-under-training runner (ML) |
| `InlineCost.cpp` | Inlining cost estimation |
| `InlineOrder.cpp` | Inlining order determination |
| `ImportedFunctionsInliningStatistics.cpp` | Statistics for imported function inlining |
| `InstructionPrecedenceTracking.cpp` | Instruction ordering analysis |
| `TensorSpec.cpp` | Tensor specification (for ML) |
| `TFLiteUtils.cpp` | TensorFlow Lite model utilities |
| `TrainingLogger.cpp` | Machine learning training data logger |

## Subdirectories

- `models/` — Machine learning models for analysis (inline advisors, etc.)

## For AI Agents

### Working In This Directory

1. **Analyses are read-only**: An analysis pass must not modify IR. Use `const` and `const&` parameters throughout.

2. **Results are cached**: Analyses are computed once and cached by the pass manager. If IR changes, analyses must be invalidated.

3. **Understand invalidation**: When writing a transformation pass, understand which analyses it invalidates. Most structural changes (adding/removing blocks or instructions) invalidate many analyses.

4. **Use lazy evaluation**: Some analyses (like `LazyCallGraph`) compute results on-demand. This can be more efficient than eager computation.

5. **Depend on other analyses**: Your analysis can depend on other analyses. Use `getAnalysis<>()` in the legacy pass manager or the new `getResult<>()` in the new pass manager.

6. **Handle analysis ownership**: In the new pass manager, analyses are owned by the pass manager. Do not store pointers to analysis results if the IR will be modified.

7. **Profile-guided analysis**: Many analyses use profiling data. Check `BlockFrequencyInfo` and `BranchProbabilityInfo` for examples.

### Common Patterns

**Writing a simple analysis pass (legacy):**
```cpp
struct MyAnalysis : public FunctionPass {
  static char ID;
  MyAnalysis() : FunctionPass(ID) {}
  
  bool runOnFunction(Function &F) override {
    // Compute analysis results
    return false;  // Don't modify IR
  }
};
```

**Accessing an analysis result:**
```cpp
auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
```

**Using scalar evolution to analyze a loop:**
```cpp
const SCEV *Expr = SE.getSCEV(Value);
if (const SCEVConstant *C = dyn_cast<SCEVConstant>(Expr)) {
  ConstantInt *CInt = C->getValue();
}
```

**Checking alias analysis:**
```cpp
AliasResult AR = AA.alias(MemoryLocation(PtrA), MemoryLocation(PtrB));
if (AR == NoAlias) {
  // Pointers definitely don't alias
}
```

## Dependencies

### Internal

- **llvm/lib/IR/** — All analyses operate on IR
- **llvm/lib/Support/** — Utility functions
- **llvm/include/llvm/Analysis/** — Public analysis headers

### External

None.

### Dependents

- **llvm/lib/Transforms/** — Transformation passes use analyses
- **llvm/lib/CodeGen/** — Code generation uses analyses

## Notes for Developers

- **Always implement `getAnalysisUsage()`**: Document which analyses your pass requires and which it preserves (doesn't invalidate).
- **Consider incremental updates**: For long-lived analyses, implement incremental update logic (see `MemorySSAUpdater`).
- **Profile feedback**: Use profiling data to improve heuristics (see `BlockFrequencyInfo`).
- **Machine learning**: Recent additions like `MLInlineAdvisor` show the direction of LLVM development—consider ML for complex heuristics.
- **Avoid expensive analyses in hot paths**: Some analyses are expensive (e.g., `DependenceAnalysis`). Use sparingly or make lazy.

<!-- MANUAL: -->
