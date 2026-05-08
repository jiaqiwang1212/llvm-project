<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmNeon2dToIntr Conversion

## Purpose
Lowers ArmNeon 2D matrix ops to LLVM AArch64 NEON intrinsic calls. Converts `arm_neon.2d.*` ops (SDOT, UDOT, SMMLA, UMMLA) to `llvm.intr.aarch64.neon.*` intrinsics.

## Key Files
| File | Description |
|------|-------------|
| `ArmNeon2dToIntr.cpp` | Conversion patterns from 2D Neon ops to LLVM intrinsics |

## For AI Agents

### Working In This Directory
- Each `arm_neon.2d.*` op maps to a specific LLVM intrinsic by name; the mapping is straightforward 1:1.
- Type checking ensures vector operand shapes match the intrinsic's expected types.
- This is one of the final lowering steps before LLVM IR export for AArch64 targets.

## Dependencies
- Source: `mlir/Dialect/ArmNeon`
- Target: `mlir/Dialect/LLVMIR` (NEON intrinsics)

<!-- MANUAL: -->
