<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSVE Dialect

## Purpose
Represents Arm Scalable Vector Extension (SVE) operations for AArch64: scalable vector arithmetic, predicated execution, and SVE-specific intrinsics not covered by the generic Vector dialect.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and dialect definitions (see `IR/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension for ArmSVE (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- SVE uses scalable vector types (`vector<[4]xf32>`); ensure scalable dims are handled
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names follow `arm_sve.*` convention
- Lowered to LLVM AArch64 SVE intrinsics via ArmSVE-to-LLVM conversion

## Dependencies
- Depends on: Vector dialect, LLVMIR dialect

<!-- MANUAL: -->
