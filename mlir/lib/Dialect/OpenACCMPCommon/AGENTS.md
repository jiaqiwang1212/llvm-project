<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenACCMPCommon Dialect

## Purpose
Implements shared infrastructure between the OpenACC and OpenMP dialects. Contains interfaces and utilities common to both directive-based parallel programming models: clause interfaces, data clause handling, and shared reduction logic.

## Key Files
| File | Description |
|------|-------------|
| `Interfaces/` | Shared clause interfaces used by both OpenACC and OpenMP ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Interfaces/` | Common interfaces for directive-based parallel ops (data clauses, reductions, etc.) |

## For AI Agents

### Working In This Directory
- This directory contains no executable ops — only interface definitions shared between `mlir/Dialect/OpenACC` and `mlir/Dialect/OpenMP`.
- New shared clause interface types go here to avoid duplication between the two dialects.
- Interfaces here follow the MLIR `OpInterface` / `AttrInterface` pattern defined in TableGen under `mlir/include/mlir/Dialect/OpenACCMPCommon/`.

### Common Patterns
- Interface methods are declared in TableGen `.td` files in the corresponding `include/` directory; `.cpp` implementations in `Interfaces/` provide default implementations.

## Dependencies
- `mlir/IR`, `mlir/Dialect/OpenACC`, `mlir/Dialect/OpenMP`

<!-- MANUAL: -->
