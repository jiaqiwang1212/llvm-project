<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToLLVMSPV Conversion

## Purpose
Lowers GPU dialect ops to LLVM SPIR-V dialect for Intel GPU targets (using the LLVM SPIR-V backend rather than the MLIR SPIR-V dialect). Converts GPU thread/block index ops to LLVM SPIR-V built-in variable accesses.

## Key Files
| File | Description |
|------|-------------|
| `GPUToLLVMSPV.cpp` | Patterns lowering GPU index ops to LLVM SPIR-V built-in accesses |

## For AI Agents

### Working In This Directory
- Targets the LLVM SPIR-V backend path (distinct from `GPUToSPIRV` which targets the MLIR SPIR-V dialect).
- `gpu.thread_id` → `__spirv_BuiltInLocalInvocationId` global variable access via LLVM dialect.
- Used in Intel's oneAPI/Level Zero compilation pipeline.

## Dependencies
- Source: `mlir/Dialect/GPU`
- Target: `mlir/Dialect/LLVMIR` (with SPIR-V built-in attributes)

<!-- MANUAL: -->
