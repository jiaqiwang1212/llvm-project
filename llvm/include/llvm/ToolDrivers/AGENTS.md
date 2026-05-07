<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ToolDrivers

## Purpose
Driver implementations for llvm-lib and llvm-dlltool. Windows library and DLL generation tools that mirror MSVC lib.exe and dlltool behavior.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `llvm-lib/` | Windows static library (.lib) creation |
| `llvm-dlltool/` | DLL export list and stub generation |

## For AI Agents

### Working In This Directory
ToolDrivers provides Windows-specific compilation tools:
- **llvm-lib**: Create static libraries from object files
- **llvm-dlltool**: Generate import libraries and DLL stubs
- **MSVC compatibility**: Drop-in replacements for MS tools

### Common Patterns
- Parse command-line arguments compatible with MSVC tools
- Manage object file collections for linking
- Generate import libraries for DLL linking
- Error handling for Windows-specific issues

## Dependencies

### Internal
- `llvm/Object` (ObjectFile, Archive)
- `llvm/Support` (StringRef, Error)
- `llvm/Option` (command-line parsing)

<!-- MANUAL: -->
