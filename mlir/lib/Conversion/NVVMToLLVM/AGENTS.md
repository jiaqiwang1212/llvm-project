<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVVMToLLVM

## Purpose
Implements the NVVM to LLVM lowering pass. Converts NVVM dialect ops and intrinsics (special register reads, memory barriers, warp shuffles) into LLVM dialect ops or inline PTX `llvm.inline_asm` calls.

## Key Files
| File | Description |
|------|-------------|
| `NVVMToLLVM.cpp` | Conversion patterns from NVVM ops to LLVM dialect |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateNVVMToLLVMConversionPatterns()`
- Handles lowering of NVVM-specific types and ops that have no direct LLVM equivalent

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- Inline PTX is emitted via `LLVM::InlineAsmOp` for ops with no intrinsic equivalent

## Dependencies
- Headers: `include/mlir/Conversion/NVVMToLLVM/`
- Source dialect: `lib/Dialect/LLVMIR/NVVM/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
