<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/CAPI/Target

## Purpose
Implements C API wrappers for MLIR translation targets, allowing C callers to invoke module-to-target translation (e.g., export MLIR to LLVM IR bittext or SMTLIB).

## Key Files
| File | Description |
|------|-------------|
| `LLVMIR.cpp` | C API for `mlirTranslateModuleToLLVMIR()`: exports an MLIR module containing LLVM dialect ops to an `LLVMModuleRef` (LLVM-C IR handle) |
| `ExportSMTLIB.cpp` | C API for exporting SMT dialect ops to SMTLIB text format |

## For AI Agents

### Working In This Directory
- `mlirTranslateModuleToLLVMIR` requires the LLVM dialect to be loaded and the module to be lowered to LLVM dialect first.
- Declarations are in `mlir/include/mlir-c/Target/LLVMIR.h` and `mlir/include/mlir-c/Target/SMTLIB.h`.

## Dependencies

### Internal
- `mlir/lib/Target/LLVMIR/` — C++ translation implementation
- `mlir/lib/Target/SMTLIB/` — SMTLIB export
- `mlir/lib/CAPI/IR/` — core C API

### External
- `llvm/lib/Support`, LLVM-C API (`llvm-c/Core.h`)

<!-- MANUAL: -->
