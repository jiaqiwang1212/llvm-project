<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/XCOFF

## Purpose

Implements object file transformation for XCOFF (Extended Common Object File Format) used on IBM AIX and Power Systems. Handles reading, modifying, and writing XCOFF headers, sections, symbols, and auxiliary symbol records.

## Key Files

| File | Description |
|------|-------------|
| `XCOFFObjcopy.cpp` | Main XCOFF transformation logic and command handling |
| `XCOFFObject.h` | XCOFF object class definition |
| `XCOFFReader.cpp` | XCOFF file parsing and loading |
| `XCOFFReader.h` | XCOFF reader interface |
| `XCOFFWriter.cpp` | XCOFF file serialization |
| `XCOFFWriter.h` | XCOFF writer interface |

## For AI Agents

### Working In This Directory

1. Understand XCOFF file structure specific to IBM AIX and Power Systems
2. Know XCOFF section types and auxiliary symbol records
3. Test with AIX binaries if available (rare in most development environments)
4. Verify section alignment and relocation updates
5. Handle XCOFF-specific symbol properties and visibility
6. Coordinate with IBM POWER architecture details

### Key Patterns

- XCOFF extends COFF with additional auxiliary symbol records
- Symbol table includes line number and source file information
- Relocation types specific to POWER ISA
- Section alignment rules differ from standard ELF/COFF
- Debug sections store symbol and line mapping

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing IBM AIX/XCOFF object files

<!-- MANUAL: -->
