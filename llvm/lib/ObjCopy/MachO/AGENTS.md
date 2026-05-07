<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy/MachO

## Purpose

Implements object file transformation for Mach-O (Mach Object) files used on macOS and iOS. Handles reading, modifying, and writing Mach-O headers, load commands, sections, symbols, and linked data structures.

## Key Files

| File | Description |
|------|-------------|
| `MachOLayoutBuilder.cpp` | Mach-O memory layout calculation and optimization |
| `MachOLayoutBuilder.h` | Layout builder interface definition |
| `MachOObjcopy.cpp` | Main Mach-O transformation logic and command handling |
| `MachOObject.cpp` | Mach-O object representation and modification |
| `MachOObject.h` | Mach-O object class interface |
| `MachOReader.cpp` | Mach-O file parsing and loading |
| `MachOReader.h` | Mach-O reader class interface |
| `MachOWriter.cpp` | Mach-O file serialization and writing |
| `MachOWriter.h` | Mach-O writer class interface |

## For AI Agents

### Working In This Directory

1. Understand Mach-O file structure: mach_header, load commands, segments, sections
2. Know macOS-specific features: code signing, fat binaries (universal), dsymutil integration
3. Test with real macOS binaries (.o, dylib, executable files)
4. Verify segment alignment, load command offsets, and relocation updates
5. Handle code signing attributes and entitlements
6. Test on macOS or use cross-compilation tools
7. Coordinate with MachOLayoutBuilder for complex modifications

### Key Patterns

- Mach-O uses load commands for flexible header structure
- Segments map to virtual memory regions; sections group related code/data
- Symbol resolution linked to indirect symbol stubs and lazy bindings
- Code signing seals the binary; modifications invalidate it (resign required)
- Fat binaries can contain multiple architectures with independent layouts

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM Object, LLVM BinaryFormat, ObjCopy common infrastructure
- Used by: llvm-objcopy when processing macOS/iOS object files

<!-- MANUAL: -->
