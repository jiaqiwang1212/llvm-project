<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCF Utils

## Purpose
Utility functions for SCF dialect: affine canonicalization utilities and general SCF helper functions for loop manipulation and analysis.

## Key Files
| File | Description |
|------|-------------|
| `Utils.h` | General SCF utility functions (loop normalization, cloning, etc.) |
| `AffineCanonicalizationUtils.h` | Utilities for canonicalizing affine ops within SCF loops |

## For AI Agents

### Working In This Directory
- `normalizeLoop` from `Utils.h` normalizes `scf.for` to start=0, step=1 form
- Affine canonicalization converts affine.apply to arith ops within scf loop bodies

## Dependencies
- Depends on: SCF IR, Affine dialect

<!-- MANUAL: -->
