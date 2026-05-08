<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Func Transforms

## Purpose
Transformation passes for the Func dialect: function type conversions, argument/result attribute propagation, and cross-function optimization passes.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `FuncConversions.h` | Patterns for converting function signatures and call sites |

## For AI Agents

### Working In This Directory
- `FuncConversions.h` provides `populateFuncOpTypeConversionPattern` used in dialect conversions
- Implementations live in `lib/Dialect/Func/Transforms/`

## Dependencies
- Depends on: Func IR, MLIR transform infrastructure

<!-- MANUAL: -->
