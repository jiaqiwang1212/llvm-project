<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ReconcileUnrealizedCasts

## Purpose
Declares the ReconcileUnrealizedCasts cleanup pass. Eliminates `unrealized_conversion_cast` ops left over from dialect conversion by folding or removing redundant cast chains.

## Key Files
| File | Description |
|------|-------------|
| `ReconcileUnrealizedCasts.h` | Pass entry-points |

## For AI Agents

### Working In This Directory
- Edit `ReconcileUnrealizedCasts.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ReconcileUnrealizedCasts/`
- This pass is typically run as a cleanup step after all dialect conversions

## Dependencies
- Operates on `builtin.unrealized_conversion_cast` ops produced by any conversion pass

<!-- MANUAL: -->
