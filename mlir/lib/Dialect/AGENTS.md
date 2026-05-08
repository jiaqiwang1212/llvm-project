<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Dialect

## Purpose
Contains the implementations of all bundled MLIR dialects. Each subdirectory is a self-contained dialect with its ops, types, attributes, interfaces, and transformation passes. Dialects are the primary extension point of MLIR — each one defines a domain-specific set of operations and type system extensions.

## Subdirectories

Each subdirectory below is a distinct dialect. Per-dialect AGENTS.md files are maintained separately and are not part of this scope.

| Directory | Dialect | Brief Purpose |
|-----------|---------|---------------|
| `Affine/` | `affine` | Polyhedral loop and memory access abstractions with affine map/set constraints |
| `AMDGPU/` | `amdgpu` | AMD GPU-specific intrinsics and memory ops for CDNA/RDNA architectures |
| `Arith/` | `arith` | Integer and floating-point arithmetic ops on scalars and vectors |
| `ArmNeon/` | `arm_neon` | ARM NEON SIMD intrinsics |
| `ArmSME/` | `arm_sme` | ARM Scalable Matrix Extension (SME) ops |
| `ArmSVE/` | `arm_sve` | ARM Scalable Vector Extension (SVE) ops |
| `Async/` | `async` | Asynchronous execution: `async.execute`, `async.await`, coroutine-based concurrency |
| `Bufferization/` | `bufferization` | Tensor-to-buffer conversion (One-Shot Bufferization infrastructure) |
| `Complex/` | `complex` | Complex number arithmetic and math ops |
| `ControlFlow/` | `cf` | Low-level control flow: `cf.br`, `cf.cond_br`, `cf.switch`, `cf.assert` |
| `DLTI/` | `dlti` | Data Layout and Target Info attributes for target-aware compilation |
| `EmitC/` | `emitc` | C/C++ code emission ops; target for C++ code generation |
| `Func/` | `func` | Function definition, call, and return ops |
| `GPU/` | `gpu` | GPU kernel launch, thread/block indexing, memory barriers |
| `Index/` | `index` | Machine-word-sized integer ops abstracting `index` type arithmetic |
| `IRDL/` | `irdl` | IR Definition Language — runtime-interpretable dialect definitions |
| `Linalg/` | `linalg` | Structured linear algebra ops (matmul, conv, generic) and their tiling/fusion infrastructure |
| `LLVMIR/` | `llvm` | LLVM IR dialect — direct representation of LLVM IR ops and types in MLIR |
| `Math/` | `math` | Extended math functions (exp, log, sin, cos, erf, etc.) |
| `MemRef/` | `memref` | Memory reference ops: alloc, dealloc, load, store, subview, reshape |
| `MLProgram/` | `ml_program` | ML program constructs: global variables, function calls for ML frameworks |
| `MPI/` | `mpi` | MPI (Message Passing Interface) collective communication ops |
| `NVGPU/` | `nvgpu` | NVIDIA GPU-specific ops (Tensor Core wmma/wgmma, cp.async, etc.) |
| `OpenACC/` | `acc` | OpenACC GPU offloading directives |
| `OpenACCMPCommon/` | — | Shared utilities between OpenACC and OpenMP dialects |
| `OpenMP/` | `omp` | OpenMP parallel programming directives |
| `PDL/` | `pdl` | Pattern Description Language — compiled pattern representation |
| `PDLInterp/` | `pdl_interp` | PDL interpreter dialect — compiled PDL bytecode ops |
| `Ptr/` | `ptr` | Pointer type and ops for generic pointer abstraction |
| `Quant/` | `quant` | Quantization types and ops for quantized neural networks |
| `SCF/` | `scf` | Structured control flow: `scf.if`, `scf.for`, `scf.while`, `scf.forall` |
| `Shape/` | `shape` | Dynamic shape computation and constraint propagation |
| `Shard/` | `shard` | Tensor sharding annotations for distributed computation |
| `SMT/` | `smt` | SMT (Satisfiability Modulo Theories) solver interface ops |
| `SparseTensor/` | `sparse_tensor` | Sparse tensor types, encoding attributes, and sparse computation passes |
| `SPIRV/` | `spirv` | SPIR-V dialect — full representation of SPIR-V shader/compute programs |
| `Tensor/` | `tensor` | Tensor ops: `extract`, `insert`, `reshape`, `pad`, `concat`, `pack`, `unpack` |
| `Tosa/` | `tosa` | TOSA (Tensor Operator Set Architecture) — ML operator standard |
| `Transform/` | `transform` | Transform dialect — first-class IR transformation schedules |
| `UB/` | `ub` | Undefined behavior ops: `ub.poison` |
| `Utils/` | — | Shared utility code used across multiple dialects (not a dialect itself) |
| `Vector/` | `vector` | Multi-dimensional vector ops: transfer, contraction, reduction, shuffles |
| `WasmSSA/` | `wasm_ssa` | WebAssembly SSA-form representation |
| `X86/` | — | x86-specific intrinsics (AMX, etc.) |
| `XeGPU/` | `xegpu` | Intel Xe GPU ops |
| `Traits.cpp` | — | Shared op trait implementations used across multiple dialects |

## For AI Agents

### Working In This Directory
- Each dialect subdirectory is independent; changes to one dialect should not require changes to others unless through interfaces.
- Dialect implementations `#include` generated `.inc` files from `mlir/include/mlir/Dialect/<Name>/`. Run the dialect's `*IncGen` CMake targets before editing.
- The `Utils/` subdirectory contains shared helpers (e.g., `ArithUtils`, `LinalgUtils`) that are linked into multiple dialect libraries; treat it as a dialect-internal utility layer.
- `Traits.cpp` at the top level implements op traits that span multiple dialects.

### Common Patterns
- Dialect libraries are named `MLIR<Name>Dialect` in CMake.
- Op implementations split into `IR/<Name>Ops.cpp` (op semantics) and `Transforms/<Name>Passes.cpp` (pass implementations) within each dialect subdirectory.
- Interface external models (attaching interfaces to ops from other dialects) are registered in `<Name>Dialect.cpp`.

## Dependencies

### Internal
- `mlir/lib/IR/` — all core IR
- `mlir/lib/Interfaces/` — shared interfaces
- `mlir/lib/Analysis/` — for analysis-dependent passes within dialects

### External
- `llvm/lib/Support` — ADT
- Dialect-specific: SPIRV headers, CUDA/ROCm SDKs, etc.

<!-- MANUAL: -->
