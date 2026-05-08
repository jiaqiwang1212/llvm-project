<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPUToLLVMSPV

## Purpose
Declares the GPU to LLVM-SPIRV lowering pass. Lowers GPU dialect ops to LLVM dialect with SPIRV extensions for targets that consume LLVM-SPIRV IR (e.g., Intel GPU).

## Key Files
| File | Description |
|------|-------------|
| `GPUToLLVMSPVPass.h` | Pass pipeline entry-points and option structs |

## For AI Agents

### Working In This Directory
- Edit `GPUToLLVMSPVPass.h` to modify pass declarations
- The implementation lives under `mlir/lib/Conversion/GPUToLLVMSPV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/GPU/`
- Target dialect: `include/mlir/Dialect/LLVMIR/` with SPIRV extensions
- Shared utilities: `GPUCommon/`

<!-- MANUAL: -->
