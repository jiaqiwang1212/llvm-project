<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DLTI Dialect

## Purpose
Implements the Data Layout and Target Info (DLTI) dialect. Provides `dlti.dl_spec` attributes and interfaces for attaching data layout information (type sizes, alignments, endianness) and target device information to modules and ops.

## Key Files
| File | Description |
|------|-------------|
| `DLTI.cpp` | Dialect registration, `DataLayoutSpecAttr`, query interface implementations |
| `Traits.cpp` | Op traits related to data layout (e.g., `HasDataLayout`) |
| `TransformOps/` | Transform dialect extension for DLTI queries |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `TransformOps/` | Transform dialect ops for querying DLTI information |

## For AI Agents

### Working In This Directory
- DLTI is metadata-only: no executable ops, only attributes and interfaces.
- `DataLayoutSpecAttr` stores a list of `DataLayoutEntryAttr` pairs mapping types to size/alignment/ABI info.
- Consumers use `DataLayout::getTypeSize()` / `getTypeABIAlignment()` which query the DLTI spec attached to the enclosing module.
- Target info (`dlti.target_system_spec`) stores device properties for heterogeneous compilation.
- `TransformOps/` lets Transform scripts query DLTI info at compile time.

### Common Patterns
- Attach a `dlti.dl_spec` attribute to a `ModuleOp` to enable data layout queries.
- Use `DataLayout` class (from `mlir/Interfaces/DataLayoutInterfaces.h`) to query, not direct attribute access.

## Dependencies
- `mlir/IR`, `mlir/Interfaces/DataLayoutInterfaces`

<!-- MANUAL: -->
