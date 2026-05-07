<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFLinker/Classic

## Purpose

Original sequential, single-threaded DWARF debug information linker. Combines debug information from multiple object files with deduplication and optimization for correctness and compatibility.

## Key Files

| File | Description |
|------|-------------|
| `DWARFLinker.cpp` | Main DWARF linking logic |
| `DWARFLinkerCompileUnit.cpp` | Compile unit processing and deduplication |
| `DWARFLinkerDeclContext.cpp` | Declaration context tracking |
| `DWARFStreamer.cpp` | DWARF section output streaming |

## For AI Agents

### Working In This Directory

1. Understand DWARF compilation unit and type hierarchy structure
2. Know DIE (Debug Information Entry) types and relationships
3. Study deduplication algorithms for DIE matching
4. Test with multi-object binaries and cross-module references
5. Verify debug information correctness in final binary
6. Test with dwarf validation tools (llvm-dwarfdump, eu-readelf)
7. Coordinate with address relocation during linking

### Key Patterns

- Compile units processed sequentially
- Type deduplication via signature matching
- References updated to point to deduplicated types
- Address relocation applied to all location expressions
- Output written to .debug_* sections in final binary

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM DebugInfo, LLVM MC
- Used by: Linkers as part of DWARF linking pass

<!-- MANUAL: -->
