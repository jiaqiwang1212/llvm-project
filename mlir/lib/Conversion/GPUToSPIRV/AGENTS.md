<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToSPIRV Conversion

## Purpose
Lowers GPU dialect ops to SPIR-V dialect for OpenCL/Vulkan compute targets. Converts GPU kernel functions, thread/block index ops, barriers, and memory ops to SPIR-V built-in variables and execution mode ops.

## Key Files
| File | Description |
|------|-------------|
| `GPUToSPIRV.cpp` | Main lowering patterns for GPU ops to SPIR-V |
| `GPUToSPIRVPass.cpp` | Pass definition |
| `WmmaOpsToSPIRV.cpp` | Lowers GPU WMMA/cooperative matrix ops to SPIR-V cooperative matrix extension ops |

## For AI Agents

### Working In This Directory
- `gpu.thread_id x` → `spirv.BuiltIn.LocalInvocationId` variable load + extract component.
- `gpu.func` → `spirv.func` with `spirv.entry_point_abi` attribute specifying workgroup size.
- `gpu.barrier` → `spirv.ControlBarrier` with Workgroup scope.
- WMMA ops in `WmmaOpsToSPIRV.cpp` target the `SPV_KHR_cooperative_matrix` extension.
- Uses `AttrToSPIRVConverter` from GPUCommon for memory space mapping.

## Dependencies
- Source: `mlir/Dialect/GPU`
- Target: `mlir/Dialect/SPIRV`
- Utility: `mlir/Conversion/GPUCommon`

<!-- MANUAL: -->
