<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization Extensions

## Purpose
Extension registrations for the Bufferization dialect: sharding interface implementations and a convenience header to register all extensions at once.

## Key Files
| File | Description |
|------|-------------|
| `AllExtensions.h` | Registers all bufferization dialect extensions |
| `ShardingExtensions.h` | Sharding interface implementations for bufferization ops |

## For AI Agents

### Working In This Directory
- Call `registerAllExtensions(registry)` from `AllExtensions.h` in tool/driver setup
- Sharding extensions connect bufferization ops to the Shard dialect infrastructure

## Dependencies
- Depends on: Bufferization IR, Shard interfaces

<!-- MANUAL: -->
