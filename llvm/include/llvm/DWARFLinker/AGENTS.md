<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFLinker

## Purpose

Links DWARF debug information from multiple object files into a single debug output (or keeps separated). Removes duplicate type information, relocates debug data, and applies cross-compilation address mapping. Used by llvm-dsymutil (Darwin) and dsymutil (Mach-O/.dSYM bundles).

## Key Files

| File | Description |
|------|-------------|
| `DWARFLinkerBase.h` | Abstract linker interface; shared linking logic (5 KB) |
| `AddressesMap.h` | Address relocation mapping between input and output (8 KB) |
| `DWARFFile.h` | Input file wrapper; tracks compilation units and section offsets (1 KB) |
| `IndexedValuesMap.h` | Indexed value deduplication (types, strings); prevents duplicate entries (1 KB) |
| `StringPool.h` | String pool for debug strings; shared across linked output (3 KB) |
| `Utils.h` | Utility functions for DWARF traversal and manipulation (3 KB) |

## Subdirectories (if applicable)

| Directory | Purpose |
|-----------|---------|
| `Classic/` | Original single-threaded DWARF linker implementation; straightforward DIE copying |
| `Parallel/` | Multi-threaded linker; parallelizes CU processing for large binaries |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding DWARF linking strategy (DIE cloning, type deduplication)
- Analyzing address mapping algorithms
- Reviewing debug section relocation logic

**Implementation tasks:**
- Optimizing linking performance (caching, parallel CU processing)
- Implementing new address mapping schemes (e.g., for new architectures)
- Adding support for new DWARF extensions

### Common Patterns

1. **DIE cloning**: Copy input DIE tree to output, applying address relocations
2. **Type deduplication**: IndexedValuesMap identifies identical type definitions across CUs; reuses first occurrence
3. **Address relocation**: AddressesMap translates input PC ranges to output ranges (linker addresses)
4. **String pooling**: All debug strings go to shared StringPool; DIEs reference by offset
5. **CU remapping**: DWARFFile tracks input CUs; linker generates output CU headers

### Classic vs Parallel

- **Classic**: Single-threaded; simpler logic; suitable for small binaries
- **Parallel**: Multi-threaded CU processing; scales well for large binaries with many CUs
- Both use same DWARFLinkerBase interface

### Important Notes

- **dsymutil workflow**: Collects debug info from input .o files, links into separate .dSYM bundle (Darwin)
- **Duplicate elimination**: Type deduplication can reduce output size by 40-60% (common in C++ templates)
- **Address mapping critical**: Incorrect addresses break symbolization; validate all relocation mappings

## Dependencies

### Internal
- `llvm/DebugInfo/DWARF/` — DWARF parser (DWARFContext, DWARFUnit, DWARFDie)
- `llvm/Object/` — ObjectFile, ELFObjectFile for input reading
- `llvm/Support/` — Error, DataExtractor

### External
- None

<!-- MANUAL: -->
