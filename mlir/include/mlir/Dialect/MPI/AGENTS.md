<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MPI Dialect

## Purpose
Represents Message Passing Interface (MPI) operations for distributed memory parallel programming: send, receive, broadcast, reduce, and other MPI collective/point-to-point operations.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type definitions and utilities (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Op names follow `mpi.*` convention
- Lowered to MPI library calls via MPIToLLVM conversion

### Common Patterns
- `mpi.send` / `mpi.recv` for point-to-point; `mpi.allreduce` for collectives
- Return retcode values that can be checked or ignored

## Dependencies
- Depends on: MemRef dialect (for buffer arguments), LLVMIR dialect (for lowering)

<!-- MANUAL: -->
