<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UBToSPIRV

## Purpose
Declares the UB (Undefined Behavior) to SPIR-V lowering pass. Lowers UB dialect poison ops to SPIR-V undef composite constants for GPU shader/kernel compilation.

## Key Files
| File | Description |
|------|-------------|
| `UBToSPIRV.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `UBToSPIRV.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/UBToSPIRV/`

## Dependencies
- Source dialect: `include/mlir/Dialect/UB/`
- Target dialect: `include/mlir/Dialect/SPIRV/`

<!-- MANUAL: -->
