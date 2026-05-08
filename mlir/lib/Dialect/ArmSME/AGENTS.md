<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArmSME Dialect

## Purpose
Implements the Arm Scalable Matrix Extension (SME) dialect. SME provides 2D tile registers for matrix operations on AArch64 with scalable tile sizes. This dialect models SME tile load/store, outer product accumulate, and streaming mode control.

## Key Files
| File | Description |
|------|-------------|
| `IR/ArmSME.cpp` | Op implementations and verifiers |
| `IR/Utils.cpp` | SME-specific utility functions (tile type queries, streaming mode helpers) |
| `Transforms/EnableArmStreaming.cpp` | Inserts streaming mode entry/exit around SME kernel regions |
| `Transforms/OuterProductFusion.cpp` | Fuses chains of outer products into combined MOPA instructions |
| `Transforms/TileAllocation.cpp` | Allocates SME tile registers to ops |
| `Transforms/VectorLegalization.cpp` | Legalizes vector ops for SME tile constraints |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Tile ops, streaming attributes, utility functions |
| `Transforms/` | Streaming mode insertion, tile allocation, outer product fusion |

## For AI Agents

### Working In This Directory
- SME tiles are scalable 2D: size is `[vscale * M, vscale * N]`; ops carry tile ID attributes.
- `TileAllocation.cpp` runs before lowering; it assigns concrete tile IDs to abstract tile values.
- `EnableArmStreaming.cpp` wraps functions that use SME ops with `smstart`/`smstop` intrinsics.
- Scalable vector types (`!svbool`, `!svint8`) from the SVE dialect are also used here.

### Common Patterns
- Verifiers check tile element type matches the op's expected type via SME hardware constraints.
- `OuterProductFusion` uses a peephole rewriter matching consecutive `arm_sme.outerproduct` ops.

## Dependencies
- `mlir/Dialect/ArmSVE`, `mlir/Dialect/Vector`, `mlir/Dialect/LLVMIR`, `mlir/Dialect/Func`

<!-- MANUAL: -->
