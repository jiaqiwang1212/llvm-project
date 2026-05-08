<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU Dialect

## Purpose
Represents AMD GPU-specific operations: matrix multiply-accumulate (MFMA/WMMA), raw buffer loads/stores, LDS (local data share) operations, and other AMDGPU intrinsics not expressible in generic GPU dialect.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, attribute, and enum definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Chipset version utilities (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- ODS `.td` files in `IR/` generate C++ — edit `.td`, not generated `.h.inc` files
- Chipset-specific behavior is gated by the `Chipset` struct from `Utils/Chipset.h`

### Common Patterns
- Op names follow `amdgpu.*` convention
- Many ops map directly to AMDGPU LLVM intrinsics lowered via AMDGPUToROCDL

## Dependencies
- Depends on: GPU dialect, LLVMIR dialect, Vector dialect

<!-- MANUAL: -->
