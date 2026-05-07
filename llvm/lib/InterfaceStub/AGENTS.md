<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# InterfaceStub

## Purpose
Generates and reads ELF interface stub (.ifs) files that describe object file interfaces without full implementation. Used for binary compatibility analysis and minimal binary distribution.

## Key Files
| File | Description |
|------|-------------|
| `ELFObjHandler.cpp` | ELF object file handling |
| `IFSHandler.cpp` | Interface stub format handler |
| `IFSStub.cpp` | Interface stub data structure |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Understand .ifs format for interface definition
- Symbol table extraction and stub generation
- Version information and ABI compatibility tracking
- Integration with Object/ for ELF handling

## Dependencies

### Internal
- `llvm/lib/Object/` — object file reading/writing
- `llvm/lib/BinaryFormat/` — binary format handling

<!-- MANUAL: -->
