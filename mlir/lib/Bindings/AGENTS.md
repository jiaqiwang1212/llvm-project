<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Bindings

## Purpose
Implements language bindings for MLIR. Currently contains only Python bindings via pybind11, which expose the full MLIR C API (and some C++ API) to Python. These bindings power `import mlir` and are the foundation for the MLIR Python ecosystem.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Python/` | pybind11-based Python bindings for MLIR IR, passes, dialects, and execution engine (see `Python/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Python bindings depend on the C API in `mlir/lib/CAPI/`; C API must be complete before adding Python coverage.
- The Python bindings are built as a shared library (`MLIRPythonCAPI`); ABI stability matters.
- New language bindings (Rust, Julia, etc.) would be added as new subdirectories here.

### Common Patterns
- Python binding source files use pybind11 `PYBIND11_MODULE` macros.
- Dialect-specific Python bindings (custom types, attributes) live under `mlir/lib/Bindings/Python/` with per-dialect `.cpp` files.

## Dependencies

### Internal
- `mlir/lib/CAPI/` — all C API

### External
- pybind11
- Python C API

<!-- MANUAL: -->
