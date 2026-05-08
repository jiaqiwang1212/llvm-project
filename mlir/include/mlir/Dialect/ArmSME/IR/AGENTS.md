<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSME IR

## Purpose
Core op, interface, and enum definitions for the ArmSME dialect. Covers tile load/store, outer-product ops, ZA enable/disable, and streaming-mode control ops.

## Key Files
| File | Description |
|------|-------------|
| `ArmSME.h` | Op class declarations |
| `ArmSME.td` | Top-level ODS include |
| `ArmSMEOps.td` | Op definitions for all SME ops |
| `ArmSMEIntrinsicOps.td` | Intrinsic-level op definitions |
| `ArmSMEOpInterfaces.h` | SME-specific op interface declarations |
| `ArmSMEEnums.h` | Generated enum declarations |

## For AI Agents

### Working In This Directory
- `ArmSMEIntrinsicOps.td` holds lower-level intrinsic ops used after tile allocation
- Interface impls connect SME ops to the generic ArmSMEOpInterface

## Dependencies
- Depends on: Vector dialect types, LLVMIR dialect

<!-- MANUAL: -->
