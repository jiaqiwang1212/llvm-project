<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/LLVMIR/Dialect/

## Purpose
Per-dialect LLVM translation interface implementations. Each subdirectory contains the `LLVMTranslationDialectInterface` and/or `LLVMImportDialectInterface` headers for one dialect, enabling that dialect's ops to participate in MLIR ↔ LLVM IR translation.

## Key Files
| File | Description |
|------|-------------|
| `All.h` | Convenience header that includes all dialect LLVM translation interfaces |
| `OpenMPCommon.h` | Shared OpenMP/OpenACC utilities for LLVM IR translation |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `ArmNeon/` | ArmNeon dialect LLVM translation interface |
| `ArmSME/` | ArmSME dialect LLVM translation interface |
| `ArmSVE/` | ArmSVE dialect LLVM translation interface |
| `Builtin/` | Builtin dialect LLVM translation (module, function attributes) |
| `GPU/` | GPU dialect LLVM translation interface |
| `LLVMIR/` | LLVM IR dialect translation (core ops → llvm::Instructions) |
| `NVVM/` | NVVM dialect LLVM translation (NVIDIA intrinsics) |
| `OpenACC/` | OpenACC dialect LLVM translation interface |
| `OpenMP/` | OpenMP dialect LLVM translation interface |
| `Ptr/` | Pointer dialect LLVM translation interface |
| `ROCDL/` | ROCDL dialect LLVM translation (AMD GPU intrinsics) |
| `SPIRV/` | SPIR-V dialect LLVM translation interface |
| `VCIX/` | VCIX dialect LLVM translation interface |
| `XeVM/` | XeVM dialect LLVM translation interface |

## For AI Agents

### Working In This Directory
- Each subdirectory registers a `LLVMTranslationDialectInterface` with the dialect to hook into `ModuleTranslation`.
- Include `All.h` to register all dialect translation interfaces at once (used by `mlir-translate`).
- When adding translation support for a new dialect, create a new subdirectory here following the existing pattern.

### Common Patterns
- Interface: `void registerMyDialectTranslationInterface(DialectRegistry &registry);`
- Called in: `mlir::registerAllToLLVMIRTranslations(registry)`.

## Dependencies

### Internal
- `mlir/Target/LLVMIR/LLVMTranslationDialectInterface.h`
- Respective dialect headers in `mlir/Dialect/`

### External
- `llvm/IR/` (IRBuilder, Function, Module)

<!-- MANUAL: -->
