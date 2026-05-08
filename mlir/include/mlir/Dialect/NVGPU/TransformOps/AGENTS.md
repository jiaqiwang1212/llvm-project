<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU TransformOps

## Purpose
Transform dialect extension ops for NVGPU: hopper-specific transforms such as wgmma pipelining and TMA copy lowering from Transform IR scripts.

## Key Files
| File | Description |
|------|-------------|
| `NVGPUTransformOps.h` | Transform op declarations |
| `NVGPUTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.nvgpu.*` convention
- Register via `registerNVGPUTransformOps`

## Dependencies
- Depends on: Transform IR, NVGPU dialect IR

<!-- MANUAL: -->
