<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToXeGPU

## Purpose
Implements the Vector to XeGPU lowering pass. Converts `vector` dialect matrix and transfer ops into Intel XeGPU dialect ops for Intel GPU subgroup-level matrix operations (DPAS, 2D block load/store).

## Key Files
| File | Description |
|------|-------------|
| `VectorToXeGPU.cpp` | Conversion patterns from vector ops to XeGPU subgroup matrix ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToXeGPUConversionPatterns()`
- XeGPU `TensorDesc` types describe 2D block memory regions; vector shapes must match DPAS tile sizes

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `vector.contract` → `xegpu.dpas` for matrix multiply-accumulate
- `vector.transfer_read/write` → `xegpu.load_nd`/`xegpu.store_nd` with 2D descriptors

## Dependencies
- Headers: `include/mlir/Conversion/VectorToXeGPU/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/XeGPU/`

<!-- MANUAL: -->
