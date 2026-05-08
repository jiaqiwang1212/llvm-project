<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/ExecutionEngine

## Purpose
Implements the C API for the MLIR JIT execution engine. Allows C and Python callers to compile an MLIR module to native code and invoke exported functions at runtime.

## Key Files
| File | Description |
|------|-------------|
| `ExecutionEngine.cpp` | C API: `mlirExecutionEngineCreate()`, `mlirExecutionEngineDestroy()`, `mlirExecutionEngineLookup()`, `mlirExecutionEngineInvokePacked()`, `mlirExecutionEngineDumpToObjectFile()` |

## For AI Agents

### Working In This Directory
- The execution engine requires LLVM JIT infrastructure (ORC JIT); it is only available in builds with LLVM JIT enabled.
- Declaration is in `mlir/include/mlir-c/ExecutionEngine.h`.

## Dependencies

### Internal
- `mlir/lib/ExecutionEngine/ExecutionEngine.cpp` — C++ JIT engine
- `mlir/lib/CAPI/IR/` — core C API

### External
- LLVM ORC JIT (`llvm/lib/ExecutionEngine/`)

<!-- MANUAL: -->
