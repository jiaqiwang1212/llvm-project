<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/Interfaces

## Purpose
Implements C API wrappers for querying MLIR op interfaces. Allows C callers to check whether an operation implements a given interface and to invoke interface methods without depending on C++ vtable dispatch.

## Key Files
| File | Description |
|------|-------------|
| `Interfaces.cpp` | C API for interface queries: `mlirOpIsAInferTypeOpInterface`, `mlirInferTypeOpInterfaceInferReturnTypes`, etc.; covers `InferTypeOpInterface`, `InferShapedTypeOpInterface`, and similar core interfaces |

## For AI Agents

### Working In This Directory
- Add new interface C API wrappers here when a C++ interface gains C binding coverage.
- Declarations go in `mlir/include/mlir-c/Interfaces.h`.

### Common Patterns
- Pattern: `mlirOpIsA<InterfaceName>(MlirOperation op)` returns bool; `mlir<InterfaceName><Method>(...)` calls the method.

## Dependencies

### Internal
- `mlir/lib/CAPI/IR/` — `MlirOperation` unwrapping
- `mlir/lib/Interfaces/` — C++ interface implementations

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
