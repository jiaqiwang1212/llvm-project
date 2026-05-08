<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine IR

## Purpose
Core op, interface, and value-map definitions for the Affine dialect. Includes affine.for, affine.if, affine.load, affine.store and related ops, plus memory-op interfaces.

## Key Files
| File | Description |
|------|-------------|
| `AffineOps.h` | Op class declarations (generated from ODS) |
| `AffineOps.td` | ODS op definitions for all affine ops |
| `AffineMemoryOpInterfaces.h` | Read/write memory-op interface declarations |
| `AffineMemoryOpInterfaces.td` | ODS interface definitions |
| `AffineValueMap.h` | AffineValueMap: an affine map bound to SSA values |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds interface implementations for affine ops |

## For AI Agents

### Working In This Directory
- Edit `AffineOps.td` to add or modify ops; the `.h.inc`/`.cpp.inc` files are generated
- `AffineValueMap` wraps AffineMap + SSA operands for concrete bound queries

### Common Patterns
- Op names follow `affine.for`, `affine.load`, etc.
- Memory ops implement `AffineReadOpInterface` / `AffineWriteOpInterface`

## Dependencies
- Depends on: MLIRIr (AffineMap, IntegerSet), MemRef types

<!-- MANUAL: -->
