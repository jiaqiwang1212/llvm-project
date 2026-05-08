<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU Utils

## Purpose
MMA (matrix multiply-accumulate) utility functions for NVGPU: helpers for reasoning about wgmma tile shapes, fragmented register layouts, and MMA operand types.

## Key Files
| File | Description |
|------|-------------|
| `MMAUtils.h` | MMA shape, layout, and operand utility functions |

## For AI Agents

### Working In This Directory
- Use `MMAUtils.h` when generating wgmma ops to compute correct tile sizes and accumulator shapes

## Dependencies
- Depends on: NVGPU IR, Vector dialect

<!-- MANUAL: -->
