<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/LLVMIR

## Purpose
Implements bidirectional translation between the MLIR LLVM dialect and LLVM IR (`llvm::Module`). This is the primary exit path from MLIR to native code generation. Also implements the import direction (LLVM IR → MLIR LLVM dialect) used for embedding existing LLVM IR into an MLIR pipeline.

## Key Files
| File | Description |
|------|-------------|
| `ModuleTranslation.cpp` | Core MLIR → LLVM IR translator: walks LLVM dialect ops and emits `llvm::Value` / `llvm::BasicBlock` / `llvm::Function` objects; handles op-by-op dispatch via `LLVMTranslationDialectInterface` |
| `ModuleImport.cpp` | Core LLVM IR → MLIR importer: walks `llvm::Module` and produces MLIR LLVM dialect ops; reconstructs SSA values, basic blocks, and metadata |
| `ConvertToLLVMIR.cpp` | Entry point: `mlir::translateModuleToLLVMIR()` — sets up the translation context and invokes `ModuleTranslation` |
| `ConvertFromLLVMIR.cpp` | Entry point: `mlir::translateLLVMIRToModule()` — sets up the import context and invokes `ModuleImport` |
| `TypeToLLVM.cpp` | Translates MLIR LLVM dialect types to `llvm::Type*` |
| `TypeFromLLVM.cpp` | Translates `llvm::Type*` to MLIR LLVM dialect types |
| `DataLayoutImporter.cpp` | Imports LLVM data layout strings into MLIR `DataLayoutSpecAttr` |
| `DebugTranslation.cpp` / `DebugTranslation.h` | Translates MLIR location attributes to LLVM `llvm::DILocation` / `DISubprogram` debug metadata (export direction) |
| `DebugImporter.cpp` / `DebugImporter.h` | Imports LLVM debug metadata back to MLIR location attributes (import direction) |
| `LoopAnnotationTranslation.cpp` / `.h` | Translates MLIR loop annotation attributes to LLVM `llvm.loop` metadata and back |
| `LoopAnnotationImporter.cpp` / `.h` | Imports LLVM loop metadata to MLIR loop annotation attributes |
| `LLVMImportInterface.cpp` | `LLVMImportInterface` registration: allows dialects to register custom op importers for LLVM intrinsics |
| `AttrKindDetail.h` | Internal mapping tables for LLVM attribute kind IDs |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Dialect/` | Per-dialect LLVM translation interface implementations (NVVM, ROCDL, etc. register custom op translators here) |
| `Transforms/` | IR transformations applied during translation (e.g., legalization before export) |

## For AI Agents

### Working In This Directory
- When adding translation support for a new LLVM intrinsic, implement `LLVMTranslationDialectInterface::convertOperation()` in the relevant dialect and register it.
- `ModuleTranslation.cpp` is performance-critical for large modules; avoid O(n²) patterns.
- Debug metadata translation is optional and controlled by whether the MLIR module has `FileLineColLoc` locations.
- `DataLayoutImporter.cpp` must stay in sync with changes to MLIR's `DataLayoutSpecAttr`.

### Common Patterns
- The translator uses a `ModuleTranslation` object that caches `llvm::Value*` for each `mlir::Value` in a `DenseMap`.
- Dialect-specific ops register custom translation via `LLVMTranslationDialectInterface` on the dialect.

## Dependencies

### Internal
- `mlir/lib/Dialect/LLVMIR/` — LLVM dialect op definitions
- `mlir/lib/IR/` — core IR
- `mlir/lib/Target/LLVM/` — `ModuleToObject` shared infrastructure

### External
- `llvm/lib/IR` — `llvm::Module`, `llvm::Value`, `llvm::Type`, debug metadata
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
