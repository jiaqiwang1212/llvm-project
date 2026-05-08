<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToAMX

## Purpose
Declares the Vector to AMX lowering pass. Lowers Vector dialect matrix multiply ops to Intel AMX (Advanced Matrix Extensions) tile intrinsics for x86 matrix acceleration.

## Key Files
| File | Description |
|------|-------------|
| `VectorToAMX.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `VectorToAMX.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/VectorToAMX/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Vector/`
- Target dialect: `include/mlir/Dialect/X86Vector/` (AMX ops)

<!-- MANUAL: -->
