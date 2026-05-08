<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target

## Purpose
Implements translation targets — code paths that convert MLIR IR to or from external formats (LLVM IR bitcode, SPIR-V binary, C++, WebAssembly, SMTLIB). Each subdirectory handles one target format. Unlike conversion passes (which stay in the MLIR IR world), targets produce or consume non-MLIR artifacts.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `LLVMIR/` | Bidirectional translation between MLIR LLVM dialect and LLVM IR (`llvm::Module`); the primary lowering path to native code (see `LLVMIR/AGENTS.md`) |
| `LLVM/` | Common infrastructure for LLVM-based targets: `ModuleToObject` for LLVM IR → object file, plus NVVM/ROCDL/XeVM specializations (see `LLVM/AGENTS.md`) |
| `SPIRV/` | SPIR-V binary serialization and deserialization for the MLIR SPIR-V dialect (see `SPIRV/AGENTS.md`) |
| `Cpp/` | Translates MLIR EmitC dialect ops to C++ source text (see `Cpp/AGENTS.md`) |
| `IRDLToCpp/` | Translates IRDL dialect definitions to C++ dialect boilerplate (see `IRDLToCpp/AGENTS.md`) |
| `SMTLIB/` | Exports SMT dialect ops to SMTLIB2 text format (see `SMTLIB/AGENTS.md`) |
| `Wasm/` | Imports WebAssembly binary modules into the WasmSSA MLIR dialect (see `Wasm/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Translations are registered with `TranslateFromMLIRRegistration` / `TranslateToMLIRRegistration` and invoked by `mlir-translate`.
- Each subdirectory has its own `TranslateRegistration.cpp` that calls the registration constructor.
- Adding a new target: create a subdirectory, implement the translation, add a `TranslateRegistration.cpp`, and add the subdirectory to `CMakeLists.txt`.

### Common Patterns
- Translation functions have signature `LogicalResult translateXxx(Operation*, llvm::raw_ostream&)` for to-external, or `OwningOpRef<ModuleOp> translateXxx(llvm::SourceMgr&, MLIRContext*)` for from-external.
- Translations are separated from passes; they are not part of the pass pipeline.

## Dependencies

### Internal
- `mlir/lib/IR/` — IR traversal
- `mlir/lib/Dialect/` — dialect-specific ops consumed/produced by translations

### External
- LLVM IR library, SPIR-V headers, wabt (WebAssembly), format-specific libraries

<!-- MANUAL: -->
