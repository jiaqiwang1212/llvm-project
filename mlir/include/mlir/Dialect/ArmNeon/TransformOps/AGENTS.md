<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmNeon TransformOps

## Purpose
Transform dialect extension ops for targeting ArmNeon vector operations. Enables script-driven lowering of generic vector ops to NEON-specific patterns.

## Key Files
| File | Description |
|------|-------------|
| `ArmNeonVectorTransformOps.h` | Transform op declarations |
| `ArmNeonVectorTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.arm_neon.*` convention
- Register via `registerArmNeonVectorTransformOps`

## Dependencies
- Depends on: Transform IR, ArmNeon dialect IR, Vector dialect

<!-- MANUAL: -->
