<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# EmitC IR

## Purpose
Core op, type, attribute, and interface definitions for the EmitC dialect.

## Key Files
| File | Description |
|------|-------------|
| `EmitC.h` | Op class declarations |
| `EmitC.td` | Top-level ODS include |
| `EmitCBase.td` | Dialect definition and base classes |
| `EmitCOps.td` | Op definitions (via EmitC.td) |
| `EmitCAttributes.td` | Attribute definitions (opaque, CExpression, etc.) |
| `EmitCTypes.td` | Type definitions (ptr, array, opaque, lvalue) |
| `EmitCInterfaces.h` | Interface declarations |
| `EmitCInterfaces.td` | ODS interface definitions |

## For AI Agents

### Working In This Directory
- `emitc.opaque` type wraps arbitrary C type strings; use sparingly
- `emitc.lvalue` represents an l-value reference in generated C

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
