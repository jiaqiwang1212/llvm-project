<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/

## Purpose
Translation target headers for converting MLIR IR to external formats. Each subdirectory covers one target: LLVM IR (the primary backend), SPIR-V binary, C/C++ source emission, WebAssembly, SMTLIB, and IRDL-to-C++. These are translation entry points, not pass-based lowerings — they produce external artifacts directly from MLIR modules.

## Key Files
(none at top level — all content is in subdirectories)

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `LLVMIR/` | Export MLIR to LLVM IR (`llvm::Module`) and import LLVM IR to MLIR (see `LLVMIR/AGENTS.md`) |
| `SPIRV/` | Serialize/deserialize SPIR-V binary and export MLIR SPIR-V dialect (see `SPIRV/AGENTS.md`) |
| `Cpp/` | Emit C/C++ source from MLIR EmitC dialect (see `Cpp/AGENTS.md`) |
| `IRDLToCpp/` | Translate IRDL dialect definitions to C++ (see `IRDLToCpp/AGENTS.md`) |
| `SMTLIB/` | Export MLIR SMT dialect to SMTLIB2 format (see `SMTLIB/AGENTS.md`) |
| `Wasm/` | WebAssembly binary import support (see `Wasm/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Translations are registered via `mlir::registerTranslation()` and invoked by `mlir-translate`.
- Each target subdirectory contains both import (external format → MLIR) and/or export (MLIR → external format) functions.
- Unlike passes, translations do not use the pass manager; they operate directly on `ModuleOp`.

### Common Patterns
- Export function signature: `LogicalResult mlirModuleToXxx(ModuleOp module, raw_ostream &os)`.
- Registration: `mlir::TranslateFromMLIRRegistration reg("mlir-to-xxx", "...", translateFn, registerFn)`.

## Dependencies

### Internal
- `mlir/IR/` (ModuleOp, MLIRContext)
- `mlir/Support/` (LogicalResult)

### External
- Target-specific: `llvm/IR/` for LLVM, SPIR-V headers for SPIR-V, etc.

<!-- MANUAL: -->
