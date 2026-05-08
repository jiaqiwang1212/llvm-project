<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shard Interfaces

## Purpose
Sharding interface definitions used across multiple dialects to express how ops can be distributed across a device mesh. The `ShardingInterface` connects ops to the sharding inference and partition framework.

## Key Files
| File | Description |
|------|-------------|
| `ShardingInterface.h` | ShardingInterface declaration |
| `ShardingInterface.td` | ODS interface definition |
| `ShardingInterfaceImpl.h` | Default ShardingInterface implementations |

## For AI Agents

### Working In This Directory
- Implement `ShardingInterface` on compute ops (Linalg, Arith, etc.) to enable auto-sharding
- `ShardingInterfaceImpl.h` provides reusable default implementations for structured ops

## Dependencies
- Used by: Linalg, Arith, Tensor, Bufferization, Func dialects

<!-- MANUAL: -->
