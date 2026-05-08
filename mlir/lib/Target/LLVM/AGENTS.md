<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/LLVM

## Purpose
Provides shared infrastructure for LLVM-based compilation targets. `ModuleToObject` drives the LLVM IR → object file / PTX / AMDGPU ISA pipeline. Subdirectories provide target-specific specializations for NVVM (CUDA), ROCDL (ROCm/HIP), and XeVM (Intel GPU).

## Key Files
| File | Description |
|------|-------------|
| `ModuleToObject.cpp` | `ModuleToObject`: takes an `llvm::Module`, runs LLVM optimization passes, and compiles to an object blob; used as base class for GPU targets |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `NVVM/` | NVVM target: compiles LLVM IR to PTX / CUBIN for NVIDIA GPUs using libNVVM or the LLVM NVPTX backend |
| `ROCDL/` | ROCDL target: compiles LLVM IR to AMDGPU ISA using the LLVM AMDGPU backend for ROCm/HIP |
| `XeVM/` | XeVM target: compiles LLVM IR to Intel GPU ISA using the Intel LLVM fork or SPIRV path |

## For AI Agents

### Working In This Directory
- `ModuleToObject` is the base class; GPU targets subclass and override `serializeToObject()`.
- Each subdirectory adds its own pass pipeline configuration and GPU-SDK calls.
- These targets are only available when the corresponding LLVM targets are compiled in.

### Common Patterns
- Target compilation result is a `std::string` blob (PTX text or binary ISA).
- GPU kernel attributes (grid/block dims, shared memory size) are attached as `llvm.metadata` before compilation.

## Dependencies

### Internal
- `mlir/lib/Target/LLVMIR/` — MLIR → `llvm::Module` translation (prerequisite)
- `mlir/lib/Dialect/GPU/` — GPU dialect op definitions

### External
- `llvm/lib/IR`, LLVM Target libraries (NVPTX, AMDGPU, SPIRV)
- libNVVM, ROCm device libs (optional)

<!-- MANUAL: -->
