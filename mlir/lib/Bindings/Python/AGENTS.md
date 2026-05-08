<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Bindings/Python

## Purpose
Implements the MLIR Python bindings using pybind11. Exposes the MLIR IR (context, operations, types, attributes, passes, execution engine) and all bundled dialects as importable Python modules. These bindings are used by downstream projects like IREE, torch-mlir, and the MLIR Python integration tests.

## Key Files
| File | Description |
|------|-------------|
| (Contents vary; directory contains per-module `.cpp` files for core IR, dialects, passes, and the execution engine, plus Python `.py` glue modules) |  |

## For AI Agents

### Working In This Directory
- Python bindings files are generated in part by TableGen (`mlir-tblgen --gen-python-bindings`); regenerate with `cmake --build <build_dir> --target mlir-python-sources` before editing the generated portions.
- Core binding modules are in `mlir/_mlir_libs/`; hand-written Python wrappers live in `mlir/python/mlir/`.
- When adding a new dialect's Python bindings, add both the `.cpp` pybind11 file here and the corresponding Python `.py` wrapper in `mlir/python/mlir/dialects/`.
- Test Python bindings via `python -m pytest mlir/test/python/`.

### Common Patterns
- Dialect bindings register with `MlirPythonExtension` and call `mlirPythonCapsuleToContext()` / `mlirPythonContextToCapsule()` for capsule interop.
- Custom attribute/type Python classes use `mlir.ir.Type._CAPICreate()` to wrap C API handles.

## Dependencies

### Internal
- `mlir/lib/CAPI/` — entire C API surface

### External
- pybind11, Python C API

<!-- MANUAL: -->
