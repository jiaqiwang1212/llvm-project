<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/

## Purpose
Top-level include directory for the MLIR project. Contains all public C++ headers, TableGen definitions, and C API headers that constitute the MLIR library interface. This directory is the root of the MLIR public API surface, organized into the `mlir/` C++ namespace headers and the `mlir-c/` stable C API headers.

## Key Files
| File | Description |
|------|-------------|
| (none at root) | All content lives in subdirectories |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `mlir/` | C++ headers for the full MLIR framework (see `mlir/AGENTS.md`) |
| `mlir-c/` | Stable C API headers for embedding MLIR in non-C++ projects (see `mlir-c/AGENTS.md`) |

## For AI Agents

### Working In This Directory
Do not add files directly to `include/`. New headers belong in `include/mlir/` (C++ API) or `include/mlir-c/` (C API). Both subdirectories have their own CMakeLists.txt-driven install rules.

### Common Patterns
- C++ headers use `#pragma once` or include guards of the form `MLIR_<SUBDIR>_<FILE>_H`
- TableGen files (`.td`) define ops, types, attributes, interfaces, and passes; they are processed by `mlir-tblgen` to generate `.h.inc` and `.cpp.inc` files
- C API headers in `mlir-c/` use `MLIR_CAPI_EXPORTED` for symbol visibility and opaque pointer types

## Dependencies

### Internal
- None (this is the root include tree)

### External
- `llvm/` headers (LLVM core utilities, ADT, Support)

<!-- MANUAL: -->
