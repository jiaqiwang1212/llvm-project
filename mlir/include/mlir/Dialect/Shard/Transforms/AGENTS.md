<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shard Transforms

## Purpose
Transformation passes for the Shard dialect: partitioning, resharding, and simplification passes that lower sharding annotations to explicit collective communication ops.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `Partition.h` | Partitioning pass declarations |
| `Simplify.h` | Simplification pass declarations |

## For AI Agents

### Working In This Directory
- `Partition.h` drives the main sharding annotation to explicit collective lowering
- Implementations live in `lib/Dialect/Shard/Transforms/`

## Dependencies
- Depends on: Shard IR, Shard Interfaces, MPI dialect

<!-- MANUAL: -->
