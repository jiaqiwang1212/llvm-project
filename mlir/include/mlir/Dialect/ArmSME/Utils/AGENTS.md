<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSME Utils

## Purpose
Utility functions for the ArmSME dialect: tile type queries, SME vector type helpers, and ZA storage layout utilities.

## Key Files
| File | Description |
|------|-------------|
| `Utils.h` | SME utility functions (tile size queries, type helpers) |

## For AI Agents

### Working In This Directory
- `getSMETileSliceMinNumElts` and similar helpers query SME hardware constants
- Use these when generating tile-size-parameterized code

## Dependencies
- Depends on: ArmSME IR, Vector dialect types

<!-- MANUAL: -->
