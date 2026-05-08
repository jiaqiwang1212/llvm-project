<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Bindings/

## Purpose
Language binding support headers for MLIR. Currently contains the Python binding infrastructure using nanobind. These headers provide the glue layer between the MLIR C API and higher-level language bindings, including IR adaptors, diagnostics, interfaces, and TableGen-driven binding generation.

## Key Files
(none at top level — all content is in subdirectories)

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Python/` | Nanobind-based Python binding helpers and adaptors (see `Python/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- This directory is currently Python-only; future language bindings would add parallel subdirectories.
- Python bindings are built on top of the C API (`mlir-c/`) rather than the C++ API directly.
- Do not add C++ MLIR headers here — bindings consume the stable C API.

### Common Patterns
- Python binding headers use nanobind (`#include <nanobind/nanobind.h>`).
- New dialect bindings follow the pattern in `mlir/lib/Bindings/Python/`.

## Dependencies

### Internal
- `mlir-c/` (C API consumed by Python bindings)

### External
- `nanobind/` (Python/C++ binding library)

<!-- MANUAL: -->
