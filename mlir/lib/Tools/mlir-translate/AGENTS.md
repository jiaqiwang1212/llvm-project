<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-translate

## Purpose
Implements `MlirTranslateMain()`, the entry-point library for the `mlir-translate` tool. Dispatches to registered translation functions (MLIR → LLVM IR, MLIR → SPIR-V binary, Wasm → MLIR, etc.) based on the `--mlir-to-*` or `--*-to-mlir` flag given on the command line.

## Key Files
| File | Description |
|------|-------------|
| `MlirTranslateMain.cpp` | `mlir::MlirTranslateMain()`: parses command-line flags, looks up the registered translation function, opens input/output files, and invokes the translation |
| `Translation.cpp` | `TranslateFromMLIRRegistration` / `TranslateToMLIRRegistration`: the registration objects that bind a flag name to a translation function; also `TranslateRegistration` for bidirectional translations |

## For AI Agents

### Working In This Directory
- New translations register themselves via `TranslateFromMLIRRegistration` or `TranslateToMLIRRegistration` constructed at static init time (in their own `TranslateRegistration.cpp`).
- `mlir-translate --mlir-to-llvmir` invokes the LLVMIR translation; `--import-llvm` invokes the reverse.
- The `Translation.cpp` registry is global; registration objects must not be destroyed before main exits.

### Common Patterns
- Translation functions: `LogicalResult myTranslation(ModuleOp, raw_ostream&)` or `OwningOpRef<ModuleOp> myTranslation(SourceMgr&, MLIRContext*)`.

## Dependencies

### Internal
- `mlir/lib/Target/` — all translation implementations
- `mlir/lib/Parser/` — for to-MLIR translations

### External
- `llvm/lib/Support` — `llvm::cl`, file I/O

<!-- MANUAL: -->
