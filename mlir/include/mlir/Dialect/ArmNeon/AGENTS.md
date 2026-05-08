<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmNeon Dialect

## Purpose
Represents Arm NEON SIMD intrinsic operations for AArch64. Provides access to NEON-specific instructions (e.g., 2D structured load/store, dot-product) not expressible in the generic Vector dialect.

## Key Files
| File | Description |
|------|-------------|
| `ArmNeon.td` | ODS op and dialect definitions |
| `ArmNeonDialect.h` | Dialect class declaration |
| `Transforms.h` | Transformation declarations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `TransformOps/` | Transform dialect extension for ArmNeon (see `TransformOps/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Edit `ArmNeon.td` to add intrinsic ops; ops map directly to LLVM AArch64 intrinsics
- Lowered via ArmNeon-to-LLVM conversion (generates `llvm.intr.aarch64.*` calls)

### Common Patterns
- Op names follow `arm_neon.*` convention
- `smull`/`sdot`/`usmmla` and related integer SIMD ops are primary targets

## Dependencies
- Depends on: Vector dialect, LLVMIR dialect

<!-- MANUAL: -->
