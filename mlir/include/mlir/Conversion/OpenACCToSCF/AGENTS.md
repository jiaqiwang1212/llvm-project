<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACCToSCF

## Purpose
Declares the OpenACC to SCF lowering pass. Lowers OpenACC dialect ops to SCF structured control flow ops as an intermediate step in the OpenACC GPU compilation pipeline.

## Key Files
| File | Description |
|------|-------------|
| `ConvertOpenACCToSCF.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ConvertOpenACCToSCF.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/OpenACCToSCF/`

## Dependencies
- Source dialect: `include/mlir/Dialect/OpenACC/`
- Target dialect: `include/mlir/Dialect/SCF/`

<!-- MANUAL: -->
