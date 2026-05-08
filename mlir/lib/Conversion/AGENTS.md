<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Conversion

## Purpose
Implements all bundled dialect-to-dialect conversion passes. Each subdirectory provides a progressive lowering step that converts ops from a higher-level dialect to a lower-level (or target-specific) one. Conversions are implemented as pass pipelines using the `DialectConversion` framework from `mlir/lib/Transforms/Utils/`.

## Subdirectories

| Directory | Converts From → To | Notes |
|-----------|-------------------|-------|
| `AffineToStandard/` | `affine` → `scf`, `memref`, `arith` | Lowers affine loops and memory ops to structured control flow |
| `AMDGPUToROCDL/` | `amdgpu` → `rocdl` | AMD GPU intrinsics to ROCDL dialect |
| `ArithAndMathToAPFloat/` | `arith`+`math` → APFloat | Software-float lowering using `llvm::APFloat` |
| `ArithCommon/` | — | Shared utilities for Arith lowering passes |
| `ArithToAMDGPU/` | `arith` → `amdgpu` | Arith ops to AMD GPU-specific equivalents |
| `ArithToArmSME/` | `arith` → `arm_sme` | Arith ops to ARM SME |
| `ArithToEmitC/` | `arith` → `emitc` | Arith ops to C++ emission |
| `ArithToLLVM/` | `arith` → `llvm` | Arith ops to LLVM dialect |
| `ArithToSPIRV/` | `arith` → `spirv` | Arith ops to SPIR-V |
| `ArmNeon2dToIntr/` | `arm_neon` 2D → intrinsics | Lowers 2D NEON ops to LLVM intrinsics |
| `ArmSMEToLLVM/` | `arm_sme` → `llvm` | ARM SME to LLVM dialect |
| `ArmSMEToSCF/` | `arm_sme` → `scf` | ARM SME tile ops to loop-based equivalents |
| `AsyncToLLVM/` | `async` → `llvm` | Async runtime calls to LLVM dialect |
| `BufferizationToMemRef/` | `bufferization` → `memref` | Bufferization ops to MemRef |
| `ComplexCommon/` | — | Shared utilities for Complex lowering |
| `ComplexToLibm/` | `complex` → libm calls | Complex math to C math library calls |
| `ComplexToLLVM/` | `complex` → `llvm` | Complex ops to LLVM dialect |
| `ComplexToROCDLLibraryCalls/` | `complex` → ROCDL | Complex to ROCm device library calls |
| `ComplexToSPIRV/` | `complex` → `spirv` | Complex to SPIR-V |
| `ComplexToStandard/` | `complex` → `arith`+`math` | Complex ops to scalar arithmetic |
| `ControlFlowToLLVM/` | `cf` → `llvm` | Branches and asserts to LLVM dialect |
| `ControlFlowToSCF/` | `cf` → `scf` | Unstructured CFG to structured control flow (reverse lowering) |
| `ControlFlowToSPIRV/` | `cf` → `spirv` | Control flow to SPIR-V |
| `ConvertToEmitC/` | generic → `emitc` | Generic conversion to EmitC |
| `ConvertToLLVM/` | generic → `llvm` | Shared infrastructure for lowering to LLVM dialect |
| `FuncToEmitC/` | `func` → `emitc` | Function ops to EmitC |
| `FuncToLLVM/` | `func` → `llvm` | Function/call/return to LLVM dialect |
| `FuncToSPIRV/` | `func` → `spirv` | Function ops to SPIR-V |
| `GPUCommon/` | — | Shared utilities for GPU lowerings |
| `GPUToLLVMSPV/` | `gpu` → LLVM+SPIRV | GPU ops to LLVM SPIR-V path |
| `GPUToNVVM/` | `gpu` → `nvvm` | GPU kernel launch to NVVM (CUDA) |
| `GPUToROCDL/` | `gpu` → `rocdl` | GPU kernel launch to ROCDL (ROCm) |
| `GPUToSPIRV/` | `gpu` → `spirv` | GPU ops to SPIR-V |
| `IndexToLLVM/` | `index` → `llvm` | Index type arithmetic to LLVM i64/i32 |
| `IndexToSPIRV/` | `index` → `spirv` | Index to SPIR-V |
| `LinalgToStandard/` | `linalg` → `func`+`memref` | Named linalg ops to library calls |
| `LLVMCommon/` | — | Shared type converter and pattern helpers for LLVM dialect lowerings |
| `MathToEmitC/` | `math` → `emitc` | Math ops to C++ emission |
| `MathToFuncs/` | `math` → function calls | Math ops to `func.call` (software implementations) |
| `MathToLibm/` | `math` → libm | Math ops to C math library calls |
| `MathToLLVM/` | `math` → `llvm` | Math ops to LLVM intrinsics |
| `MathToNVVM/` | `math` → `nvvm` | Math ops to NVVM (CUDA) intrinsics |
| `MathToROCDL/` | `math` → `rocdl` | Math ops to ROCm device library calls |
| `MathToSPIRV/` | `math` → `spirv` | Math ops to SPIR-V |
| `MathToXeVM/` | `math` → `xevm` | Math ops to Intel XeVM |
| `MemRefToEmitC/` | `memref` → `emitc` | MemRef ops to C++ emission |
| `MemRefToLLVM/` | `memref` → `llvm` | MemRef alloc/load/store to LLVM dialect |
| `MemRefToSPIRV/` | `memref` → `spirv` | MemRef to SPIR-V |
| `MPIToLLVM/` | `mpi` → `llvm` | MPI ops to LLVM dialect calls |
| `NVGPUToNVVM/` | `nvgpu` → `nvvm` | NVIDIA GPU tensor-core ops to NVVM |
| `NVVMToLLVM/` | `nvvm` → `llvm` | NVVM intrinsics to LLVM dialect |
| `OpenACCToSCF/` | `acc` → `scf` | OpenACC constructs to structured control flow |
| `OpenMPToLLVM/` | `omp` → `llvm` | OpenMP ops to LLVM dialect |
| `PDLToPDLInterp/` | `pdl` → `pdl_interp` | PDL patterns to PDL interpreter bytecode |
| `PtrToLLVM/` | `ptr` → `llvm` | Pointer ops to LLVM dialect |
| `ReconcileUnrealizedCasts/` | — | Removes leftover `unrealized_conversion_cast` pairs |
| `SCFToControlFlow/` | `scf` → `cf` | Structured loops/branches to unstructured CFG |
| `SCFToEmitC/` | `scf` → `emitc` | SCF to C++ emission |
| `SCFToGPU/` | `scf` → `gpu` | SCF parallel loops to GPU kernel launch |
| `SCFToOpenMP/` | `scf` → `omp` | SCF parallel ops to OpenMP |
| `SCFToSPIRV/` | `scf` → `spirv` | SCF to SPIR-V |
| `ShapeToStandard/` | `shape` → `arith`+`scf` | Shape ops to arithmetic |
| `ShardToMPI/` | `shard` → `mpi` | Shard communication ops to MPI |
| `SPIRVCommon/` | — | Shared utilities for SPIR-V lowering passes |
| `SPIRVToLLVM/` | `spirv` → `llvm` | SPIR-V ops to LLVM dialect |
| `TensorToLinalg/` | `tensor` → `linalg` | Tensor ops to Linalg structured form |
| `TensorToSPIRV/` | `tensor` → `spirv` | Tensor ops to SPIR-V |
| `TosaToArith/` | `tosa` → `arith` | TOSA scalar ops to Arith |
| `TosaToLinalg/` | `tosa` → `linalg` | TOSA tensor ops to Linalg |
| `TosaToMLProgram/` | `tosa` → `ml_program` | TOSA to MLProgram |
| `TosaToSCF/` | `tosa` → `scf` | TOSA control flow to SCF |
| `TosaToTensor/` | `tosa` → `tensor` | TOSA tensor ops to Tensor dialect |
| `UBToLLVM/` | `ub` → `llvm` | Poison ops to LLVM `undef` |
| `UBToSPIRV/` | `ub` → `spirv` | Poison ops to SPIR-V undef |
| `VectorToAMX/` | `vector` → AMX | Vector ops to Intel AMX tile intrinsics |
| `VectorToArmSME/` | `vector` → `arm_sme` | Vector ops to ARM SME |
| `VectorToGPU/` | `vector` → `gpu`+`nvgpu` | Vector transfer ops to GPU tensor-core ops |
| `VectorToLLVM/` | `vector` → `llvm` | Vector ops to LLVM dialect |
| `VectorToSCF/` | `vector` → `scf`+`memref` | Vector transfer ops to scalar loops |
| `VectorToSPIRV/` | `vector` → `spirv` | Vector ops to SPIR-V |
| `VectorToXeGPU/` | `vector` → `xegpu` | Vector ops to Intel XeGPU |
| `XeGPUToXeVM/` | `xegpu` → `xevm` | XeGPU ops to Intel XeVM |
| `XeVMToLLVM/` | `xevm` → `llvm` | Intel XeVM to LLVM dialect |

