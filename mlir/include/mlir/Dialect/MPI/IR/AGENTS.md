<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MPI IR

## Purpose
Core op, type definitions, and utility headers for the MPI dialect.

## Key Files
| File | Description |
|------|-------------|
| `MPI.h` | Op class declarations |
| `MPI.td` | Top-level ODS include |
| `MPIOps.td` | ODS op definitions |
| `MPITypes.td` | ODS type definitions (retval type, etc.) |
| `Utils.h` | MPI utility functions |

## For AI Agents

### Working In This Directory
- Edit `MPIOps.td` to add new MPI ops
- `Utils.h` provides helpers for building MPI call patterns

## Dependencies
- Depends on: MemRef types, MLIR IR core

<!-- MANUAL: -->
