<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/Conversion

## Purpose
Implements C API wrappers for registering MLIR conversion passes (dialect-to-dialect lowering passes). Exposes `mlirRegisterConversionPasses()` so C and Python callers can populate a pass manager with conversion passes by name.

## Key Files
| File | Description |
|------|-------------|
| `Passes.cpp` | Calls all `mlirRegisterXxxToYyyPass()` registration functions generated from conversion pass `.td` files; exposes `mlirRegisterConversionPasses()` as the single registration entry point |

## For AI Agents

### Working In This Directory
- Add a new `mlirRegisterXxxToYyyPass()` call here when a new conversion pass is added under `mlir/lib/Conversion/`.
- Declaration is in `mlir/include/mlir-c/Conversion.h`.

## Dependencies

### Internal
- `mlir/lib/Conversion/` — conversion pass implementations
- `mlir/lib/CAPI/IR/` — core C API

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
