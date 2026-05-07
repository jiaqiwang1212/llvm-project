<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWP

## Purpose

DWARF Package file (.dwp) writer for distributing debug information. Collects DWARF debug sections from split-DWARF object files (.dwo) into a single shareable package. Enables efficient binary distribution without embedding full debug info in executables.

## Key Files

| File | Description |
|------|-------------|
| `DWP.h` | Main DWP writer; collects .dwo sections, builds package index (4 KB) |
| `DWPError.h` | DWP-specific error codes (0.5 KB) |
| `DWPStringPool.h` | String deduplication pool for DWP output (1 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding split-DWARF and .dwp format
- Analyzing package indexing strategy
- Reviewing section collection logic

**Implementation tasks:**
- Optimizing DWP file layout
- Implementing new index formats
- Enhancing string deduplication

### Common Patterns

1. **Split-DWARF workflow**: Compile with `-gsplit-dwarf`; linker collects .dwo files into .dwp
2. **Section indexing**: .dwp contains index of debug sections by CU offset; enables fast lookup
3. **String pooling**: DWPStringPool deduplicates strings across CUs; reduces output size
4. **Offset mapping**: Track input .dwo offsets -> .dwp offsets for relocation
5. **Package structure**: Fixed header, multiple debug sections, index table, string pool

### Split-DWARF Benefits

- **Smaller executables**: Debug info in separate .dwo files; not linked into binary
- **Parallel compilation**: Multiple .dwo files built independently; linker gathers later
- **Efficient distribution**: .dwp packages can be distributed separately (CDN, symbol servers)

### Important Notes

- **DWARF 4+ feature**: Split-DWARF introduced in DWARF 4; compiler must support
- **Linker integration**: Linker detects .dwo files, calls DWP writer to create package
- **Backward compatibility**: Tools must handle both embedded and split-DWARF debug info

## Dependencies

### Internal
- `llvm/DebugInfo/DWARF/` — DWARFContext, DWARFUnit for debug section reading
- `llvm/Object/` — ObjectFile for .dwo input files
- `llvm/Support/` — Error, DataExtractor

### External
- None

<!-- MANUAL: -->
