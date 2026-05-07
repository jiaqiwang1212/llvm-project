<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy

## Purpose

Object file transformation infrastructure used by llvm-objcopy. Provides a uniform interface for reading, modifying, and writing object files across multiple formats (ELF, COFF, Mach-O, WebAssembly, XCOFF, DXContainer).

## Key Files

| File | Description |
|------|-------------|
| `CommonConfig.cpp` | Shared configuration for all object file formats |
| `ConfigManager.cpp` | Configuration management and parsing |
| `ObjCopy.cpp` | Main objcopy driver and command routing |
| `Archive.cpp` | Archive file handling (static libraries) |
| `Archive.h` | Archive file interface definition |

## Subdirectories

- **COFF/** - COFF/PE (Windows executable) transformation
- **DXContainer/** - DirectX container format handling
- **ELF/** - ELF (Unix/Linux executable) transformation
- **MachO/** - Mach-O (macOS/iOS executable) transformation
- **wasm/** - WebAssembly module transformation
- **XCOFF/** - IBM XCOFF format handling

## For AI Agents

### Working In This Directory

1. Understand the object file format being modified (consult subdirectory AGENTS.md)
2. Use format-specific subdirectories for implementation
3. Common transformations: strip sections, rename symbols, change permissions, rebase
4. Test with real binaries and verify output tool compatibility
5. Changes here affect all formats; test cross-platform

### Key Patterns

- Each format has Reader, Writer, and Object abstraction classes
- CommonConfig provides unified interface across formats
- Archive handling supports multi-object library files
- Transformations applied in consistent order across all formats

## Dependencies

### Internal
- Depends on: LLVM Support, BinaryFormat, MC (lib/MC)
- Used by: llvm-objcopy tool, build system utilities

<!-- MANUAL: -->
