<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/ELF

## Purpose

Implements object file transformation for ELF (Executable and Linkable Format) files used on Unix-like systems (Linux, BSD, etc.). Handles reading, modifying, and writing ELF headers, sections, symbols, relocations, and program headers.

## Key Files

| File | Description |
|------|-------------|
| `ELFObjcopy.cpp` | Main ELF transformation logic and command handling |
| `ELFObject.cpp` | ELF object representation and modification interface |
| `ELFObject.h` | ELF object class definition |

## For AI Agents

### Working In This Directory

1. Understand ELF file structure: e_header, program headers, section headers, symbol table, relocations
2. Know ELF section types (.text, .data, .bss, .rodata, .symtab, .strtab, .dynsym, .dynstr, etc.)
3. Test with real Unix binaries (executables, shared libraries, object files)
4. Verify section permissions, alignment, and relocation updates
5. Handle symbol visibility and dynamic linking requirements
6. Test stripping, section removal, and symbol name changes

### Key Patterns

- ELF reader/writer pair for symmetric I/O
- Section alignment constraints and load address mapping
- Symbol table linked to relocations and dynamic linking
- Program headers define runtime memory layout
- GNU extensions (.gnu_hash, .note.gnu.build-id, .gnu.version*) need special handling

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM Object, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing Unix/Linux object files

<!-- MANUAL: -->
