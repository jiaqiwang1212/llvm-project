<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UBToLLVM

## Purpose
Implements the UB (Undefined Behavior) to LLVM lowering pass. Converts `ub` dialect ops (poison values) into LLVM dialect equivalents (`llvm.mlir.poison`) for propagation through LLVM IR.

## Key Files
| File | Description |
|------|-------------|
| `UBToLLVM.cpp` | Conversion patterns from ub ops to LLVM dialect poison ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateUBToLLVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering for poison-typed values

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- `ub.poison` → `llvm.mlir.poison` with the appropriate lowered type
- Poison semantics are preserved through the LLVM layer for correctness

## Dependencies
- Headers: `include/mlir/Conversion/UBToLLVM/`
- Source dialect: `lib/Dialect/UB/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
