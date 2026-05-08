<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Async IR

## Purpose
Core op, type, and dialect definitions for the Async dialect. Defines `async.token`, `async.value<T>`, `async.group`, and execution ops.

## Key Files
| File | Description |
|------|-------------|
| `Async.h` | Op and type class declarations |
| `AsyncDialect.td` | Dialect definition |
| `AsyncOps.td` | ODS op definitions |
| `AsyncTypes.h` | Type class declarations |
| `AsyncTypes.td` | ODS type definitions |

## For AI Agents

### Working In This Directory
- Edit `AsyncOps.td` for new ops; `AsyncTypes.td` for new async types
- `async.token` represents a completion signal; `async.value<T>` wraps a future value

## Dependencies
- Depends on: MLIR IR core types

<!-- MANUAL: -->
