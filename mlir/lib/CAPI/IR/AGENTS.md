<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/IR

## Purpose
Implements the core IR portion of the MLIR C API. Provides C-linkage functions for creating and manipulating `MlirContext`, `MlirOperation`, `MlirBlock`, `MlirRegion`, `MlirValue`, `MlirType`, `MlirAttribute`, `MlirLocation`, and `MlirPass` objects.

## Key Files
| File | Description |
|------|-------------|
| `IR.cpp` | Bulk of the core C API: context lifecycle, op creation/cloning/erasure, block/region management, value/type/attribute accessors, printing, verification, walking |
| `AffineExpr.cpp` | C API for `MlirAffineExpr`: creation of dim/symbol/constant/composed affine expressions |
| `AffineMap.cpp` | C API for `MlirAffineMap`: creation, composition, and querying of affine maps |
| `BuiltinAttributes.cpp` | C API for all built-in attribute kinds (integer, float, string, array, dense elements, dictionary, etc.) |
| `BuiltinTypes.cpp` | C API for all built-in type kinds (integer, float, index, tensor, memref, vector, tuple, function) |
| `Diagnostics.cpp` | C API for `MlirDiagnostic` and diagnostic handler registration |
| `DialectHandle.cpp` | C API for `MlirDialectHandle`: dialect registration and lookup via handle |
| `ExtensibleDialect.cpp` | C API for dynamic (extensible) dialects and dynamically-typed operations |
| `IntegerSet.cpp` | C API for `MlirIntegerSet`: affine integer constraint sets |
| `Pass.cpp` | C API for `MlirPassManager`, `MlirOpPassManager`: creating, populating, and running pass pipelines |
| `Support.cpp` | C API utilities: `MlirStringRef`, `MlirLogicalResult`, `MlirTypeID`, memory buffer wrappers |

## For AI Agents

### Working In This Directory
- All functions are declared in `mlir/include/mlir-c/IR.h` and friends; implementation must stay in sync with those headers.
- `wrap()` / `unwrap()` from `mlir/include/mlir/CAPI/Utils.h` convert between `MlirXxx` handles and C++ pointers.
- Never add C++-style overloading, templates, or exceptions; this must remain pure C-compatible.

### Common Patterns
- Opaque handle pattern: `struct MlirOperation { void *ptr; }` — the `.ptr` field holds a `mlir::Operation*`.
- Null checks: C API functions should handle null handles gracefully and document null behavior.

## Dependencies

### Internal
- `mlir/lib/IR/` — all C++ IR classes

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
