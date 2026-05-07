<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/COFF

## Purpose

Implements object file transformation for COFF (Common Object File Format) and PE (Portable Executable) files used on Windows. Handles reading, modifying, and writing COFF/PE headers, sections, symbols, and relocation information.

## Key Files

| File | Description |
|------|-------------|
| `COFFObjcopy.cpp` | Main COFF transformation logic and command handling |
| `COFFObject.cpp` | COFF object representation and modification interface |
| `COFFObject.h` | COFF object class definition |
| `COFFReader.cpp` | COFF file parsing and loading |
| `COFFReader.h` | COFF reader class interface |
| `COFFWriter.cpp` | COFF file serialization and writing |
| `COFFWriter.h` | COFF writer class interface |

## For AI Agents

### Working In This Directory

1. Understand COFF/PE file structure: headers, sections, symbols, relocations
2. Know Windows executable format specifics (DOS header, PE header, data directories)
3. Test with real Windows binaries (.exe, .dll, .obj files)
4. Verify section permissions, alignment, and relocation updates
5. Handle symbol table modifications and name mangling
6. Test on Windows platform or use cross-compilation tools

### Key Patterns

- COFF reader/writer pair for symmetric I/O
- Sections identified by name (.text, .data, .reloc, etc.)
- Symbol table linked to relocations
- PE data directories must track modified section addresses
- Export tables, import tables, and resource sections need special handling

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing Windows object files

<!-- MANUAL: -->