## For AI Agents

### Working In This Directory
- Each conversion subdirectory implements one or more `ConversionPattern`s registered in a pass via `populateXxxToYyyConversionPatterns()`.
- All conversions use `DialectConversionTarget` and `TypeConverter` from `mlir/lib/Transforms/Utils/DialectConversion.cpp`.
- `LLVMCommon/` and `*Common/` subdirectories provide shared `TypeConverter` instances; reuse them instead of creating new ones.
- `ReconcileUnrealizedCasts/` is always run last in a lowering pipeline to clean up leftover `unrealized_conversion_cast` pairs introduced by partial lowering.
- Generated `.inc` files for pass declarations come from `.td` files in `mlir/include/mlir/Conversion/`; run the corresponding `*IncGen` CMake target before editing.

### Common Patterns
- Conversion patterns inherit from `ConversionPattern` or `OpConversionPattern<OpType>`.
- Passes call `applyPartialConversion(op, target, patterns)` or `applyFullConversion(op, target, patterns)`.
- Type converters map source types to target types; operand/result rematerialization is handled automatically by the conversion framework.

## Dependencies

### Internal
- `mlir/lib/Transforms/Utils/DialectConversion.cpp` — conversion framework
- `mlir/lib/Dialect/` — source and target dialects
- `mlir/lib/IR/` — core IR

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
