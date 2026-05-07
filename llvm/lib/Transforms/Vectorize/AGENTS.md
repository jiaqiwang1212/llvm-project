<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Vectorize

## Purpose

Auto-vectorization passes that transform scalar code to vector (SIMD) code for parallel execution on vector hardware. Includes loop vectorization (LV), superword-level parallelism (SLP), and vector combine optimizations.

## Key Files

| File | Description |
|------|-------------|
| `Vectorize.cpp` | Main vectorization dispatcher |
| `LoopVectorize.cpp` | Loop vectorization pass |
| `LoopVectorizationLegality.cpp` | Legality checking for loop vectorization |
| `SLPVectorizer.cpp` | Superword-level parallelism vectorizer |
| `VectorCombine.cpp` | Vector operation combining and simplification |
| `LoadStoreVectorizer.cpp` | Load/store vectorization |
| `LoopIdiomVectorize.cpp` | Loop idiom recognition for vectorization |
| `VPlan.cpp` | Vectorization plan implementation |
| `VPlan.h` | Vectorization plan data structures |
| `VPlanAnalysis.cpp` | VPlan analysis utilities |
| `VPlanAnalysis.h` | VPlan analysis interfaces |
| `VPlanCFG.h` | VPlan control flow graph |
| `VPlanConstruction.cpp` | VPlan construction from loops |
| `VPlanDominatorTree.h` | VPlan dominator tree |
| `VPlanHelpers.h` | VPlan helper functions |
| `VPlanPatternMatch.h` | VPlan pattern matching |
| `VPlanPredicator.cpp` | VPlan predication |
| `VPlanRecipes.cpp` | VPlan recipe implementations |
| `VPlanSLP.cpp` | SLP within VPlan |
| `VPlanSLP.h` | SLP infrastructure for VPlan |
| `VPlanTransforms.cpp` | VPlan transformations |
| `VPlanTransforms.h` | VPlan transformation interfaces |
| `VPlanUnroll.cpp` | VPlan unrolling |
| `VPlanUtils.cpp` | VPlan utilities |
| `VPlanUtils.h` | VPlan utility interfaces |
| `VPlanValue.h` | VPlan value types |
| `VPlanVerifier.cpp` | VPlan verification and validation |
| `VPlanVerifier.h` | VPlan verifier interface |
| `VPRecipeBuilder.h` | VPlan recipe builder |
| `SandboxVectorizer` | Experimental sandbox vectorizer directory |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When working on vectorization:

1. Understand loop structure: analyze induction variables, dependencies, memory patterns
2. Use VPlan for representing vectorization strategy (modern approach)
3. Implement legality checks in LoopVectorizationLegality.cpp
4. Generate vector IR using VPlan recipes
5. Test with llvm-lit in llvm/test/Transforms/Vectorize/

### Common Patterns

- **Dependency analysis**: Check data dependencies to ensure vectorization is safe
- **VPlan construction**: Build VPlan from loop representation
- **Cost analysis**: Estimate speedup vs register pressure to decide vectorization factor
- **Predication**: Handle control flow using masked operations or branches

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, instructions, intrinsics
- `llvm/include/llvm/Analysis/LoopInfo.h` — loop analysis
- `llvm/include/llvm/Analysis/DependenceAnalysis.h` — data dependency analysis
- `llvm/include/llvm/Analysis/ScalarEvolution.h` — SCEV for loop bounds
- `llvm/include/llvm/Analysis/TargetTransformInfo.h` — cost modeling
- `llvm/lib/Analysis/` — analysis implementations
- `llvm/lib/Transforms/Utils/` — code generation helpers

### External
- Standard C++ library (containers for VPlan graphs)

<!-- MANUAL: -->
