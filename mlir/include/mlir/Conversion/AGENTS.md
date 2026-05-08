<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Conversion/

## Purpose
Dialect-to-dialect conversion pass headers. Each subdirectory provides `populate*ConversionPatterns()` functions and `create*ConversionPass()` factories for one conversion path (e.g., ArithToLLVM, SCFToControlFlow, LinalgToStandard). These are the primary mechanism for progressive lowering pipelines in MLIR.

## Key Files
(none at top level — all content is in subdirectories)

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AffineToStandard/` | Lower Affine dialect to Standard/SCF/MemRef ops |
| `AMDGPUToROCDL/` | Lower AMDGPU dialect to ROCDL dialect |
| `ArithAndMathToAPFloat/` | Lower Arith/Math ops to APFloat-based computation |
| `ArithCommon/` | Shared utilities for Arith conversions |
| `ArithToAMDGPU/` | Lower Arith ops to AMDGPU dialect |
| `ArithToArmSME/` | Lower Arith ops to ArmSME dialect |
| `ArithToEmitC/` | Lower Arith ops to EmitC dialect |
| `ArithToLLVM/` | Lower Arith ops to LLVM dialect |
| `ArithToSPIRV/` | Lower Arith ops to SPIR-V dialect |
| `ArmNeon2dToIntr/` | Lower ArmNeon 2D ops to LLVM intrinsics |
| `ArmSMEToLLVM/` | Lower ArmSME ops to LLVM dialect |
| `ArmSMEToSCF/` | Lower ArmSME ops to SCF loops |
| `AsyncToLLVM/` | Lower Async dialect to LLVM dialect (coroutines) |
| `BufferizationToMemRef/` | Lower Bufferization ops to MemRef ops |
| `ComplexCommon/` | Shared utilities for Complex conversions |
| `ComplexToLibm/` | Lower Complex ops to libm calls |
| `ComplexToLLVM/` | Lower Complex ops to LLVM dialect |
| `ComplexToROCDLLibraryCalls/` | Lower Complex ops to ROCDL library calls |
| `ComplexToSPIRV/` | Lower Complex ops to SPIR-V |
| `ComplexToStandard/` | Lower Complex ops to standard arithmetic |
| `ControlFlowToLLVM/` | Lower ControlFlow ops to LLVM dialect |
| `ControlFlowToSCF/` | Lower ControlFlow ops to SCF dialect |
| `ControlFlowToSPIRV/` | Lower ControlFlow ops to SPIR-V |
| `ConvertToEmitC/` | Generic conversion to EmitC dialect |
| `ConvertToLLVM/` | Generic MLIR-to-LLVM conversion infrastructure |
| `FuncToEmitC/` | Lower Func ops to EmitC |
| `FuncToLLVM/` | Lower Func dialect to LLVM dialect |
| `FuncToSPIRV/` | Lower Func dialect to SPIR-V |
| `GPUCommon/` | Shared GPU conversion utilities |
| `GPUToLLVMSPV/` | Lower GPU dialect to LLVM/SPIR-V |
| `GPUToNVVM/` | Lower GPU dialect to NVVM dialect |
| `GPUToROCDL/` | Lower GPU dialect to ROCDL dialect |
| `GPUToSPIRV/` | Lower GPU dialect to SPIR-V |
| `IndexToLLVM/` | Lower Index dialect to LLVM dialect |
| `IndexToSPIRV/` | Lower Index dialect to SPIR-V |
| `LinalgToStandard/` | Lower Linalg named ops to loops |
| `LLVMCommon/` | Shared LLVM conversion type converter and utilities |
| `MathToEmitC/` | Lower Math ops to EmitC |
| `MathToFuncs/` | Lower Math ops to function calls |

## For AI Agents

### Working In This Directory
- Each conversion subdirectory typically contains a `Passes.h` (factories), a `Passes.td` (options), and pattern populate functions.
- The standard convention is `populateFooToBarConversionPatterns(RewritePatternSet&, TypeConverter&)`.
- `LLVMCommon/` provides `LLVMTypeConverter` and `ConvertToLLVMPatternInterface` used by most LLVM-targeting conversions.
- `ConvertToLLVM/` is the generic infrastructure for the `--convert-to-llvm` pipeline pass.
- When adding a new conversion, also add it to `mlir/include/mlir/InitAllPasses.h`.

### Common Patterns
- Include: `#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"`
- Usage: `mlir::arith::populateArithToLLVMConversionPatterns(typeConverter, patterns);`
- Pass creation: `mlir::createConvertArithToLLVMPass()`

## Dependencies

### Internal
- `mlir/IR/` (Operation, MLIRContext)
- `mlir/Transforms/` (DialectConversion, TypeConverter)
- `mlir/Dialect/` (source and target dialects)

### External
- `llvm/ADT/` (SmallVector)

<!-- MANUAL: -->
