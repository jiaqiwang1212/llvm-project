<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TextAPI

## Purpose
Apple TBD (Text-Based Description) file format support. Represents library interfaces in human-readable text for binary-only distributions and cross-platform builds.

## Key Files
| File | Description |
|------|-------------|
| `InterfaceFile.h` | Main interface file representation |
| `TextAPIReader.h` | Parse TBD files |
| `TextAPIWriter.h` | Generate TBD files |
| `Symbol.h` | Symbol definition |
| `SymbolSet.h` | Symbol collections |
| `Target.h` | Target triple information |
| `Record.h` | Record base class |
| `RecordsSlice.h` | Record slice grouping |
| `RecordVisitor.h` | Visitor pattern for records |
| `Architecture.h` | CPU architecture definitions |
| `ArchitectureSet.h` | Multiple architecture support |
| `Platform.h` | Platform identifiers |
| `FileTypes.h` | TBD format versions |
| `PackedVersion.h` | Version representation |
| `DylibReader.h` | Dynamic library parsing |
| `TextAPIError.h` | Error types |
| `Utils.h` | Utility functions |
| `Architecture.def` | Architecture definitions |

## For AI Agents

### Working In This Directory
TextAPI provides TBD format handling:
- **TBD format**: Text-based library interface specification
- **Symbol management**: Define exported symbols with properties
- **Multi-architecture**: Support multiple targets in single file
- **Version compatibility**: Track interface versions

### Common Patterns
- Read TBD files via TextAPIReader
- Write TBD files via TextAPIWriter
- Iterate symbols in InterfaceFile
- Define symbols with API information (availability, flags)

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- `llvm/ADT` (SmallVector, StringSet)

<!-- MANUAL: -->
