<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPUToNVVM

## Purpose
Implements the NVGPU to NVVM lowering pass. Converts high-level NVIDIA GPU dialect ops (warp-level matrix ops, async copies, tensor memory access) into NVVM intrinsic calls targeting PTX-level constructs.

## Key Files
| File | Description |
|------|-------------|
| `NVGPUToNVVM.cpp` | Conversion patterns from NVGPU ops to NVVM intrinsics |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateNVGPUToNVVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering; NVGPU tensor types lower to LLVM structs/vectors

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- NVVM intrinsics are emitted via `LLVM::IntrinsicOp` or `nvvm.*` ops

## Dependencies
- Headers: `include/mlir/Conversion/NVGPUToNVVM/`
- Source dialect: `lib/Dialect/NVGPU/`
- Target dialect: `lib/Dialect/LLVMIR/NVVM/`

<!-- MANUAL: -->
