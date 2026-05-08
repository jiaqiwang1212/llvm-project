<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/RegisterEverything

## Purpose
Provides the C API entry point that registers all bundled MLIR dialects, passes, and extensions into an `MlirContext` / `MlirDialectRegistry` in a single call. This is the C API equivalent of `mlir::registerAllDialects()` + `mlir::registerAllPasses()`.

## Key Files
| File | Description |
|------|-------------|
| `RegisterEverything.cpp` | Implements `mlirRegisterAllDialects()`, `mlirRegisterAllPasses()`, and `mlirRegisterAllExtensions()`: the single-call registration API used by Python bindings and other embedding environments |

## For AI Agents

### Working In This Directory
- This is the C API mirror of `mlir/lib/RegisterAllDialects.cpp` etc.
- When a new dialect/pass is added to the bundled set, update both the C++ `RegisterAll*.cpp` files in `mlir/lib/` and this file.
- Declaration is in `mlir/include/mlir-c/RegisterEverything.h`.

## Dependencies

### Internal
- `mlir/lib/RegisterAllDialects.cpp`, `RegisterAllPasses.cpp`, `RegisterAllExtensions.cpp`
- `mlir/lib/CAPI/IR/` — core C API

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
