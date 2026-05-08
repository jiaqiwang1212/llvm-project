<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MemRef IR

## Purpose
Core op and interface definitions for the MemRef dialect: allocation, load/store, cast, subview, reshape, copy, and atomic ops on memref types.

## Key Files
| File | Description |
|------|-------------|
| `MemRef.h` | Op class declarations |
| `MemRefBase.td` | Dialect definition and base classes |
| `MemRefOps.td` | ODS op definitions |
| `MemoryAccessOpInterfaces.h` | Read/write memory access interface declarations |
| `MemoryAccessOpInterfaces.td` | ODS memory access interface definitions |
| `MemRefMemorySlot.h` | Memory slot interface implementations for memref ops |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds impls for memref ops |

## For AI Agents

### Working In This Directory
- Edit `MemRefOps.td` to add new memref ops
- `MemRefMemorySlot.h` connects memref to the mem2reg promotion framework

## Dependencies
- Depends on: MLIR built-in memref type, Index/Arith types

<!-- MANUAL: -->
