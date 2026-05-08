<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# EmitC Transforms

## Purpose
Transformation passes and type conversion utilities for the EmitC dialect: expression formation, variable hoisting, and type converter for dialect conversions targeting EmitC.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `TypeConversions.h` | Type converter for converting to EmitC types |

## For AI Agents

### Working In This Directory
- `TypeConversions.h` provides the type converter used in dialect-to-EmitC conversion passes
- Implementations live in `lib/Dialect/EmitC/Transforms/`

## Dependencies
- Depends on: EmitC IR

<!-- MANUAL: -->
