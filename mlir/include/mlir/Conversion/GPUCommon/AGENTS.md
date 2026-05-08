<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUCommon

## Purpose
Provides shared utilities for GPU dialect conversion passes. Contains common pass infrastructure, LLVM lowering helpers, and attribute converters shared across NVVM, ROCDL, and SPIRV GPU targets.

## Key Files
| File | Description |
|------|-------------|
| `AttrToSPIRVConverter.h` | GPU attribute to SPIR-V attribute conversion utilities |
| `GPUCommonPass.h` | Shared GPU-to-LLVM pass base infrastructure |
| `GPUToLLVM.h` | Common GPU-to-LLVM conversion pattern declarations |

## For AI Agents

### Working In This Directory
- Edit `GPUToLLVM.h` for shared GPU-to-LLVM patterns; `GPUCommonPass.h` for shared pass infrastructure
- This is a utility directory; do not register standalone passes here
- Consumers: `GPUToNVVM/`, `GPUToROCDL/`, `GPUToLLVMSPV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/GPU/`
- Target dialects: `include/mlir/Dialect/LLVMIR/`, `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
