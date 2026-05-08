<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DLTI Dialect

## Purpose
Data Layout and Target Information (DLTI) dialect. Attaches target-specific data layout information (type sizes, alignments, endianness, alloca address space, etc.) to modules and functions as attributes.

## Key Files
| File | Description |
|------|-------------|
| `DLTI.h` | Dialect and attribute class declarations |
| `DLTI.td` | Top-level ODS include |
| `DLTIAttrs.td` | Attribute definitions (DataLayoutEntryAttr, DataLayoutSpecAttr, etc.) |
| `DLTIBase.td` | Dialect base definition |
| `Traits.h` | DLTI trait declarations |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `TransformOps/` | Transform dialect extension for DLTI queries (see `TransformOps/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- DLTI attributes are attached to ops via the `dlti.dl_spec` attribute
- Query data layout through `DataLayout` class from `mlir/Interfaces/DataLayoutInterfaces.h`

### Common Patterns
- `DataLayoutEntryAttr` maps a type key to a layout value (size, alignment)
- `DataLayoutSpecAttr` aggregates a list of entries for a whole module

## Dependencies
- Depends on: MLIR IR core (attributes, types)

<!-- MANUAL: -->
