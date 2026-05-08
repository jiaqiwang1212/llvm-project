<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDL IR

## Purpose
Core op, type, and dialect definitions for the PDL (Pattern Description Language) dialect.

## Key Files
| File | Description |
|------|-------------|
| `PDL.h` | Op and type class declarations |
| `PDLDialect.td` | Dialect definition |
| `PDLOps.h` | Op class declarations |
| `PDLOps.td` | ODS op definitions |
| `PDLTypes.h` | Type class declarations |
| `PDLTypes.td` | ODS type definitions (operation, value, type, attribute, range) |

## For AI Agents

### Working In This Directory
- Edit `PDLOps.td` to add new PDL ops; `PDLTypes.td` for new PDL types
- `pdl.apply_native_constraint` / `pdl.apply_native_rewrite` call registered C++ functions

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
