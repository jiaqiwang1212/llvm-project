<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/Transforms

## Purpose
Implements C API wrappers for MLIR transformation passes and the rewrite pattern infrastructure. Exposes pass registration and rewrite utilities to C callers.

## Key Files
| File | Description |
|------|-------------|
| `Passes.cpp` | Calls all `mlirRegisterTransformsXxxPass()` registration functions; exposes `mlirRegisterTransformsPasses()` as the bulk registration entry point |
| `Rewrite.cpp` | C API for `MlirRewriterBase`, `MlirFrozenRewritePatternSet`, `MlirGreedyRewriteDriverConfig`: wraps the C++ greedy pattern rewrite driver for use from C |

## For AI Agents

### Working In This Directory
- `Rewrite.cpp` provides the C-level interface to `applyPatternsGreedily()`; changes to the C++ driver API require updating this wrapper.
- Declarations are in `mlir/include/mlir-c/Transforms.h` and `mlir/include/mlir-c/Rewrite.h`.

## Dependencies

### Internal
- `mlir/lib/Transforms/` and `mlir/lib/Rewrite/` — C++ implementations
- `mlir/lib/CAPI/IR/` — core C API utilities

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
