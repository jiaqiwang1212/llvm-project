<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tensor IR

## Purpose
Core op definitions and interface implementations for the Tensor dialect.

## Key Files
| File | Description |
|------|-------------|
| `Tensor.h` | Op class declarations |
| `TensorBase.td` | Dialect definition and base classes |
| `TensorOps.td` | ODS op definitions |
| `TensorInferTypeOpInterfaceImpl.h` | InferType interface impls for tensor ops |
| `TensorTilingInterfaceImpl.h` | TilingInterface impls for tensor ops |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds impls for tensor ops |

## For AI Agents

### Working In This Directory
- Edit `TensorOps.td` to add new tensor ops
- `tensor.empty` is the preferred way to allocate result tensor storage in destination-passing style

## Dependencies
- Depends on: MLIR built-in tensor type, Arith/Index types

<!-- MANUAL: -->
