<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToArmSME

## Purpose
Declares the Vector to ArmSME lowering pass. Lowers Vector dialect matrix and tile ops to Arm SME (Scalable Matrix Extension) dialect ops for Arm matrix acceleration.

## Key Files
| File | Description |
|------|-------------|
| `VectorToArmSME.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `VectorToArmSME.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/VectorToArmSME/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/ArmSME/`

<!-- MANUAL: -->
