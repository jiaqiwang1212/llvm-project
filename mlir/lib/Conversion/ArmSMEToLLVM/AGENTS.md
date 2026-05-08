<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSMEToLLVM Conversion

## Purpose
Lowers the ArmSME dialect to LLVM IR dialect. Converts SME tile operations, load/store ops, and streaming mode control to AArch64 SME LLVM intrinsics (`llvm.intr.aarch64.sme.*`).

## Key Files
| File | Description |
|------|-------------|
| `ArmSMEToLLVM.cpp` | All conversion patterns from ArmSME ops to LLVM SME intrinsics |

## For AI Agents

### Working In This Directory
- SME tile ops map to intrinsics like `llvm.intr.aarch64.sme.mopa.*` (outer product accumulate).
- Tile IDs (assigned by `TileAllocation` pass) become integer operands to the intrinsics.
- Streaming mode intrinsics (`smstart`/`smstop`) are inserted by `EnableArmStreaming` before this lowering.
- Must run after `ArmSMEToSCF` (which lowers multi-tile iteration) and tile allocation.

## Dependencies
- Source: `mlir/Dialect/ArmSME`
- Target: `mlir/Dialect/LLVMIR` (SME intrinsics)

<!-- MANUAL: -->
