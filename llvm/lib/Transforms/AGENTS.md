<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# lib/Transforms

## Purpose

Root container for LLVM transformation passes — compiler optimization and analysis pipelines that optimize code at IR level. Includes peephole optimizations, scalar transforms, interprocedural optimizations, vectorization, instrumentation, and utility passes.

## Key Files

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Build configuration grouping all subdirectories |

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `AggressiveInstCombine/` | Advanced instruction combining requiring program analysis |
| `CFGuard/` | Microsoft Control Flow Guard instrumentation |
| `Coroutines/` | C++ coroutine lowering (co_await, co_yield) |
| `HipStdPar/` | HIP/CUDA standard parallelism offloading |
| `InstCombine/` | Core peephole optimization pass |
| `Instrumentation/` | Code instrumentation for sanitizers, profiling, coverage |
| `IPO/` | Inter-procedural optimizations |
| `ObjCARC/` | Objective-C automatic reference counting optimization |
| `Scalar/` | Scalar optimizations (SROA, GVN, LICM, unrolling, etc.) |
| `Utils/` | Utility passes and transformation helpers |
| `Vectorize/` | Auto-vectorization (loop and SLP vectorization) |

## For AI Agents

### Working In This Directory

This directory is the root module for all transformation passes. Most work happens in subdirectories. When adding a new pass:

1. Create a new subdirectory with CMakeLists.txt and implementation files
2. Register the pass in lib/Passes/PassRegistry.def
3. Add it to the appropriate pipeline in PassBuilder.cpp
4. Create include/llvm/Transforms/YourPass.h if it's public

### Common Patterns

- **Pass registration**: Use FUNCTION_PASS, MODULE_PASS, LOOP_PASS macros in include/llvm/Transforms/
- **Pass dependencies**: Declare required analyses via getAnalysisUsage()
- **IR traversal**: Use visitor patterns (InstVisitor, ModulePass, FunctionPass)
- **Optimization validation**: Use LLVM_DEBUG and assertions to verify IR properties

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR types, instructions, values
- `llvm/include/llvm/Analysis/` — analysis passes (DominatorTree, LoopInfo, etc.)
- `llvm/include/llvm/Transforms/` — pass interfaces
- `llvm/lib/Analysis/` — analysis implementations
- `llvm/lib/Passes/` — pass manager and pipeline building

### External
- Standard C++ library (std::vector, std::unordered_map, etc.)

<!-- MANUAL: -->
