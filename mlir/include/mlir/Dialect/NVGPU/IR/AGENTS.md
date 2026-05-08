<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# NVGPU IR

## Purpose
Core op, type, and dialect definitions for the NVGPU dialect: wgmma ops, TMA descriptors/ops, async copies, device-async ops, and mbarrier synchronization.

## Key Files
| File | Description |
|------|-------------|
| `NVGPUDialect.h` | Dialect class declaration |
| `NVGPU.td` | Top-level ODS include |
| `NVGPUOps.td` | ODS op definitions |
| `NVGPUTypes.td` | ODS type definitions (TensorMap, mbarrier, warpgroup accumulator) |

## For AI Agents

### Working In This Directory
- Edit `NVGPUOps.td` to add new NVGPU ops
- TMA (Tensor Memory Accelerator) descriptor types encode tensor layout for hardware DMA

## Dependencies
- Depends on: GPU dialect, NVVM dialect, MemRef types

<!-- MANUAL: -->
