<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPUToROCDL Conversion

## Purpose
Lowers the AMDGPU dialect to the ROCDL dialect (AMD GPU LLVM intrinsics). Converts MFMA/WMMA matrix ops, raw buffer operations, and AMD-specific atomic ops to their corresponding ROCDL intrinsic calls.

## Key Files
| File | Description |
|------|-------------|
| `AMDGPUToROCDL.cpp` | All conversion patterns from AMDGPU ops to ROCDL intrinsics |

## For AI Agents

### Working In This Directory
- MFMA ops map to `rocdl.mfma.*` intrinsics with specific type signatures per hardware chipset.
- Raw buffer ops (`amdgpu.raw_buffer_load/store`) map to `rocdl.buffer.load/store` with buffer descriptor arguments.
- Atomic operations map to ROCDL atomic intrinsics with appropriate memory ordering semantics.
- Chipset version from `AMDGPUAttrs` determines which intrinsic variant to emit.

## Dependencies
- Source: `mlir/Dialect/AMDGPU`
- Target: `mlir/Dialect/ROCDL`, `mlir/Dialect/LLVMIR`

<!-- MANUAL: -->
