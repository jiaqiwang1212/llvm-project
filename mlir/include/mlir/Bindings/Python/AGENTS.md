<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Bindings/Python/

## Purpose
Nanobind-based Python binding infrastructure headers for MLIR. Provides the C++ glue layer for implementing Python bindings: IR object adaptors, diagnostic integration, interface bindings, type/attribute accessor helpers, and TableGen-driven binding generation utilities.

## Key Files
| File | Description |
|------|-------------|
| `IRCore.h` | Core IR Python binding types: `PyMlirContext`, `PyOperation`, `PyType`, `PyAttribute`, `PyValue` |
| `IRTypes.h` | Python binding helpers for builtin type classes |
| `IRAttributes.h` | Python binding helpers for builtin attribute classes |
| `IRInterfaces.h` | Python binding infrastructure for op interfaces |
| `Diagnostics.h` | Python diagnostic handler integration |
| `Globals.h` | Global state for Python bindings (context cache, etc.) |
| `NanobindAdaptors.h` | Nanobind adaptors: automatic wrap/unwrap of MLIR C API handles |
| `NanobindUtils.h` | Utility functions for nanobind bindings |
| `Nanobind.h` | Convenience header including nanobind and common adaptors |
| `PythonBindings.td` | TableGen definitions for generating Python binding stubs |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Python bindings use the C API (`mlir-c/`) as the ABI boundary, not the C++ API.
- `NanobindAdaptors.h` provides `mlirApiObjectToCapsu()` and related helpers for C API handle exchange.
- `IRCore.h` defines the primary Python-visible MLIR objects; these map to `MlirXxx` C API types.
- New dialect Python bindings should implement a `registerXxxDialectBinding(nb::module_)` function.
- `PythonBindings.td` is used to auto-generate `_ods_ir` accessor code for op attributes.

### Common Patterns
- Module binding: `void populateXxxSubmodule(nb::module_ &m) { m.def("create_op", ...); }`
- Adaptor usage: `auto *op = unwrap(mlirOperation); /* use C++ API */ return wrap(op);`
- TableGen-driven: `include "mlir/Bindings/Python/PythonBindings.td"` in dialect `.td` for auto-generated accessors.

## Dependencies

### Internal
- `mlir-c/` (all C API headers)
- `mlir/IR/` (transitively, via C API)

### External
- `nanobind/nanobind.h`
- Python C API (`Python.h`)

<!-- MANUAL: -->
