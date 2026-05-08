<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linalg IR

## Purpose
Core op, interface, and structured-op definitions for the Linalg dialect. Includes `linalg.generic`, named ops, relayout ops, and the `LinalgOp` / `TilingInterface` implementations.

## Key Files
| File | Description |
|------|-------------|
| `Linalg.h` | Op class declarations |
| `LinalgBase.td` | Dialect definition and base classes |
| `LinalgOps.td` | Generic and utility op definitions |
| `LinalgStructuredOps.td` | Structured (named) op definitions via ODS |
| `LinalgRelayoutOps.td` | Relayout op definitions (pack, unpack, transpose) |
| `LinalgInterfaces.h` | Interface declarations (`LinalgOp`, `TilingInterface`, etc.) |
| `LinalgInterfaces.td` | ODS interface definitions |
| `LinalgEnums.td` | Enum definitions |
| `LinalgDoc.td` | Documentation TD for named ops |
| `LinalgNamedStructuredOps.yaml` | YAML definitions for named structured ops (generated) |
| `RelayoutOpInterface.h` | Relayout interface declarations |
| `RelayoutOpInterface.td` | ODS relayout interface definitions |
| `ValueBoundsOpInterfaceImpl.h` | ValueBounds interface impls for linalg ops |

## For AI Agents

### Working In This Directory
- Named ops come from `LinalgNamedStructuredOps.yaml`; edit YAML to change named op definitions
- `LinalgInterfaces.td` is the primary source for `LinalgOp`, `ContractionOpInterface`, etc.

## Dependencies
- Depends on: Tensor and MemRef types, Affine maps, Vector types

<!-- MANUAL: -->
