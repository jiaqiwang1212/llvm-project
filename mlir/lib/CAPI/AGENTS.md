<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI

## Purpose
Implements the stable C API for MLIR. The C API wraps the C++ implementation behind an ABI-stable C interface, enabling language bindings (Python, Rust, Julia, etc.) and embedding MLIR in C-based build systems without exposing C++ ABI details. Each subdirectory corresponds to a logical grouping of the C++ API.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core IR C API: `MlirContext`, `MlirOperation`, `MlirType`, `MlirAttribute`, `MlirBlock`, `MlirRegion`, `MlirValue` (see `IR/AGENTS.md`) |
| `Interfaces/` | C API for querying op interfaces (see `Interfaces/AGENTS.md`) |
| `Dialect/` | Per-dialect C API wrappers for all bundled dialects (see `Dialect/AGENTS.md`) |
| `Conversion/` | C API for conversion pass registration (see `Conversion/AGENTS.md`) |
| `Transforms/` | C API for transformation pass registration and rewrite utilities (see `Transforms/AGENTS.md`) |
| `Target/` | C API for translation targets (LLVM IR export, SMTLIB) (see `Target/AGENTS.md`) |
| `ExecutionEngine/` | C API for `MlirExecutionEngine` (JIT compilation and invocation) (see `ExecutionEngine/AGENTS.md`) |
| `Debug/` | C API for debug/action infrastructure (see `Debug/AGENTS.md`) |
| `RegisterEverything/` | C API entry point to register all bundled dialects, passes, and extensions (see `RegisterEverything/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- C API functions follow the naming convention `mlirXxxYyy()` where `Xxx` is the type and `Yyy` is the operation.
- C API types are opaque `struct { void *ptr; }` wrappers; never expose internal pointers directly.
- Public headers are in `mlir/include/mlir-c/`; this directory contains only the `.cpp` implementations.
- ABI stability is a hard requirement: never change the layout of existing `MlirXxx` structs or remove/rename functions. Use versioned additions instead.
- The Python bindings in `mlir/lib/Bindings/Python/` depend heavily on this C API.

### Common Patterns
- Conversion between C and C++ types uses `unwrap()` / `wrap()` helpers declared in `mlir/include/mlir/CAPI/Utils.h`.
- Error reporting uses `MlirDiagnostic` callbacks rather than C++ exceptions.

## Dependencies

### Internal
- `mlir/lib/IR/` — all core IR
- `mlir/lib/Pass/` — pass manager
- `mlir/lib/Transforms/` — transformation passes
- `mlir/lib/ExecutionEngine/` — JIT engine

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
