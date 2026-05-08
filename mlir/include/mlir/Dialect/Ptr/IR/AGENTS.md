<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Ptr IR

## Purpose
Core op, type, attribute, enum, and interface definitions for the Ptr dialect.

## Key Files
| File | Description |
|------|-------------|
| `PtrDialect.h` | Dialect class declaration |
| `PtrDialect.td` | Dialect definition |
| `PtrOps.h` | Op class declarations |
| `PtrOps.td` | ODS op definitions |
| `PtrTypes.h` | Type class declarations |
| `PtrAttrDefs.td` | Attribute definitions (memory space attrs) |
| `PtrAttrs.h` | Generated attribute declarations |
| `PtrEnums.h` | Generated enum declarations |
| `PtrEnums.td` | Enum definitions |
| `MemorySpaceInterfaces.h` | Memory space interface declarations |
| `MemorySpaceInterfaces.td` | ODS memory space interface definitions |

## For AI Agents

### Working In This Directory
- `MemorySpaceInterfaces` allow backends to plug in custom address space semantics
- Edit `PtrOps.td` for new pointer ops; `PtrAttrDefs.td` for new memory space attributes

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
