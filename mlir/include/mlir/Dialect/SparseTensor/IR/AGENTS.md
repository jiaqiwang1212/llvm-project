<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor IR

## Purpose
Core op, type, attribute, and interface definitions for the SparseTensor dialect. Includes the sparse encoding attribute, storage layout types, and iteration/manipulation ops.

## Key Files
| File | Description |
|------|-------------|
| `SparseTensor.h` | Op class declarations |
| `SparseTensorBase.td` | Dialect definition and base classes |
| `SparseTensorOps.td` | ODS op definitions |
| `SparseTensorAttrDefs.td` | Attribute definitions (SparseTensorEncodingAttr, etc.) |
| `SparseTensorInterfaces.h` | Interface declarations |
| `SparseTensorInterfaces.td` | ODS interface definitions |
| `SparseTensorTypes.td` | Type definitions |
| `SparseTensorType.h` | SparseTensorType wrapper class |
| `SparseTensorStorageLayout.h` | Storage layout computation helpers |
| `Enums.h` | Generated enum declarations (DimLevelType, etc.) |

## For AI Agents

### Working In This Directory
- `SparseTensorEncodingAttr` is the key attribute; it specifies dimension ordering and level types
- `DimLevelType` enum values: `dense`, `compressed`, `singleton`, etc.

## Dependencies
- Depends on: Tensor dialect types, MLIR built-in types

<!-- MANUAL: -->
