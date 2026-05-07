<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFLinker

## Purpose

DWARF debug information linker. Combines and optimizes debug information from multiple object files during linking. Supports both classic sequential implementation and modern parallel implementation for faster linking.

## Key Files

| File | Description |
|------|-------------|
| `DWARFLinkerBase.cpp` | Shared base functionality for DWARF linking |
| `Utils.cpp` | Utility functions for DWARF processing |

## Subdirectories

- **Classic/** - Sequential single-threaded DWARF linker (original implementation)
- **Parallel/** - Parallel multi-threaded DWARF linker (newer, faster for large binaries)

## For AI Agents

### Working In This Directory

1. This is an organizational directory. Do not add source files here.
2. Use Classic/ for sequential DWARF linking implementation
3. Use Parallel/ for multi-threaded DWARF linking
4. Understand DWARF format and debug information semantics
5. Read subdirectory AGENTS.md for implementation details

### Key Patterns

- DWARF linking deduplicates identical type information
- Optimization reduces final binary debug size by 30-70%
- Address relocation maps object-file addresses to linked addresses
- Parallel version uses work queuing and thread pools

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM DebugInfo, DWARF libraries
- Used by: linkers (lld, ld), dsymutil tool

<!-- MANUAL: -->
