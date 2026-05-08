<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSME Dialect

## Purpose
Represents Arm Scalable Matrix Extension (SME) operations for AArch64. Models tile registers, outer-product accumulation, and ZA storage array operations used in matrix/ML workloads on SME-capable hardware.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, interface, and enum definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | SME utility helpers (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- SME tile registers are hardware-defined; tile ID allocation is handled during lowering
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names follow `arm_sme.*` convention
- Lowering path: ArmSME → SCF → LLVM intrinsics

## Dependencies
- Depends on: Vector dialect, SCF dialect, LLVMIR dialect

<!-- MANUAL: -->
