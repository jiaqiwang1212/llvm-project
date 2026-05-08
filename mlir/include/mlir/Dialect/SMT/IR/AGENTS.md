<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SMT IR

## Purpose
Core op, type, and attribute definitions for the SMT dialect: Boolean, bit-vector, integer, and array ops plus dialect and visitor infrastructure.

## Key Files
| File | Description |
|------|-------------|
| `SMTOps.h` | Op class declarations |
| `SMTOps.td` | Top-level ODS include for SMT ops |
| `SMT.td` | Dialect-level ODS include |
| `SMTDialect.h` | Dialect class declaration |
| `SMTDialect.td` | Dialect definition |
| `SMTBitVectorOps.td` | Bit-vector op definitions |
| `SMTIntOps.td` | Integer theory op definitions |
| `SMTArrayOps.td` | Array theory op definitions |
| `SMTTypes.h` | Type class declarations |
| `SMTTypes.td` | ODS type definitions |
| `SMTAttributes.h` | Attribute class declarations |
| `SMTAttributes.td` | ODS attribute definitions |
| `SMTVisitors.h` | Visitor pattern helpers for SMT ops |

## For AI Agents

### Working In This Directory
- `SMTVisitors.h` provides type-safe dispatch over SMT op/type variants
- Edit `SMTBitVectorOps.td` / `SMTIntOps.td` etc. to add theory-specific ops

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
