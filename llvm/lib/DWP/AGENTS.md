<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWP

## Purpose

DWARF package file writer. Creates .dwp (DWARF Package) files that bundle split DWARF debug information (.dwo files) into a single indexed file for efficient debugging. Reduces lookup time and disk space during debugging.

## Key Files

| File | Description |
|------|-------------|
| `DWP.cpp` | Main DWP file generation and packaging |
| `DWARFError.cpp` | DWARF-specific error handling |

## For AI Agents

### Working In This Directory

1. Understand split DWARF (.dwo) file structure and split DWARF debugging
2. Know DWP package format and index structure
3. Test with clang -gsplit-dwarf generated binaries
4. Verify index enables fast DWE (DWARF Expression Evaluator) lookups
5. Test with debugging tools (lldb, gdb) that use .dwp files
6. Verify package correctness with dwarf validation tools
7. Test cross-object file references within DWP

### Key Patterns

- Split DWARF separates debug info (.dwo) from executable code
- DWP file contains concatenated .dwo data with index
- Index maps CU offset to file position for fast lookup
- Accelerator tables (names, types, line info) indexed
- CU ID uniquely identifies compilation unit across package

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM DebugInfo, DWARF utilities
- Used by: Build systems using split DWARF (clang -gsplit-dwarf), dsymutil

<!-- MANUAL: -->
