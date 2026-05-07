<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Scalar

## Purpose

Scalar optimizations for single-threaded code. Includes classic compiler optimizations like dead code elimination, loop transformations, global value numbering, strength reduction, and control flow simplification.

## Key Files

| File | Description |
|------|-------------|
| `Scalar.cpp` | Main scalar optimizations dispatcher |
| `SROA.cpp` | Scalar Replacement of Aggregates |
| `GVN.cpp` | Global Value Numbering |
| `GVNHoist.cpp` | GVN-based load hoisting |
| `GVNSink.cpp` | GVN-based load sinking |
| `NewGVN.cpp` | Next-generation GVN implementation |
| `LICM.cpp` | Loop Invariant Code Motion |
| `LoopUnrollPass.cpp` | Loop unrolling |
| `LoopUnrollAndJamPass.cpp` | Loop unroll-and-jam |
| `LoopRotation.cpp` | Loop rotation for canonicalization |
| `LoopSimplifyCFG.cpp` | Loop control flow simplification |
| `LoopDeletion.cpp` | Dead loop elimination |
| `LoopFusion.cpp` | Loop fusion for improved cache locality |
| `LoopDistribute.cpp` | Loop distribution for parallelization |
| `LoopInterchange.cpp` | Loop interchange for cache optimization |
| `LoopFlatten.cpp` | Loop flattening for nested loops |
| `LoopLoadElimination.cpp` | Load elimination within loops |
| `LoopSink.cpp` | Loop sinking optimization |
| `LoopBoundSplit.cpp` | Loop bound splitting |
| `LoopVersioningLICM.cpp` | Loop versioning for LICM |
| `LoopPredication.cpp` | Loop predication for branch elimination |
| `LoopIdiomRecognize.cpp` | Loop idiom recognition (memset, memcpy patterns) |
| `LoopInstSimplify.cpp` | Instruction simplification in loops |
| `LoopPassManager.cpp` | Loop pass manager |
| `IndVarSimplify.cpp` | Induction variable simplification |
| `InductiveRangeCheckElimination.cpp` | Range check elimination |
| `CorrelatedValuePropagation.cpp` | Value propagation using dataflow |
| `Reassociate.cpp` | Instruction reassociation for optimization |
| `DCE.cpp` | Dead code elimination |
| `DeadStoreElimination.cpp` | Dead store elimination |
| `MemCpyOptimizer.cpp` | Memory copy optimization |
| `SimplifyCFGPass.cpp` | Control flow graph simplification |
| `FlattenCFGPass.cpp` | CFG flattening |
| `StructurizeCFG.cpp` | CFG restructuring for GPU targets |
| `Sink.cpp` | Instruction sinking |
| `JumpThreading.cpp` | Jump threading (branch prediction) |
| `DFAJumpThreading.cpp` | DFA-based jump threading |
| `TailRecursionElimination.cpp` | Tail call optimization |
| `SpeculativeExecution.cpp` | Speculative execution of instructions |
| `EarlyCSE.cpp` | Early common subexpression elimination |
| `MergedLoadStoreMotion.cpp` | Merged load/store motion |
| `MergeICmps.cpp` | Integer comparison merging |
| `InstSimplifyPass.cpp` | Instruction simplification wrapper |
| `SimplifyLibCalls.cpp` | Library call simplification |
| `PartiallyInlineLibCalls.cpp` | Partial inlining of library calls |
| `Scalarizer.cpp` | Vector to scalar conversion |
| `ScalarizeMaskedMemIntrin.cpp` | Masked memory intrinsic scalarization |
| `ConstraintElimination.cpp` | Constraint-based dead code elimination |
| `NaryReassociate.cpp` | N-ary reassociation |
| `SeparateConstOffsetFromGEP.cpp` | GEP constant offset separation |
| `LoopDataPrefetch.cpp` | Loop data prefetching |
| `ConstantHoisting.cpp` | Constant materialization hoisting |
| `DropUnnecessaryAssumes.cpp` | Unnecessary assume removal |
| `GuardWidening.cpp` | Guard condition widening |
| `ExpandMemCmp.cpp` | Memory comparison expansion |
| `Float2Int.cpp` | Float to integer conversion for optimization |
| `AlignmentFromAssumptions.cpp` | Alignment deduction from assumptions |
| `DivRemPairs.cpp` | Division/remainder pairing |
| `InferAddressSpaces.cpp` | Address space inference |
| `InferAlignment.cpp` | Alignment inference |
| `JumpTableToSwitch.cpp` | Jump table to switch conversion |
| `PlaceSafepoints.cpp` | GC safepoint placement |
| `RewriteStatepointsForGC.cpp` | GC-aware statepoint rewriting |
| `StraightLineStrengthReduce.cpp` | Straight-line strength reduction |
| `SCCP.cpp` | Sparse conditional constant propagation |
| `Reg2Mem.cpp` | Register to memory conversion |
| `WarnMissedTransforms.cpp` | Warnings for missed optimizations |
| `LowerAtomicPass.cpp` | Atomic operation lowering |
| `LowerConstantIntrinsics.cpp` | Constant intrinsic lowering |
| `LowerExpectIntrinsic.cpp` | Expect intrinsic lowering |
| `LowerGuardIntrinsic.cpp` | Guard intrinsic lowering |
| `LowerMatrixIntrinsics.cpp` | Matrix intrinsic lowering |
| `LowerWidenableCondition.cpp` | Widening condition lowering |
| `MakeGuardsExplicit.cpp` | Guard condition explicitization |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When adding new scalar optimizations:

1. Identify the optimization type (loop, CFG, instruction, or value)
2. Create new .cpp file for the pass
3. Implement FunctionPass or LoopPass as appropriate
4. Use analysis passes (LoopInfo, DominatorTree, etc.)
5. Verify IR properties before transformations
6. Test with llvm-lit in llvm/test/Transforms/Scalar/

### Common Patterns

- **Loop analysis**: Use LoopInfo, SCEV (Scalar Evolution) for loop properties
- **Dataflow analysis**: Use dominance, post-dominance for CFG analysis
- **Value numbering**: Detect equivalent expressions for CSE
- **Safety checks**: Use assert() to verify IR validity after transformations

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, types
- `llvm/include/llvm/Analysis/LoopInfo.h` — loop analysis
- `llvm/include/llvm/Analysis/ScalarEvolution.h` — SCEV analysis
- `llvm/include/llvm/Analysis/DominatorTree.h` — dominance analysis
- `llvm/include/llvm/Analysis/ValueTracking.h` — value properties
- `llvm/lib/Analysis/` — analysis implementations

### External
- Standard C++ library (containers, algorithms)

<!-- MANUAL: -->
