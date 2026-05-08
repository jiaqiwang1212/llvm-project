<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IndexToSPIRV

## Purpose
Declares the Index to SPIR-V lowering pass. Lowers Index dialect ops to SPIR-V integer arithmetic ops for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `IndexToSPIRV.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `IndexToSPIRV.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/IndexToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Index/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
