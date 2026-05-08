<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToAMDGPU Conversion

## Purpose
Lowers Arith dialect ops to AMDGPU dialect ops for AMD GPU-specific arithmetic. Handles non-standard float types and operations that have AMD-specific lowering paths (e.g., packed float16 arithmetic, bf16 operations on specific chipsets).

## Key Files
| File | Description |
|------|-------------|
| `ArithToAMDGPU.cpp` | Conversion patterns from arith ops to amdgpu ops for AMD-specific arithmetic |

## For AI Agents

### Working In This Directory
- Primarily handles float type conversions and arithmetic ops that AMDGPU implements natively (f16, bf16, packed ops).
- Chipset-specific patterns are guarded by `AMDGPUAttrs` chipset version checks.
- Packed float ops (operating on two f16 values in a single 32-bit register) have special patterns here.

## Dependencies
- Source: `mlir/Dialect/Arith`
- Target: `mlir/Dialect/AMDGPU`

<!-- MANUAL: -->
