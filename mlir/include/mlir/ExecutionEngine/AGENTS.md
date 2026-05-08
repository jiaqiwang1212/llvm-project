<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/ExecutionEngine/

## Purpose
JIT execution engine and runtime support headers for MLIR. Provides the `ExecutionEngine` for JIT-compiling MLIR modules to native code via LLVM ORC JIT, runtime utility headers (`CRunnerUtils.h`, `RunnerUtils.h`) that are compiled into test runners, and sparse tensor runtime support. Also includes the async runtime for async dialect lowering.

## Key Files
| File | Description |
|------|-------------|
| `ExecutionEngine.h` | `ExecutionEngine` — JIT compile an MLIR module and look up function pointers |
| `JitRunner.h` | `JitRunnerMain` — entry point for `mlir-cpu-runner`-style JIT runner tools |
| `CRunnerUtils.h` | C runtime utilities: `MemRef` descriptor structs, print helpers (included in runner binaries) |
| `RunnerUtils.h` | Higher-level runner utilities built on top of `CRunnerUtils.h` |
| `MemRefUtils.h` | `OwningMemRef<T>` — RAII owner for MemRef descriptors |
| `AsyncRuntime.h` | Runtime for the async dialect: tokens, values, groups |
| `Float16bits.h` | `f16` / `bf16` bit representations for runtime use |
| `OptUtils.h` | LLVM optimization pipeline helpers for the JIT |
| `SparseTensorRuntime.h` | Sparse tensor runtime functions called by lowered sparse ops |
| `Msan.h` | MemorySanitizer integration helpers |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `SparseTensor/` | Sparse tensor storage format headers used by the sparse runtime (see `SparseTensor/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `ExecutionEngine` wraps LLVM ORC JIT; call `engine->lookup("func")` to get a function pointer.
- `CRunnerUtils.h` and `RunnerUtils.h` are included in compiled runner objects, not just headers — they contain function definitions.
- Sparse tensor runtime headers define the ABI between lowered MLIR code and the runtime library.
- `AsyncRuntime.h` defines the ABI for the async dialect's lowered coroutine-style code.

### Common Patterns
- JIT compile: `ExecutionEngine::create(module, options)` returns an `llvm::Expected<ExecutionEngine>`.
- Function lookup: `engine->lookupPacked("main")` for functions taking/returning packed structs.
- MemRef ABI: `StridedMemRefType<T, N>` matches the lowered MemRef descriptor struct layout.

## Dependencies

### Internal
- `mlir/IR/` (Module, MLIRContext)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ExecutionEngine/` (ORC JIT)
- `llvm/IR/` (Module, LLVMContext)

<!-- MANUAL: -->
