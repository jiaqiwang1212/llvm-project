<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSVE IR

## Purpose
Core op and dialect definitions for the ArmSVE dialect: scalable vector arithmetic, reduction, gather/scatter, and predicate-handling ops.

## Key Files
| File | Description |
|------|-------------|
| `ArmSVE.td` | ODS op and dialect definitions |
| `ArmSVEDialect.h` | Dialect class declaration |

## For AI Agents

### Working In This Directory
- Edit `ArmSVE.td` to add new SVE ops
- SVE scalable types use `vector<[n]xT>` notation with the scalable dim marker

## Dependencies
- Depends on: Vector dialect types, LLVMIR dialect attribute conventions

<!-- MANUAL: -->
