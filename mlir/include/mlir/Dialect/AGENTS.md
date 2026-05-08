<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Dialect/

## Purpose
Headers for all bundled MLIR dialects. Each subdirectory contains the complete public header tree for one dialect: op declarations, type/attribute definitions, interfaces, passes, and utilities. This is the largest part of the MLIR include tree, covering dialects from low-level (LLVM, NVVM, ROCDL) to high-level (Linalg, Affine, SCF) and domain-specific (SparseTensor, GPU, OpenMP).

## Key Files
| File | Description |
|------|-------------|
| `CommonFolders.h` | Shared folding utilities used by multiple dialects |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Affine/` | Affine dialect: loop/if ops with affine map constraints |
| `AMDGPU/` | AMDGPU dialect: AMD GPU-specific intrinsics and ops |
| `Arith/` | Arithmetic dialect: integer and float arithmetic ops |
| `ArmNeon/` | ARM NEON dialect: ARM SIMD intrinsic wrappers |
| `ArmSME/` | ARM SME dialect: Scalable Matrix Extension ops |
| `ArmSVE/` | ARM SVE dialect: Scalable Vector Extension ops |
| `Async/` | Async dialect: async/await-style concurrency ops |
| `Bufferization/` | Bufferization dialect: tensor-to-memref allocation ops |
| `Complex/` | Complex number dialect |
| `ControlFlow/` | Control flow dialect: branch, switch, assert ops |
| `DLTI/` | Data Layout and Target Info dialect |
| `EmitC/` | EmitC dialect: ops that map directly to C/C++ constructs |
| `Func/` | Func dialect: `func.func`, `func.call`, `func.return` |
| `GPU/` | GPU dialect: kernel launch, thread/block indexing, barriers |
| `Index/` | Index dialect: index-typed arithmetic ops |
| `IRDL/` | IRDL dialect: IR Definition Language for dynamic dialects |
| `Linalg/` | Linalg dialect: structured loop-free linear algebra ops |
| `LLVMIR/` | LLVM IR dialect: direct mapping of LLVM IR instructions |
| `Math/` | Math dialect: transcendental and math functions |
| `MemRef/` | MemRef dialect: memory reference allocation/manipulation |
| `MLProgram/` | ML Program dialect: variables and subprogram ops |
| `MPI/` | MPI dialect: Message Passing Interface ops |
| `NVGPU/` | NVGPU dialect: NVIDIA GPU-specific ops (Tensor Core, etc.) |
| `OpenACC/` | OpenACC dialect: directive-based GPU offload |
| `OpenACCMPCommon/` | Shared utilities for OpenACC and OpenMP dialects |
| `OpenMP/` | OpenMP dialect: directive-based parallelism ops |
| `PDL/` | PDL dialect: Pattern Description Language IR |
| `PDLInterp/` | PDL Interpreter dialect: compiled PDL bytecode |
| `Ptr/` | Pointer dialect: typed pointer ops |
| `Quant/` | Quantization dialect: quantized type modeling |
| `SCF/` | Structured Control Flow dialect: for, if, while, parallel ops |
| `Shape/` | Shape dialect: dynamic shape computation ops |
| `Shard/` | Sharding dialect: tensor distribution annotations |
| `SMT/` | SMT dialect: Satisfiability Modulo Theories ops |
| `SparseTensor/` | Sparse tensor dialect: sparse storage and computation |
| `SPIRV/` | SPIR-V dialect: Khronos SPIR-V instruction set |
| `Tensor/` | Tensor dialect: tensor manipulation ops |
| `Tosa/` | TOSA dialect: Tensor Operator Set Architecture |

## For AI Agents

### Working In This Directory
- Each dialect subdirectory is self-contained; see its own `AGENTS.md` for details.
- Dialect headers are **not** documented here — this is a top-level index only.
- When adding a new dialect, create a new subdirectory following the pattern of existing dialects and register it in `mlir/include/mlir/InitAllDialects.h`.
- Dialect ops, types, and attributes are defined via TableGen (`.td`) files and included via `.h.inc` generated files.

### Common Patterns
- Dialect includes: `#include "mlir/Dialect/Foo/IR/FooOps.h"`
- Dialect registration: `mlir::registerFooDialect(registry)` or `context.loadDialect<FooDialect>()`.

## Dependencies

### Internal
- `mlir/IR/` (Dialect, Op, Type, Attribute base classes)
- `mlir/Interfaces/` (interfaces implemented by dialect ops)

### External
- Dialect-specific: LLVM IR types for LLVMIR dialect, SPIR-V headers for SPIRV dialect, etc.

<!-- MANUAL: -->
