<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TextAPI

## Purpose
Apple TBD (Text-Based Dylib) format parser and writer. Provides human-readable interface stubs for macOS/iOS libraries without requiring actual binary files.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `BinaryReader/` | Binary TBD format reading (see `BinaryReader/AGENTS.md`) |

## Key Files
| File | Description |
|------|-------------|
| `InterfaceFile.cpp` | Interface file representation |
| `TextStub.cpp` | Text stub format parser |
| `TextStubV5.cpp` | TBD v5 format support |
| `TextStubCommon.cpp` | Shared text stub utilities |
| `Symbol.cpp` | Symbol definition and properties |
| `SymbolSet.cpp` | Symbol collection management |
| `Target.cpp` | Target/platform specification |
| `Platform.cpp` | Platform detection and handling |
| `Architecture.cpp` | CPU architecture handling |
| `ArchitectureSet.cpp` | Multiple architecture support |
| `PackedVersion.cpp` | Version number handling |
| `RecordsSlice.cpp` | Record management |
| `RecordVisitor.cpp` | Record traversal |
| `TextAPIContext.h` | Context management |
| `TextAPIError.cpp` | Error handling |
| `Utils.cpp` | Utility functions |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- TBD format specification (v1-v5)
- Symbol export/import definition
- Platform and architecture specification
- Integration with Object/ for binary format

## Dependencies

### Internal
- `llvm/lib/Object/` — object file handling
- `llvm/lib/Support/` — utilities
- `llvm/lib/BinaryFormat/` — binary format specs

<!-- MANUAL: -->
