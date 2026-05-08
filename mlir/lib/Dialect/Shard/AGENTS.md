<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shard Dialect

## Purpose
Implements the Shard dialect — provides tensor sharding annotations and transformations for distributed/multi-device execution. Models device meshes, tensor shard specifications, and collective communication ops (`shard.all_reduce`, `shard.all_to_all`, etc.) used in data-parallel and model-parallel ML training.

## Key Files
| File | Description |
|------|-------------|
| `IR/ShardOps.cpp` | Op implementations: `shard.mesh`, `shard.shard`, collective communication ops |
| `Transforms/ShardingPropagation.cpp` | Propagates sharding annotations through a computation graph |
| `Transforms/Partition.cpp` | Partitions a tensor program into per-device sub-programs |
| `Transforms/Simplify.cpp` | Simplifies shard annotations (removes redundant sharding) |
| `Transforms/Transforms.cpp` | General shard transformation infrastructure |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Mesh, shard, and collective ops |
| `Transforms/` | Sharding propagation, partitioning, simplification |
| `Interfaces/` | Sharding interface definitions |

## For AI Agents

### Working In This Directory
- `shard.mesh` defines a logical device mesh with a named grid of devices; ops reference it by symbol.
- `shard.shard` annotates a tensor value with a sharding specification (which dimensions map to which mesh axes).
- `ShardingPropagation.cpp` uses the `ShardingInterface` (implemented by linalg and other ops) to infer shardings from annotated inputs/outputs.
- `Partition.cpp` splits a module into per-device programs, inserting collective communication ops at shard boundaries.
- The lowering path: `ShardToMPI` converts collective ops to MPI calls.

### Common Patterns
- Sharding interface: ops implement `ShardingInterface::getShardingOption()` to specify how their iteration space can be partitioned.
- Propagation: forward and backward passes through the IR use interface queries to infer unspecified shardings.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/Tensor`, `mlir/Dialect/Linalg`, `mlir/Dialect/MPI`

<!-- MANUAL: -->
