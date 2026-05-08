<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSVE TransformOps

## Purpose
Transform dialect extension ops for targeting ArmSVE scalable vector operations from generic vector code.

## Key Files
| File | Description |
|------|-------------|
| `ArmSVEVectorTransformOps.h` | Transform op declarations |
| `ArmSVEVectorTransformOps.td` | ODS definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.arm_sve.*` convention
- Register via `registerArmSVEVectorTransformOps`

## Dependencies
- Depends on: Transform IR, ArmSVE dialect IR, Vector dialect

<!-- MANUAL: -->
