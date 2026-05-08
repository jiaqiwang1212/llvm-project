<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRefToLLVM

## Purpose
Implements the MemRef to LLVM lowering pass. Converts `memref` types and operations (alloc, dealloc, load, store, cast, reshape, etc.) into LLVM dialect ops and `!llvm.ptr` / struct descriptors.

## Key Files
| File | Description |
|------|-------------|
| `MemRefToLLVM.cpp` | Core conversion patterns for memref ops to LLVM dialect |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `ConversionPattern` / `OpConversionPattern<OpTy>`
- Register patterns in `populateFinalizeMemRefToLLVMConversionPatterns()`
- `LLVMTypeConverter` handles `memref` → `!llvm.struct` descriptor lowering

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- MemRef descriptors use a two-pointer (allocated/aligned) + offset + strides struct layout

## Dependencies
- Headers: `include/mlir/Conversion/MemRefToLLVM/`
- Source dialect: `lib/Dialect/MemRef/`
- Target dialect: `lib/Dialect/LLVMIR/` + LLVM backend

<!-- MANUAL: -->
