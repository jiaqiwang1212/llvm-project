<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRDL IR

## Purpose
Core op, type, attribute, interface, and trait definitions for the IRDL dialect.

## Key Files
| File | Description |
|------|-------------|
| `IRDL.h` | Op class declarations |
| `IRDL.td` | Top-level ODS include |
| `IRDLOps.td` | ODS op definitions |
| `IRDLAttributes.td` | Attribute definitions for IRDL constraints |
| `IRDLTypes.td` | Type definitions |
| `IRDLInterfaces.h` | Interface declarations |
| `IRDLInterfaces.td` | ODS interface definitions |
| `IRDLTraits.h` | Trait declarations |

## For AI Agents

### Working In This Directory
- Edit `IRDLOps.td` to add new IRDL constraint or definition ops
- `IRDLInterfaces` defines the `VerifyConstraintInterface` and `VerifyRegionInterface`

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
