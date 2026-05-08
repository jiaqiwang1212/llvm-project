<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LLVMIR Dialect

## Purpose
Implements the LLVM IR dialect — the 1:1 MLIR representation of LLVM IR. Provides ops, types, and attributes that map directly to LLVM IR constructs. Also hosts NVVM, ROCDL, VCIX, and XeVM sub-dialects for GPU-specific LLVM intrinsics.

## Key Files
| File | Description |
|------|-------------|
| `IR/LLVMDialect.cpp` | Core LLVM dialect registration and type system |
| `IR/LLVMOps.cpp` | All LLVM IR ops: `llvm.add`, `llvm.load`, `llvm.store`, `llvm.getelementptr`, `llvm.call`, etc. |
| `IR/LLVMTypes.cpp` | LLVM type implementations: struct, array, pointer, vector, function types |
| `IR/LLVMAttrs.cpp` | LLVM attribute implementations (linkage, calling conventions, DIScope, etc.) |
| `IR/NVVMDialect.cpp` | NVVM dialect (NVIDIA GPU intrinsics) |
| `IR/ROCDLDialect.cpp` | ROCDL dialect (AMD GPU intrinsics) |
| `IR/XeVMDialect.cpp` | XeVM dialect (Intel GPU intrinsics) |
| `IR/VCIXDialect.cpp` | VCIX dialect (RISC-V vector crypto intrinsics) |
| `IR/LLVMMemorySlot.cpp` | Memory slot analysis for LLVM IR ops (used by mem2reg) |
| `IR/FunctionCallUtils.cpp` | Utilities for building LLVM function declarations and calls |
| `Transforms/InlinerInterfaceImpl.cpp` | Inliner interface for LLVM functions |
| `Transforms/LegalizeForExport.cpp` | Legalization pass before LLVM IR export |
| `Transforms/OptimizeForNVVM.cpp` | NVVM-specific LLVM IR optimizations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Full LLVM IR op/type/attr set plus NVVM/ROCDL/XeVM/VCIX sub-dialects |
| `Transforms/` | Export legalization, inlining, NVVM optimization, debug info, comdats |

## For AI Agents

### Working In This Directory
- This dialect is the final MLIR representation before `mlir-translate --mlir-to-llvmir` converts to LLVM IR.
- Opaque pointers: LLVM dialect uses `!llvm.ptr` (opaque) by default; `!llvm.ptr<N>` for address space variants.
- GEP ops: `llvm.getelementptr` requires a `rawConstantIndices` attribute mixed with SSA index values — use `GEPIndicesAdaptor`.
- Debug info: `LLVMDialect` carries `DIScope`, `DIType`, etc. as attributes; `DIScopeForLLVMFuncOp.cpp` attaches scopes.
- `LLVMMemorySlot.cpp` enables `--sroa` / mem2reg passes on `llvm.alloca` + `llvm.load`/`llvm.store` patterns.
- NVVM/ROCDL ops are defined via `LLVM_IntrOp` in TableGen — adding an intrinsic requires a TableGen entry plus type mapping.

### Common Patterns
- Building LLVM calls: use `FunctionCallUtils::lookupOrCreateFn()` to get/create a function declaration, then `LLVM::CallOp`.
- `LegalizeForExport` inserts `llvm.mlir.undef` where needed and handles multi-block function legalization.
- `printType()` / `parseType()` in `LLVMTypeSyntax.cpp` handle the custom `!llvm.struct<(...)>` syntax.

## Dependencies
- `mlir/IR`, LLVM `Support`, `IR` (for `APInt`, `APFloat`, `DIScope` model)

<!-- MANUAL: -->
