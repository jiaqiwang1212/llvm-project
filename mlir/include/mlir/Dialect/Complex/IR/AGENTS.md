<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Complex IR

## Purpose
Core op, attribute, and dialect definitions for the Complex dialect.

## Key Files
| File | Description |
|------|-------------|
| `Complex.h` | Op class declarations |
| `ComplexBase.td` | Dialect definition and base classes |
| `ComplexOps.td` | ODS op definitions |
| `ComplexAttributes.td` | Attribute definitions |

## For AI Agents

### Working In This Directory
- Edit `ComplexOps.td` to add new complex arithmetic ops
- Ops work on the built-in `complex<T>` parameterized type

## Dependencies
- Depends on: MLIR built-in complex type

<!-- MANUAL: -->
