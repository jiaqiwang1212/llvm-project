<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PtrToLLVM

## Purpose
Implements the Ptr dialect to LLVM lowering pass. Converts `ptr` dialect operations (pointer arithmetic, loads, stores) into LLVM dialect ops using opaque `!llvm.ptr` types.

## Key Files
| File | Description |
|------|-------------|
| `PtrToLLVM.cpp` | Conversion patterns from Ptr ops to LLVM dialect |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populatePtrToLLVMConversionPatterns()`
- `LLVMTypeConverter` maps `ptr<T>` to `!llvm.ptr`

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- GEP and load/store patterns map directly to `llvm.getelementptr` and `llvm.load`/`llvm.store`

## Dependencies
- Headers: `include/mlir/Conversion/PtrToLLVM/`
- Source dialect: `lib/Dialect/Ptr/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
