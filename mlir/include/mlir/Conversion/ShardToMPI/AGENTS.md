<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ShardToMPI

## Purpose
Declares the Shard to MPI lowering pass. Lowers mesh/shard dialect communication ops to MPI dialect calls for distributed multi-device tensor computation.

## Key Files
| File | Description |
|------|-------------|
| `ShardToMPI.h` | Pass entry-points and pattern population functions |

## For AI Agents

### Working In This Directory
- Edit `ShardToMPI.h` to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ShardToMPI/`

## Dependencies
- Source dialect: `include/mlir/Dialect/Mesh/` (shard ops)
- Target dialect: `include/mlir/Dialect/MPI/`

<!-- MANUAL: -->
