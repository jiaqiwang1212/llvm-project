<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU Transforms

## Purpose
Transformation passes for the NVGPU dialect: wgmma pipelining, TMA descriptor creation, async copy optimization, and other Hopper-specific lowering passes.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `Utils.h` | Shared utilities for NVGPU transforms |

## For AI Agents

### Working In This Directory
- Implementations live in `lib/Dialect/NVGPU/Transforms/`
- `Utils.h` provides helpers shared across multiple NVGPU transform passes

## Dependencies
- Depends on: NVGPU IR, GPU dialect, Vector dialect

<!-- MANUAL: -->
