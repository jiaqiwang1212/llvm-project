<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MPI Dialect

## Purpose
Implements the MPI dialect — an MLIR abstraction over MPI (Message Passing Interface) communication primitives. Provides ops for point-to-point communication (`mpi.send`, `mpi.recv`), collective operations (`mpi.allreduce`), and MPI lifecycle (`mpi.init`, `mpi.finalize`).

## Key Files
| File | Description |
|------|-------------|
| `IR/MPI.cpp` | Dialect registration and type implementations |
| `IR/MPIOps.cpp` | Op implementations and verifiers |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | MPI ops, retval type, communicator type |

## For AI Agents

### Working In This Directory
- MPI ops model the MPI C API; they are lowered to actual MPI library calls by `MPIToLLVM` conversion.
- `mpi.retval` is a special type representing an MPI return code (MPI_SUCCESS, etc.).
- Operations carry communicator, rank, tag, and buffer operands matching the MPI API signature.
- No transforms in this dialect — transformation is entirely handled by `MPIToLLVM` and `ShardToMPI` conversions.
- Buffer arguments are `memref` types; shape and element type must be consistent with MPI datatype mapping.

### Common Patterns
- Verifiers check that buffer operand types can be mapped to MPI datatypes (integer, float types).
- `mpi.allreduce` verifies the op attribute matches a valid MPI_Op constant.

## Dependencies
- `mlir/Dialect/MemRef`, `mlir/Dialect/LLVMIR` (for lowering)

<!-- MANUAL: -->
