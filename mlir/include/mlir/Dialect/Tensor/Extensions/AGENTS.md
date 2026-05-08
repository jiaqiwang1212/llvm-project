<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor Extensions

## Purpose
Extension registrations for the Tensor dialect: sharding interface implementations and a convenience header to register all extensions.

## Key Files
| File | Description |
|------|-------------|
| `AllExtensions.h` | Registers all Tensor dialect extensions |
| `ShardingExtensions.h` | Sharding interface implementations for tensor ops |

## For AI Agents

### Working In This Directory
- Call `registerAllExtensions(registry)` in driver/tool setup
- Sharding extensions enable tensor ops to participate in the Shard dialect distribution framework

## Dependencies
- Depends on: Tensor IR, Shard interfaces

<!-- MANUAL: -->
