<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Coroutines

## Purpose

C++20 coroutine lowering and optimization passes. Transforms coroutine intrinsics (co_await, co_yield, co_return) into state machines with suspension frames, frame allocation, and coroutine control flow.

## Key Files

| File | Description |
|------|-------------|
| `Coroutines.cpp` | Main pass entry point and coroutine detection |
| `CoroSplit.cpp` | Splits coroutine into resume/destroy/cleanup functions |
| `CoroFrame.cpp` | Builds coroutine frame layout and allocates spill storage |
| `CoroElide.cpp` | Elides (removes) unnecessary coroutine allocations |
| `CoroCleanup.cpp` | Cleans up coroutine intrinsics after lowering |
| `CoroEarly.cpp` | Early IR lowering of coroutine intrinsics |
| `CoroConditionalWrapper.cpp` | Handles conditional coroutine patterns |
| `CoroAnnotationElide.cpp` | Annotation-based coroutine elision |
| `CoroInternal.h` | Internal data structures and helpers |
| `CoroCloner.h` | Coroutine function cloning utilities |
| `MaterializationUtils.cpp` | Frame materialization helpers |
| `SpillUtils.cpp` | Register spilling and frame slot allocation |
| `SuspendCrossingInfo.cpp` | Analysis of suspend point crossing |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When working on coroutine transformations:

1. Understand coroutine intrinsics: llvm.coro.begin, llvm.coro.suspend, llvm.coro.resume, llvm.coro.destroy
2. Use CoroFrame.cpp for frame layout decisions
3. Update CoroSplit.cpp for control flow splitting
4. Add optimization passes before CoroCleanup.cpp cleans up IR
5. Test with C++20 coroutine test suite in clang/test/

### Common Patterns

- **Frame layout**: Analyze suspend points to determine what state to store
- **State machine construction**: Build resume/destroy functions from original coroutine body
- **Suspension crossing**: Detect which values cross suspend points (spilled to frame)
- **Intrinsic lowering**: Transform coro intrinsics to concrete memory operations

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, types, intrinsics
- `llvm/include/llvm/Analysis/LoopInfo.h` — loop analysis for suspend points
- `llvm/include/llvm/Transforms/Coroutines/` — coroutine pass interfaces
- `llvm/lib/Analysis/` — analysis implementations

### External
- Standard C++ library (containers for frame analysis)

<!-- MANUAL: -->
