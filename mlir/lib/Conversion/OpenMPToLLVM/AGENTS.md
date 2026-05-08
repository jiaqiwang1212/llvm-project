<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenMPToLLVM

## Purpose
Implements the OpenMP to LLVM lowering pass. Converts `omp` dialect operations (parallel regions, worksharing loops, synchronization) into LLVM dialect calls targeting the OpenMP runtime library ABI.

## Key Files
| File | Description |
|------|-------------|
| `OpenMPToLLVM.cpp` | Conversion patterns from OpenMP ops to LLVM runtime calls |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateOpenMPToLLVMConversionPatterns()`
- `LLVMTypeConverter` handles type lowering; OpenMP region types become LLVM structs/function pointers

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- OpenMP runtime calls (`__kmpc_*`) are declared as external `llvm.func` symbols

## Dependencies
- Headers: `include/mlir/Conversion/OpenMPToLLVM/`
- Source dialect: `lib/Dialect/OpenMP/`
- Target dialect: `lib/Dialect/LLVMIR/`

<!-- MANUAL: -->
