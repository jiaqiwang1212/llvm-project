<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU Pipelines

## Purpose
Pre-built pass pipeline declarations for GPU compilation: host-side and device-side pipeline configurations for targeting NVVM, ROCDL, or SPIR-V backends.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pipeline registration and option struct declarations |

## For AI Agents

### Working In This Directory
- Pipelines orchestrate GPU dialect lowering through to serialized binary form
- Options control target (CUDA/ROCm/SPIRV), optimization level, and serialization format

## Dependencies
- Depends on: GPU dialect, NVVM/ROCDL/SPIRV dialects

<!-- MANUAL: -->
