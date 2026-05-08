<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToArmSME Conversion

## Purpose
Lowers Arith dialect ops to ArmSME dialect ops. Handles constant operations that produce SME tile-sized values, converting arith constants into SME tile initialization ops.

## Key Files
| File | Description |
|------|-------------|
| `ArithToArmSME.cpp` | Conversion patterns: arith constants to SME tile zero/splat ops |

## For AI Agents

### Working In This Directory
- Primarily converts `arith.constant` with zero or splat values into `arm_sme.zero` or tile broadcast ops.
- Used early in the ArmSME lowering pipeline before vectorization and tile allocation.

## Dependencies
- Source: `mlir/Dialect/Arith`
- Target: `mlir/Dialect/ArmSME`

<!-- MANUAL: -->
