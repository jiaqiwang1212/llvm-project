<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSME Transforms

## Purpose
Transformation pass declarations for the ArmSME dialect: tile allocation, outer-product vectorization, streaming-mode insertion, and layout optimization.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |

## For AI Agents

### Working In This Directory
- Tile allocation pass (`createArmSMETileAllocationPass`) assigns ZA tile IDs
- Implementations live in `lib/Dialect/ArmSME/Transforms/`

## Dependencies
- Depends on: ArmSME IR, Vector dialect, SCF dialect

<!-- MANUAL: -->
