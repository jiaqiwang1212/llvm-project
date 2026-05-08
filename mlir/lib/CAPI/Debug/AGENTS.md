<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/Debug

## Purpose
Implements C API wrappers for the MLIR action-based debug infrastructure. Allows C callers to register action handlers and control debug execution contexts.

## Key Files
| File | Description |
|------|-------------|
| `Debug.cpp` | C API for `mlirContextEnableMultithreading()`, action handler registration (`mlirContextAttachDiagnosticHandler`), and debug context queries |

## For AI Agents

### Working In This Directory
- Declaration is in `mlir/include/mlir-c/Debug.h`.
- Extends the core diagnostic/debug infrastructure from `mlir/lib/Debug/`.

## Dependencies

### Internal
- `mlir/lib/Debug/` — C++ debug infrastructure
- `mlir/lib/CAPI/IR/` — core C API

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
