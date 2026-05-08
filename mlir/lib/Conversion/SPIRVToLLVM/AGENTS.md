<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRVToLLVM

## Purpose
Implements the SPIR-V to LLVM lowering pass. Converts SPIR-V dialect ops (arithmetic, memory, control flow, builtins) into LLVM dialect ops for CPU-side execution or testing of SPIR-V programs.

## Key Files
| File | Description |
|------|-------------|
| `SPIRVToLLVM.cpp` | Core conversion patterns from SPIR-V ops to LLVM dialect |
| `SPIRVToLLVMPass.cpp` | Pass registration and pipeline entry point |
| `ConvertLaunchFuncToLLVMCalls.cpp` | Converts `spirv.launch_func` into direct LLVM function calls |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateSPIRVToLLVMConversionPatterns()`
- `LLVMTypeConverter` maps SPIR-V scalar/vector/pointer types to LLVM equivalents

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- SPIR-V built-in variables (WorkgroupId, etc.) become function arguments in the LLVM ABI

## Dependencies
- Headers: `include/mlir/Conversion/SPIRVToLLVM/`
- Source dialect: `lib/Dialect/SPIRV/`
- Target dialect: `lib/Dialect/LLVMIR/`
- Shared utilities: `lib/Conversion/SPIRVCommon/`

<!-- MANUAL: -->
