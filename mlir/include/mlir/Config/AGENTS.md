<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Config/

## Purpose
CMake-generated configuration header for MLIR. Contains compile-time feature flags and version information derived from the CMake build configuration. This directory holds the `.cmake` template; the actual `mlir-config.h` is generated into the build directory.

## Key Files
| File | Description |
|------|-------------|
| `mlir-config.h.cmake` | CMake template for `mlir-config.h`; defines `MLIR_ENABLE_*` feature macros and version strings |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Do not edit `mlir-config.h.cmake` unless adding a new CMake-controlled feature flag.
- The generated `mlir-config.h` is written to `<build>/tools/mlir/include/mlir/Config/mlir-config.h`.
- Include as `#include "mlir/Config/mlir-config.h"` — the build system adds both source and build include paths.
- New feature flags follow the pattern `#cmakedefine MLIR_ENABLE_MYFEATURE`.

### Common Patterns
- Feature guard: `#ifdef MLIR_ENABLE_CUDA_RUNNER ... #endif`
- Version check: `MLIR_VERSION_MAJOR`, `MLIR_VERSION_MINOR`, `MLIR_VERSION_PATCH`

## Dependencies

### Internal
- None

### External
- CMake build system (generates this file)

<!-- MANUAL: -->
