<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shard IR

## Purpose
Core op and dialect definitions for the Shard dialect: mesh topology ops, sharding annotation ops, and collective communication ops.

## Key Files
| File | Description |
|------|-------------|
| `ShardDialect.h` | Dialect class declaration |
| `ShardBase.td` | Dialect definition and base classes |
| `ShardOps.h` | Op class declarations |
| `ShardOps.td` | ODS op definitions |

## For AI Agents

### Working In This Directory
- Edit `ShardOps.td` to add new sharding or collective ops
- `shard.mesh` defines logical device topology; `shard.annotate` attaches distribution specs

## Dependencies
- Depends on: Tensor dialect types, MLIR IR core

<!-- MANUAL: -->
