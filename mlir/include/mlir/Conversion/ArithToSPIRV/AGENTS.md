<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToSPIRV

## Purpose
Declares the Arith to SPIR-V lowering pass. Lowers Arith dialect arithmetic and comparison ops to SPIR-V dialect ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `ArithToSPIRV.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ArithToSPIRV.h` to add or modify pass declarations and pattern helpers
- The implementation lives under `mlir/lib/Conversion/ArithToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Arith/`
- Target dialect: `include/mlir/Dialect/SPIRV/`
- Shared utilities: `ArithCommon/`

<!-- MANUAL: -->
