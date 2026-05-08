<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Arith IR

## Purpose
Core op and interface definitions for the Arith dialect: integer/float arithmetic, bitwise ops, comparisons, and type casts.

## Key Files
| File | Description |
|------|-------------|
| `Arith.h` | Op class declarations |
| `ArithBase.td` | Dialect definition and base classes |
| `ArithOps.td` | ODS op definitions for all arith ops |
| `ArithOpsInterfaces.td` | Interface definitions (FastMath, overflow flags) |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds interface implementations |

## For AI Agents

### Working In This Directory
- Edit `ArithOps.td` to add ops; interfaces in `ArithOpsInterfaces.td`
- Signless integer convention: use `arith.extsi`/`arith.extui` for signed/unsigned extension

### Common Patterns
- Comparison ops (`arith.cmpi`, `arith.cmpf`) take predicate enum attributes
- Float ops may carry `FastMathFlags` via `ArithFastMathInterface`

## Dependencies
- No dialect dependencies (foundational)

<!-- MANUAL: -->
