<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GPU Dialect

## Purpose
Provides a target-agnostic GPU programming model: GPU modules, kernel functions, launch ops, thread/block index intrinsics, barriers, shuffles, and memory hierarchy ops. Acts as a common IR layer above NVVM, ROCDL, and SPIR-V targets.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, interface, and attribute definitions (see `IR/AGENTS.md`) |
| `Pipelines/` | Pre-built GPU compilation pipelines (see `Pipelines/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | GPU utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files
- GPU kernels live inside `gpu.module` ops with `gpu.func` functions

### Common Patterns
- `gpu.launch` / `gpu.launch_func` are the primary host-side launch ops
- Thread/block dims: `gpu.thread_id`, `gpu.block_id`, `gpu.grid_dim`, `gpu.block_dim`

## Dependencies
- Depends on: MemRef dialect, LLVMIR dialect (for lowering paths)

<!-- MANUAL: -->
