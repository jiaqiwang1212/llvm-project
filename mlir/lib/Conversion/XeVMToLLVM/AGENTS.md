<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XeVMToLLVM

## Purpose
Implements the XeVM to LLVM lowering pass. Converts XeVM dialect intrinsic ops into LLVM dialect `llvm.call_intrinsic` or `llvm.inline_asm` ops, completing the lowering to standard LLVM IR for Intel GPU compilation.

## Key Files
| File | Description |
|------|-------------|
| `XeVMToLLVM.cpp` | Conversion patterns from XeVM ops to LLVM dialect intrinsic calls |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateXeVMToLLVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering to standard LLVM IR types

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- XeVM intrinsics map to Intel GPU-specific LLVM intrinsic names (e.g., `llvm.genx.*`)

## Dependencies
- Headers: `include/mlir/Conversion/XeVMToLLVM/`
- Source dialect: `lib/Dialect/LLVMIR/XeVM/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
