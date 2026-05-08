<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToLLVM

## Purpose
Implements the Vector to LLVM lowering pass. Converts `vector` dialect ops (load, store, broadcast, shuffle, reduction, insert/extract, etc.) into LLVM dialect ops and LLVM vector intrinsics.

## Key Files
| File | Description |
|------|-------------|
| `ConvertVectorToLLVM.cpp` | Core conversion patterns from vector ops to LLVM dialect |
| `ConvertVectorToLLVMPass.cpp` | Pass registration, options, and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateVectorToLLVMConversionPatterns()`
- `LLVMTypeConverter` maps `vector<NxT>` to `!llvm.vec<N x T>`

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- Multi-dimensional vectors are flattened to 1D LLVM vectors via linearization
- Scalable vectors (`[?xN x T]`) require `llvm.vscale` for dynamic size queries

## Dependencies
- Headers: `include/mlir/Conversion/VectorToLLVM/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
