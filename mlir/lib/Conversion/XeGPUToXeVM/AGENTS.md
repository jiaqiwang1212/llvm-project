<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XeGPUToXeVM

## Purpose
Implements the XeGPU to XeVM lowering pass. Converts Intel XeGPU dialect ops (2D block loads/stores, DPAS matrix ops) into XeVM dialect intrinsic calls targeting the Intel GPU LLVM backend.

## Key Files
| File | Description |
|------|-------------|
| `XeGPUToXeVM.cpp` | Conversion patterns from XeGPU ops to XeVM intrinsics |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateXeGPUToXeVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering; XeGPU TensorDesc becomes pointer + descriptor struct

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `xegpu.dpas` → `xevm.dpas` intrinsic with A/B/C matrix operands
- `xegpu.load_nd` / `xegpu.store_nd` → XeVM 2D block load/store intrinsics

## Dependencies
- Headers: `include/mlir/Conversion/XeGPUToXeVM/`
- Source dialect: `lib/Dialect/XeGPU/`
- Target dialect: `lib/Dialect/LLVMIR/XeVM/`

<!-- MANUAL: -->
