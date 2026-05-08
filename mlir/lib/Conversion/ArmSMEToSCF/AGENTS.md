<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSMEToSCF Conversion

## Purpose
Lowers ArmSME multi-tile iteration ops to SCF loops. Converts `arm_sme.tile_load`/`arm_sme.tile_store` with scalable iteration patterns to `scf.for` loops over tile rows/columns.

## Key Files
| File | Description |
|------|-------------|
| `ArmSMEToSCF.cpp` | Patterns converting SME tile iteration ops to scf loops |

## For AI Agents

### Working In This Directory
- SME tile loads/stores with 2D scalable iteration become explicit loops over rows since SME hardware operates row-by-row.
- This pass must run before `ArmSMEToLLVM` — it produces SCF loops that are then lowered via `SCFToControlFlow`.
- Scalable vector sizes use `vscale * element_count` arithmetic to compute loop bounds.

## Dependencies
- Source: `mlir/Dialect/ArmSME`
- Target: `mlir/Dialect/SCF`, `mlir/Dialect/ArmSME` (residual ops)

<!-- MANUAL: -->
