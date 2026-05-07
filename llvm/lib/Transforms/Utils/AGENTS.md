<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Utils

## Purpose

Utility passes and helper functions for IR transformation. Provides foundational infrastructure used by other passes: function cloning, loop simplification, memory-to-register promotion, critical edge breaking, and common IR modification patterns.

## Key Files

| File | Description |
|------|-------------|
| `Utils.cpp` | Main utilities dispatcher and registration |
| `CloneFunction.cpp` | Function cloning and copying |
| `CloneModule.cpp` | Module cloning utilities |
| `CodeExtractor.cpp` | Code region extraction for separate functions |
| `BreakCriticalEdges.cpp` | Critical edge breaking for CFG simplification |
| `LoopSimplify.cpp` | Loop canonicalization and simplification |
| `LoopUtils.cpp` | Loop utility functions |
| `LoopPeel.cpp` | Loop peeling (partial unrolling) |
| `LoopRotationUtils.cpp` | Loop rotation utilities |
| `LoopConstrainer.cpp` | Loop constraint enforcement |
| `LoopUnroll.cpp` | Loop unrolling implementation |
| `LoopUnrollAndJam.cpp` | Loop unroll-and-jam implementation |
| `LoopUnrollRuntime.cpp` | Runtime loop unrolling |
| `LoopVersioning.cpp` | Loop versioning for optimization |
| `LCSSA.cpp` | Loop Closed SSA form transformation |
| `Mem2Reg.cpp` | Memory to register promotion wrapper |
| `PromoteMemoryToRegister.cpp` | Aggressive register promotion (Mem2Reg) |
| `InlineFunction.cpp` | Function inlining implementation |
| `InjectTLIMappings.cpp` | Target library info injection |
| `LowerSwitch.cpp` | Switch statement lowering |
| `LowerInvoke.cpp` | Invoke instruction lowering |
| `LowerAtomic.cpp` | Atomic operation lowering |
| `LowerMemIntrinsics.cpp` | Memory intrinsic lowering (memcpy, etc.) |
| `LowerVectorIntrinsics.cpp` | Vector intrinsic lowering |
| `Local.cpp` | Local IR transformations (instruction simplification) |
| `SimplifyCFG.cpp` | Advanced CFG simplification |
| `FlattenCFG.cpp` | CFG flattening |
| `BasicBlockUtils.cpp` | Basic block utilities |
| `DemoteRegToStack.cpp` | Register to stack demotion |
| `UnifyFunctionExitNodes.cpp` | Function exit unification |
| `UnifyLoopExits.cpp` | Loop exit unification |
| `ValueMapper.cpp` | Value remapping for cloning/inlining |
| `SSAUpdater.cpp` | SSA form updater |
| `SSAUpdaterBulk.cpp` | Bulk SSA form updates |
| `BuildLibCalls.cpp` | Library call code generation |
| `SimplifyLibCalls.cpp` | Library call simplification |
| `CallGraphUpdater.cpp` | Call graph maintenance during transforms |
| `CallPromotionUtils.cpp` | Indirect call promotion utilities |
| `CodeMoverUtils.cpp` | Instruction movement utilities |
| `ControlFlowUtils.cpp` | Control flow manipulation |
| `GlobalStatus.cpp` | Global variable status analysis |
| `ModuleUtils.cpp` | Module-level utilities |
| `FunctionComparator.cpp` | Function comparison for merging |
| `FunctionImportUtils.cpp` | Function importing utilities |
| `GuardUtils.cpp` | Guard condition utilities |
| `CtorUtils.cpp` | Constructor/destructor utilities |
| `CanonicalizeAliases.cpp` | Alias canonicalization |
| `CanonicalizeFreezeInLoops.cpp` | Freeze canonicalization in loops |
| `EscapeEnumerator.cpp` | Exception escape enumeration |
| `Evaluator.cpp` | Constant expression evaluator |
| `ScalarEvolutionExpander.cpp` | SCEV expansion to code |
| `SCCPSolver.cpp` | SCCP (Sparse Conditional CP) solver |
| `PredicateInfo.cpp` | Predicate information tracking |
| `ASanStackFrameLayout.cpp` | AddressSanitizer stack frame layout |
| `SampleProfileLoaderBaseUtil.cpp` | Sample profiling utilities |
| `SampleProfileInference.cpp` | Sample profile inference |
| `BypassSlowDivision.cpp` | Slow division bypass |
| `FixIrreducible.cpp` | Irreducible CFG fixing |
| `EntryExitInstrumenter.cpp` | Entry/exit point instrumentation |
| `Instrumentation.cpp` | General instrumentation utilities |
| `Debugify.cpp` | Debug info preservation checking |
| `DebugSSAUpdater.cpp` | Debug-aware SSA updates |
| `DeclareRuntimeLibcalls.cpp` | Runtime library declaration |
| `IntegerDivision.cpp` | Integer division lowering |
| `AddDiscriminators.cpp` | Discriminator insertion for debugging |
| `MetaRenamer.cpp` | Metadata-based renaming |
| `NameAnonGlobals.cpp` | Anonymous global naming |
| `InstructionNamer.cpp` | Instruction naming for debugging |
| `DXILUpgrade.cpp` | DXIL version upgrading |
| `SplitModule.cpp` | Module splitting |
| `SplitModuleByCategory.cpp` | Category-based module splitting |
| `SizeOpts.cpp` | Size optimization utilities |
| `SymbolRewriter.cpp` | Symbol rewriting for linking |
| `RelLookupTableConverter.cpp` | Relative lookup table conversion |
| `StripConvergenceIntrinsics.cpp` | Convergence intrinsic stripping |
| `StripGCRelocates.cpp` | GC relocation removal |
| `StripNonLineTableDebugInfo.cpp` | Line table debug info stripping |
| `VNCoercion.cpp` | Value numbering type coercion |
| `CodeLayout.cpp` | Code layout optimization |
| `CountVisits.cpp` | Block visit counting |
| `SanitizerStats.cpp` | Sanitizer statistics tracking |
| `MemoryOpRemark.cpp` | Memory operation remarks |
| `MemoryTaggingSupport.cpp` | Memory tagging utilities |
| `MisExpect.cpp` | MisExpect check support |
| `MoveAutoInit.cpp` | Auto-initialization movement |
| `ProfileVerify.cpp` | Profile correctness verification |
| `IRNormalizer.cpp` | IR canonicalization |
| `AMDGPUEmitPrintf.cpp` | AMDGPU printf emission |
| `HelloWorld.cpp` | Example/test utility pass |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new utility passes or helpers:

1. Determine if it's a standalone pass (register in Passes/PassRegistry.def) or helper
2. Create new .cpp file for the implementation
3. Place in appropriate category (loop utils, function utils, IR utils)
4. Document preconditions and postconditions (what IR properties required/preserved)
5. Test with llvm-lit in llvm/test/Transforms/Utils/

### Common Patterns

- **Visitor patterns**: Inherit from InstVisitor or use BlockFrequencyInfo for traversal
- **IR modification**: Use ValueMapper for consistent value remapping during cloning
- **SSA maintenance**: Use SSAUpdater or PromoteMemoryToRegister for phi insertion
- **Pass composition**: Combine analyses (LoopInfo, DominatorTree) to achieve transformations

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, instructions, modules
- `llvm/include/llvm/Analysis/LoopInfo.h` — loop analysis
- `llvm/include/llvm/Analysis/DominatorTree.h` — dominance analysis
- `llvm/include/llvm/Analysis/ScalarEvolution.h` — SCEV analysis
- `llvm/include/llvm/Transforms/Utils/` — utility pass interfaces
- `llvm/lib/Analysis/` — analysis implementations

### External
- Standard C++ library (containers, algorithms)

<!-- MANUAL: -->
