<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/ExecutionEngine/SparseTensor

## Purpose
Implements the sparse tensor runtime library. This is a standalone C runtime shared library (`libmlir_sparse_tensor`) that provides storage management, format conversion, file I/O, and arithmetic support for SparseTensor dialect lowerings at JIT/AOT execution time.

## Key Files
| File | Description |
|------|-------------|
| `File.cpp` | Sparse tensor file I/O: reading and writing sparse tensors in coordinate format (`.mtx` Matrix Market, Frostt `.tns`) |
| `MapRef.cpp` | `MapRef`: lightweight view over affine dimension ordering maps used during sparse tensor construction |
| `Storage.cpp` | `SparseTensorStorage<P, I, V>`: the core sparse tensor storage class managing compressed dimension levels (dense, compressed, singleton, block); implements level iteration, insertion, and conversion between formats |

## For AI Agents

### Working In This Directory
- This library must compile without MLIR headers (it is a pure C++ runtime library with `extern "C"` entry points); do not introduce MLIR IR dependencies.
- Entry points are declared in `mlir/include/mlir/ExecutionEngine/SparseTensor/` headers.
- The main `SparseTensorRuntime.cpp` in the parent directory (`mlir/lib/ExecutionEngine/`) is the `extern "C"` interface that delegates to these classes.

### Common Patterns
- Storage templates are instantiated for common `<P, I, V>` combinations (e.g., `<uint64_t, uint64_t, float>`).
- Level types are encoded as `DimLevelType` enum values.

## Dependencies

### Internal
- `mlir/lib/ExecutionEngine/SparseTensorRuntime.cpp` — entry point wrapper

### External
- `llvm/lib/Support` — `llvm::SmallVector`, `llvm::raw_ostream`
- Standard C++ library only (no MLIR IR)

<!-- MANUAL: -->
