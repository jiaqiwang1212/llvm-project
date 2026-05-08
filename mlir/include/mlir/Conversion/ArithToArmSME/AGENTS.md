<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToArmSME

## Purpose
Declares the Arith to ArmSME lowering pass. Lowers Arith dialect constants and arithmetic ops to Arm Scalable Matrix Extension (SME) dialect ops for matrix-tile-level computation.

## Key Files
| File | Description |
|------|-------------|
| `ArithToArmSME.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArithToArmSME.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArithToArmSME/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Target dialect: `include/mlir/Dialect/ArmSME/`

<!-- MANUAL: -->
