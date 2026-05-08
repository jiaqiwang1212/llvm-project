<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shard Dialect

## Purpose
Provides sharding/distribution ops and interfaces for expressing tensor distribution across a mesh of devices. Enables data-parallel and model-parallel distribution strategies in MLIR IR.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Interfaces/` | Sharding interface definitions (see `Interfaces/AGENTS.md`) |
| `IR/` | Op and dialect definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `shard.mesh` defines the device mesh topology
- `shard.annotate` attaches sharding annotations to tensor values
- Lowered to MPI or other collectives via ShardToMPI

### Common Patterns
- Op names follow `shard.*` convention
- Sharding specs use axis-based notation to describe tensor dimension distribution

## Dependencies
- Depends on: Tensor dialect, MPI dialect (for lowering), Func dialect

<!-- MANUAL: -->
