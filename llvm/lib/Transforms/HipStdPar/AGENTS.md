<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# HipStdPar

## Purpose

HIP (Heterogeneous-compute Interface for Portability) standard parallelism offloading. Transforms C++ std::par parallel algorithms to GPU kernels for AMD and NVIDIA GPUs, enabling hardware acceleration of standard library parallelism.

## Key Files

| File | Description |
|------|-------------|
| `HipStdPar.cpp` | Main pass transforming std::par patterns to GPU kernels |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory

When modifying HIP standard parallelism transforms:

1. Recognize std::par parallel algorithm patterns (std::for_each, std::transform, etc.)
2. Analyze data dependencies and workload characteristics
3. Generate HIP kernel invocations with appropriate block/thread dimensions
4. Handle memory transfers to/from GPU where needed
5. Test with HIP SDK and GPU targets (AMD/NVIDIA)

### Common Patterns

- **Algorithm detection**: Identify std::par calls in IR (from libcxx)
- **Workload analysis**: Determine parallelism granularity and data layout
- **Kernel generation**: Create HIP kernels from parallel algorithm bodies
- **Memory management**: Insert hipMalloc/hipMemcpy for GPU memory transfers

## Dependencies

### Internal
- `llvm/include/llvm/IR/` — IR instructions, calls
- `llvm/include/llvm/Analysis/` — loop and dependency analysis
- `llvm/lib/Analysis/` — analysis implementations
- `llvm/include/llvm/CodeGen/` — code generation utilities

### External
- HIP SDK headers (hip/hip_runtime.h, etc.)
- Standard C++ library (for std::par algorithms)

<!-- MANUAL: -->
