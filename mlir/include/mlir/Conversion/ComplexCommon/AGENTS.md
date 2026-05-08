<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexCommon

## Purpose
Provides shared utilities for Complex dialect conversion passes. Contains division conversion helpers reused across multiple Complex lowering targets.

## Key Files
| File | Description |
|------|-------------|
| `DivisionConverter.h` | Shared complex division conversion utilities used by multiple Complex target passes |

## For AI Agents

### Working In This Directory
- Edit `DivisionConverter.h` to add shared conversion helpers
- This is a utility directory, not a standalone pass; do not register passes here

## Dependencies
- Source dialect: `include/mlir/Dialect/Complex/`
- Consumers: `ComplexToLLVM/`, `ComplexToStandard/`, and other Complex conversion passes

<!-- MANUAL: -->
