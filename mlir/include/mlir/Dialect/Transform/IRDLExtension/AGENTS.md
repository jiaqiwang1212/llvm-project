<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform IRDLExtension

## Purpose
Transform dialect extension that integrates IRDL-defined pattern constraints with the Transform dialect, enabling IRDL-based op matching and manipulation in Transform scripts.

## Key Files
| File | Description |
|------|-------------|
| `IRDLExtension.h` | Extension registration declaration |
| `IRDLExtensionOps.h` | Op class declarations |
| `IRDLExtensionOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Op names follow `transform.irdl.*` convention
- Allows IRDL dialect definitions to be used as structural predicates in transform scripts

## Dependencies
- Depends on: Transform IR, IRDL dialect

<!-- MANUAL: -->
