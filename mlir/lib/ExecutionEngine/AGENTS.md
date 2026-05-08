<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/ExecutionEngine

## Purpose
Implements MLIR's JIT execution engine and all runtime support libraries. The execution engine compiles MLIR modules to native code via LLVM ORC JIT. The runtime utilities (CRunnerUtils, AsyncRuntime, SparseTensorRuntime, etc.) are standalone shared libraries linked at JIT time to provide C-callable implementations of MLIR dialect lowerings.

## Key Files
| File | Description |
|------|-------------|
| `ExecutionEngine.cpp` | `mlir::ExecutionEngine`: wraps LLVM ORC JIT; compiles MLIR modules lowered to LLVM dialect, manages symbol resolution and shared library loading, provides `invoke()` for calling exported functions |
| `JitRunner.cpp` | `mlir::JitRunnerMain()`: command-line driver for JIT-compiling and running MLIR files; used by `mlir-cpu-runner` and dialect-specific runners |
| `OptUtils.cpp` | LLVM optimization pipeline utilities used by the JIT runner (wraps `llvm::PassBuilder` for O0/O1/O2/O3 pipelines) |
| `CRunnerUtils.cpp` | C runtime helpers for MemRef descriptors: `printMemrefF32`, `printMemrefI64`, etc.; linked into JIT sessions |
| `RunnerUtils.cpp` | Additional runner utilities: print functions for vectors, timing helpers |
| `AsyncRuntime.cpp` | Async dialect runtime: coroutine scheduling, async value/group management; linked for programs using `async.execute` |
| `CudaRuntimeWrappers.cpp` | CUDA runtime wrappers for GPU dialect lowerings; wraps CUDA driver API calls |
| `RocmRuntimeWrappers.cpp` | ROCm/HIP runtime wrappers for AMDGPU GPU dialect lowerings |
| `VulkanRuntime.cpp` / `VulkanRuntimeWrappers.cpp` | Vulkan runtime: SPIR-V module loading, pipeline dispatch, buffer management for GPU-via-Vulkan lowerings |
| `LevelZeroRuntimeWrappers.cpp` | oneAPI Level Zero runtime wrappers for Intel GPU lowerings |
| `SyclRuntimeWrappers.cpp` | SYCL runtime wrappers |
| `SpirvCpuRuntimeWrappers.cpp` | SPIR-V CPU emulation runtime wrappers |
| `ArmRunnerUtils.cpp` | ARM-specific runner utilities (e.g., SVE vector length queries) |
| `ArmSMEStubs.cpp` | Stub implementations for ARM SME (Scalable Matrix Extension) runtime calls |
| `APFloatWrappers.cpp` | Runtime wrappers that call `llvm::APFloat` for software floating-point operations |
| `Float16bits.cpp` | Half-precision (f16/bf16) bit manipulation utilities for runtime use |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `SparseTensor/` | Sparse tensor runtime library: storage formats, COO/CSR/CSC manipulation, file I/O (see `SparseTensor/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `ExecutionEngine.cpp` requires LLVM ORC JIT support; guarded by `MLIR_ENABLE_EXECUTION_ENGINE` cmake option.
- Runtime utilities (`CRunnerUtils.cpp`, `AsyncRuntime.cpp`, etc.) are compiled as separate shared libraries (`libmlir_c_runner_utils`, `libmlir_async_runtime`, etc.) — they must not link against the MLIR libraries to avoid circular deps at JIT load time.
- When adding a new runtime wrapper (e.g., a new GPU backend), add the `.cpp` here and add a new CMake shared-lib target in `CMakeLists.txt`.

### Common Patterns
- Runtime functions use `extern "C"` linkage and flat C types (no MLIR types) so the JIT can find them by symbol name.
- MemRef descriptors are passed as `StridedMemRefType<T, N>` structs defined in `mlir/include/mlir/ExecutionEngine/CRunnerUtils.h`.

## Dependencies

### Internal
- `mlir/lib/IR/` — for compiling the input module
- `mlir/lib/Target/LLVMIR/` — lowers MLIR to LLVM IR before JIT

### External
- LLVM ORC JIT (`llvm/lib/ExecutionEngine/`)
- CUDA/ROCm/Vulkan/Level Zero SDKs (optional, guarded by cmake flags)

<!-- MANUAL: -->
