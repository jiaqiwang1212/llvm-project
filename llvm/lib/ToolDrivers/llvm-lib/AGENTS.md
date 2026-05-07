<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ToolDrivers/llvm-lib

## Purpose
LLVM implementation of MSVC librarian (lib.exe). Creates and manages static libraries (.lib files) with MSVC-compatible command-line interface.

## Key Files
| File | Description |
|------|-------------|
| (librarian implementation) | Static library management tool |

## For AI Agents

### Working In This Directory
- MSVC lib.exe command-line compatibility
- Archive creation and manipulation
- Symbol table management
- COFF object file handling

## Dependencies

### Internal
- `llvm/lib/Object/` — archive and object file handling
- `llvm/lib/BinaryFormat/` — archive format

<!-- MANUAL: -->
