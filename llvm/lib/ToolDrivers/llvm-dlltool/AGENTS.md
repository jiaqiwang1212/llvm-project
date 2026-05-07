<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ToolDrivers/llvm-dlltool

## Purpose
LLVM implementation of GNU dlltool. Generates Windows DLL import libraries (.lib files) from definition files (.def files) for creating import stubs.

## Key Files
| File | Description |
|------|-------------|
| (dlltool implementation) | DLL import library generator |

## For AI Agents

### Working In This Directory
- .def file parsing (EXPORTS section)
- Import library (.lib) generation
- COFF object file creation
- Symbol export list processing

## Dependencies

### Internal
- `llvm/lib/Object/` — object file writing
- `llvm/lib/BinaryFormat/` — COFF format

<!-- MANUAL: -->
