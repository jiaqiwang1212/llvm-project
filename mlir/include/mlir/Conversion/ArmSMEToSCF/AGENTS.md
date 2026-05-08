<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSMEToSCF

## Purpose
Declares the ArmSME to SCF lowering pass. Lowers Arm SME tile-level ops that require loop constructs into SCF (Structured Control Flow) loops before further lowering.

## Key Files
| File | Description |
|------|-------------|
| `ArmSMEToSCF.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArmSMEToSCF.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArmSMEToSCF/`

## Dependencies
- Source dialect: `include/mlir/Dialect/ArmSME/`
- Target dialect: `include/mlir/Dialect/SCF/`

<!-- MANUAL: -->
