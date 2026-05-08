<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LLVMIR Dialect

## Purpose
A near-isomorphic mapping of LLVM IR into MLIR. Provides ops for all LLVM instructions, LLVM types, and intrinsics. Also hosts NVVM (NVIDIA PTX), ROCDL (AMD), VCIX, and XeVM sub-dialects for GPU targets.

## Key Files
| File | Description |
|------|-------------|
| `LLVMDialect.h` | Dialect class declaration |
| `LLVMDialect.td` | Dialect definition |
| `LLVMOps.td` | LLVM instruction op definitions |
| `LLVMAttrDefs.td` | Attribute definitions (linkage, calling convention, etc.) |
| `LLVMAttrs.h` | Generated attribute declarations |
| `LLVMEnums.td` | Enum definitions |
| `LLVMInterfaces.h` | Interface declarations |
| `LLVMInterfaces.td` | ODS interface definitions |
| `LLVMIntrinsicOps.td` | LLVM intrinsic op definitions |
| `LLVMTypes.h` | LLVM type class declarations |
| `LLVMTypes.td` | LLVM type definitions (pointer, struct, array, etc.) |
| `LLVMOpBase.td` | Base classes for LLVM ops |
| `LLVMDialectBytecode.td` | Bytecode serialization definitions |
| `FunctionCallUtils.h` | Utilities for creating runtime/library function calls |
| `BasicPtxBuilderInterface.h` | PTX inline-asm builder interface declaration |
| `BasicPtxBuilderInterface.td` | ODS PTX builder interface definition |
| `NVVMDialect.h` | NVVM sub-dialect class declaration |
| `NVVMOps.td` | NVVM op definitions |
| `NVVMRequiresSMTraits.h` | SM version requirement traits for NVVM ops |
| `NVVMRequiresSMTraits.td` | ODS SM requirement trait definitions |
| `ROCDLDialect.h` | ROCDL sub-dialect class declaration |
| `ROCDLOps.td` | ROCDL op definitions |
| `VCIXDialect.h` | VCIX sub-dialect class declaration |
| `VCIXOps.td` | VCIX op definitions |
| `XeVMDialect.h` | XeVM sub-dialect class declaration |
| `XeVMOps.td` | XeVM op definitions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Transforms/` | LLVM IR-level transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `llvm.ptr` is opaque by default (LLVM 15+ style); use `GEP` ops for pointer arithmetic
- NVVM/ROCDL/VCIX/XeVM are sub-dialects registered alongside LLVMIR
- `FunctionCallUtils.h` is the standard way to emit runtime library calls (e.g., printf, malloc)

### Common Patterns
- Op names: `llvm.add`, `llvm.load`, `llvm.store`, `llvm.call`, `llvm.getelementptr`
- NVVM ops: `nvvm.read.ptx.sreg.tid.x`, `nvvm.mma.sync`, etc.

## Dependencies
- Receives lowerings from most other dialects via conversion passes

<!-- MANUAL: -->
