<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ToolDrivers

## Purpose
Driver implementations for MSVC-compatible binary tools. Provides Windows-compatible alternatives to GNU binutils for developers using LLVM on Windows.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `llvm-dlltool/` | GNU dlltool replacement (see `llvm-dlltool/AGENTS.md`) |
| `llvm-lib/` | MSVC librarian replacement (see `llvm-lib/AGENTS.md`) |

## Key Files
| File | Description |
|------|-------------|
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Windows binary tool emulation
- MSVC-compatible command-line interfaces
- DLL import library generation
- Static/dynamic library management

## Dependencies

### Internal
- `llvm/lib/Object/` — object file handling
- `llvm/lib/BinaryFormat/` — format specifications

<!-- MANUAL: -->
