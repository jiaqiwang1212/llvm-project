<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AsyncToLLVM Conversion

## Purpose
Lowers the Async dialect (after `AsyncToAsyncRuntime` has lowered `async.execute` to coroutine-style functions) to LLVM IR. Converts async runtime type/function declarations and coroutine resume/destroy calls to LLVM function calls into the async runtime library.

## Key Files
| File | Description |
|------|-------------|
| `AsyncToLLVM.cpp` | Patterns lowering async runtime ops and types to LLVM calls |

## For AI Agents

### Working In This Directory
- This pass runs after `--async-to-async-runtime` which converts `async.execute` to coroutine functions.
- Lowers `!async.token`, `!async.value<T>`, `!async.group` to opaque `!llvm.ptr` runtime objects.
- Async runtime API calls (`mlirAsyncRuntimeAddTokenToGroup`, `mlirAsyncRuntimeAwaitAllInGroupAndExecute`, etc.) are emitted as `llvm.call` ops.
- Coroutine passes (`llvm.coro.*`) must run after this conversion to optimize the generated coroutines.

## Dependencies
- Source: `mlir/Dialect/Async`
- Target: `mlir/Dialect/LLVMIR`

<!-- MANUAL: -->
