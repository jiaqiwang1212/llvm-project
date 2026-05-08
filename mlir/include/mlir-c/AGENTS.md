<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir-c/

## Purpose
Stable C API headers for MLIR. These headers expose MLIR functionality through an ABI-stable C interface suitable for use from non-C++ languages (Python, Rust, Swift, etc.) and for embedding MLIR in projects that cannot depend on the C++ ABI. All types are opaque structs wrapped in `MlirXxx` typedefs, and all functions use `MLIR_CAPI_EXPORTED`.

## Key Files
| File | Description |
|------|-------------|
| `IR.h` | Core IR C API: context, module, operation, region, block, value, type, attribute |
| `AffineExpr.h` | C API for affine expressions |
| `AffineMap.h` | C API for affine maps |
| `BuiltinAttributes.h` | C API for builtin attribute types (integer, float, string, array, etc.) |
| `BuiltinTypes.h` | C API for builtin types (integer, float, index, memref, tensor, etc.) |
| `Diagnostics.h` | C API for the diagnostics engine |
| `Interfaces.h` | C API for op interface queries |
| `IntegerSet.h` | C API for integer sets |
| `Pass.h` | C API for pass manager and passes |
| `Rewrite.h` | C API for pattern rewrite framework |
| `Support.h` | C API support utilities (MlirStringRef, MlirLogicalResult, etc.) |
| `Transforms.h` | C API for common transforms |
| `Debug.h` | C API for the debug/action framework |
| `ExecutionEngine.h` | C API for the JIT execution engine |
| `Conversion.h` | C API for dialect conversion |
| `RegisterEverything.h` | Register all dialects and passes via C API |
| `ExtensibleDialect.h` | C API for dynamically extensible dialects |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Dialect/` | Per-dialect C API headers (one `.h` per bundled dialect) |
| `Target/` | C API for translation targets (LLVM IR export, etc.) |

## For AI Agents

### Working In This Directory
- Every new C API function must use `MLIR_CAPI_EXPORTED` and follow the `mlirXxxYyy` naming convention.
- Opaque types are defined with `MLIR_DEFINE_CXX_EQUIVALENT` macros in `Wrap.h`.
- C API headers must not include any C++ headers — they must be valid C99.
- Implementation lives in `mlir/lib/CAPI/`; these are declaration-only headers.

### Common Patterns
- Opaque handle types: `typedef struct { void *ptr; } MlirFoo;`
- Null checks: `mlirFooIsNull(MlirFoo foo)`
- String passing: `MlirStringRef` (pointer + length, not null-terminated)
- Callbacks use function pointer + `void *userData` pairs

## Dependencies

### Internal
- `mlir/include/mlir/CAPI/` (wrap/unwrap helpers used in implementations)

### External
- Standard C headers (`<stdint.h>`, `<stddef.h>`, `<stdbool.h>`)
- `mlir-c/Support.h` (base types, included by all other mlir-c headers)

<!-- MANUAL: -->
