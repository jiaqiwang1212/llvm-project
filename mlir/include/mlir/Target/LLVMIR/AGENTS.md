<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/LLVMIR/

## Purpose
Headers for bidirectional translation between MLIR and LLVM IR. Provides both export (MLIR LLVM dialect → `llvm::Module`) and import (LLVM IR → MLIR LLVM dialect) paths, along with dialect interfaces for customizing translation behavior and type conversion utilities.

## Key Files
| File | Description |
|------|-------------|
| `Export.h` | `translateModuleToLLVMIR()` — export MLIR module to `llvm::Module` |
| `Import.h` | `translateLLVMIRToMLIR()` — import `llvm::Module` into MLIR |
| `ModuleTranslation.h` | `ModuleTranslation` — stateful MLIR-to-LLVM IR translation driver |
| `ModuleImport.h` | `ModuleImport` — stateful LLVM IR-to-MLIR import driver |
| `LLVMTranslationInterface.h` | C++ header for `LLVMTranslationDialectInterface` |
| `LLVMTranslationDialectInterface.td` | TableGen definition of per-dialect LLVM translation hooks |
| `LLVMImportInterface.h` | C++ header for `LLVMImportDialectInterface` |
| `LLVMImportDialectInterface.td` | TableGen definition of per-dialect LLVM import hooks |
| `TypeToLLVM.h` | MLIR type → LLVM IR type conversion |
| `TypeFromLLVM.h` | LLVM IR type → MLIR type conversion |
| `DataLayoutImporter.h` | Import LLVM data layout string into MLIR DLTI attribute |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Dialect/` | Per-dialect LLVM translation interface implementations (see `Dialect/AGENTS.md`) |
| `Transforms/` | Pre-translation transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- To add LLVM translation support for a new dialect, implement `LLVMTranslationDialectInterface`.
- `ModuleTranslation` is the main workhorse for export; it maintains value/block/function mapping.
- Import uses `ModuleImport` which walks LLVM IR instructions and creates MLIR ops.
- Type conversion is handled separately from op translation via `TypeToLLVM`/`TypeFromLLVM`.

### Common Patterns
- Export: `llvm::LLVMContext ctx; auto llvmModule = translateModuleToLLVMIR(mlirModule, ctx);`
- Import: `auto mlirModule = translateLLVMIRToMLIR(*llvmModule, &mlirContext);`
- Dialect hook: implement `LLVMTranslationDialectInterface::convertOperation()`.

## Dependencies

### Internal
- `mlir/Dialect/LLVMIR/` (LLVM dialect ops and types)
- `mlir/IR/` (MLIRContext, ModuleOp)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/IR/` (Module, Function, IRBuilder, Type)

<!-- MANUAL: -->
