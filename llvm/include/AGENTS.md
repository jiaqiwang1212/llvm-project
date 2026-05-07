<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include

## Purpose
Public header files for the LLVM core library. Code consuming LLVM as a library includes from here. Split into `llvm/` (C++ API) and `llvm-c/` (stable C API).

## Subdirectories

| Directory | Purpose |
|-----------|---------|
| `llvm/` | C++ public API headers organized by component (see `llvm/AGENTS.md`) |
| `llvm-c/` | Stable C API headers for language bindings and embedding (see `llvm-c/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Changes to headers here affect ABI; prefer additive changes and avoid removing or reordering fields in public structs
- The C API (`llvm-c/`) has stricter stability guarantees than the C++ API
- Headers use `#pragma once` or include guards consistently

### Common Patterns
- Forward declarations live in `llvm/Support/Casting.h` and per-component forward-declaration headers
- Use `LLVM_ABI` / `LLVM_LIBRARY_VISIBILITY` macros for symbol visibility

<!-- MANUAL: -->
