<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# WindowsDriver

## Purpose
Windows-specific driver support utilities. Provides helper functions for compiler drivers running on Windows platforms, including MSVC path detection.

## Key Files
| File | Description |
|------|-------------|
| `MSVCPaths.cpp` | MSVC compiler and SDK path detection |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- MSVC installation detection
- Visual Studio version handling
- Windows SDK location discovery
- Registry-based path resolution

## Dependencies

### Internal
- `llvm/lib/Support/` — utilities, error handling

<!-- MANUAL: -->
