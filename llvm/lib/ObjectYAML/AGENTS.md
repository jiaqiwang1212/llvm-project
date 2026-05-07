<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjectYAML

## Purpose
Bidirectional conversion between object files and YAML representation. Enables human-readable object file inspection and creation for testing purposes.

## Key Files
| File | Description |
|------|-------------|
| `ELFYAML.cpp` / `ELFEmitter.cpp` | ELF ↔ YAML conversion |
| `COFFYAML.cpp` / `COFFEmitter.cpp` | COFF ↔ YAML conversion |
| `MachOYAML.cpp` / `MachOEmitter.cpp` | Mach-O ↔ YAML conversion |
| `GOFFYAML.cpp` / `GOFFEmitter.cpp` | GOFF ↔ YAML conversion |
| `ArchiveYAML.cpp` / `ArchiveEmitter.cpp` | Archive ↔ YAML conversion |
| `DWARFYAML.cpp` / `DWARFEmitter.cpp` | DWARF debug info ↔ YAML |
| `CodeViewYAMLDebugSections.cpp` | CodeView debug sections |
| `CodeViewYAMLSymbols.cpp` | CodeView symbols |
| `CodeViewYAMLTypes.cpp` | CodeView type records |
| `CodeViewYAMLTypeHashing.cpp` | CodeView type hashing |
| `DXContainerYAML.cpp` / `DXContainerEmitter.cpp` | DirectX container format |
| `MinidumpEmitter.cpp` | Minidump format output |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Object file YAML schema definition
- Round-trip conversion preservation
- Testing binary format handling without hex editors
- Support for debugging information and metadata

## Dependencies

### Internal
- `llvm/lib/Object/` — object file reading
- `llvm/lib/Support/` — YAML library, utilities

<!-- MANUAL: -->
