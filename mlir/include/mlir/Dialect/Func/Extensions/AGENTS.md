<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Func Extensions

## Purpose
Extension registrations for the Func dialect: inliner interface implementation and sharding extensions.

## Key Files
| File | Description |
|------|-------------|
| `AllExtensions.h` | Registers all Func dialect extensions |
| `InlinerExtension.h` | Inliner interface implementation for func ops |
| `ShardingExtensions.h` | Sharding interface implementations for func ops |

## For AI Agents

### Working In This Directory
- Call `registerAllExtensions(registry)` in driver/tool setup to enable inlining of func ops
- The inliner extension is required for the standard MLIR inliner to handle `func.call`

## Dependencies
- Depends on: Func IR, Shard interfaces, MLIR Transform infrastructure

<!-- MANUAL: -->
