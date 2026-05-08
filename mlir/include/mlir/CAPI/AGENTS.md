<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/CAPI/

## Purpose
Internal C++ helpers for implementing the MLIR C API (declared in `mlir-c/`). Provides wrap/unwrap macros and inline functions that safely cast between the opaque C API handle types (`MlirXxx`) and their internal C++ counterparts. These headers are used exclusively by C API implementation files and should not be included by general MLIR C++ code.

## Key Files
| File | Description |
|------|-------------|
| `IR.h` | Wrap/unwrap for core IR types: MlirContext, MlirOperation, MlirType, MlirAttribute, MlirValue, MlirBlock, MlirRegion |
| `AffineExpr.h` | Wrap/unwrap for MlirAffineExpr |
| `AffineMap.h` | Wrap/unwrap for MlirAffineMap |
| `Diagnostics.h` | Wrap/unwrap for MlirDiagnostic |
| `Interfaces.h` | Wrap/unwrap helpers for interface queries |
| `IntegerSet.h` | Wrap/unwrap for MlirIntegerSet |
| `Pass.h` | Wrap/unwrap for MlirPass, MlirPassManager |
| `Registration.h` | Dialect and pass registration helpers for C API |
| `Rewrite.h` | Wrap/unwrap for rewrite pattern and applicator handles |
| `Support.h` | `MLIR_DEFINE_CXX_EQUIVALENT`, `unwrap()`/`wrap()` base templates |
| `Utils.h` | Miscellaneous C API implementation utilities |
| `Wrap.h` | Core wrap/unwrap macro definitions |
| `ExecutionEngine.h` | Wrap/unwrap for MlirExecutionEngine |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Dialect/` | Wrap/unwrap helpers for per-dialect C API types |

## For AI Agents

### Working In This Directory
- These headers are **implementation-only** — never include them from public MLIR C++ headers.
- `wrap(CppObj)` converts a C++ pointer/reference to the corresponding `MlirXxx` opaque handle.
- `unwrap(MlirXxx)` converts the opaque handle back to the C++ type.
- Null safety: always check `mlirXxxIsNull()` on the C side before unwrapping.

### Common Patterns
- `MLIR_DEFINE_CXX_EQUIVALENT(MlirFoo, mlir::Foo)` ties the C and C++ types together.
- Implementation files include the CAPI header to get wrap/unwrap, then use normal C++ MLIR APIs.

## Dependencies

### Internal
- `mlir/IR/` (all core types being wrapped)
- `mlir/Pass/` (PassManager wrapping)

### External
- `mlir-c/Support.h` (C API base types)

<!-- MANUAL: -->
