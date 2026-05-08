<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Async Dialect

## Purpose
Implements the Async dialect for expressing asynchronous execution in MLIR. Models async regions, token-based synchronization, and value passing across async boundaries. Provides the foundation for multi-threaded execution via the async runtime.

## Key Files
| File | Description |
|------|-------------|
| `IR/Async.cpp` | Op implementations: `async.execute`, `async.await`, `async.yield`, token/value types |
| `Transforms/AsyncToAsyncRuntime.cpp` | Lowers `async.execute` regions to async runtime function calls |
| `Transforms/AsyncParallelFor.cpp` | Converts parallel loops to async tasks |
| `Transforms/AsyncRuntimeRefCounting.cpp` | Inserts reference counting for async tokens/values |
| `Transforms/AsyncRuntimeRefCountingOpt.cpp` | Optimizes ref-count operations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Async ops and types (token, value, group) |
| `Transforms/` | Lowering to runtime calls, parallel-for, ref-counting |

## For AI Agents

### Working In This Directory
- `async.execute` creates a new async task; it returns an `!async.token` and zero or more `!async.value<T>`.
- The lowering pipeline: `async.execute` → coroutine-style functions → runtime API calls (mlirAsyncRuntimeCreate*).
- `PassDetail.h/.cpp` in Transforms are pass infrastructure helpers — they handle pass option registration.
- Ref-counting passes must run after runtime lowering but before final LLVM conversion.

### Common Patterns
- `AsyncToAsyncRuntime` uses a function outliner to lift `async.execute` body into a separate coroutine function.
- `AsyncParallelFor` detects `scf.parallel` and replaces it with a recursive async task tree.

## Dependencies
- `mlir/Dialect/Func`, `mlir/Dialect/SCF`, `mlir/Dialect/LLVMIR`, LLVM coroutines support

<!-- MANUAL: -->
