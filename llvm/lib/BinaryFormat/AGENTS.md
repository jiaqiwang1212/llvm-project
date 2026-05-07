<!-- Generated: 2026-05-07 -->

# BinaryFormat — Binary Format Constants and Utilities

## Purpose

Provides constants, enumerations, and utilities for various binary file formats supported by LLVM: ELF (Unix/Linux), COFF (Windows), Mach-O (macOS), WebAssembly, XCOFF (AIX), DWARF (debugging), and MsgPack. Does not include full format parsing or generation—only format constants and metadata. Actual parsing/generation lives in specific backend or tool code.

## Key Files

| File | Description |
|------|-------------|
| `ELF.cpp` | ELF (Executable and Linkable Format) constants and helpers |
| `COFF.cpp` | COFF (Common Object File Format) constants and helpers |
| `MachO.cpp` | Mach-O (macOS) format constants and helpers |
| `Wasm.cpp` | WebAssembly binary format constants |
| `XCOFF.cpp` | XCOFF (AIX) format constants and helpers |
| `Dwarf.cpp` | DWARF debug info format constants and utilities |
| `Magic.cpp` | File type detection (magic numbers) |
| `MsgPackDocument.cpp` | MsgPack document structure and manipulation |
| `MsgPackDocumentYAML.cpp` | YAML serialization for MsgPack documents |
| `MsgPackReader.cpp` | MsgPack binary format reading |
| `MsgPackWriter.cpp` | MsgPack binary format writing |
| `SFrame.cpp` | SFrame (stack frame) format utilities |
| `DXContainer.cpp` | DirectX container format utilities |
| `AMDGPUMetadataVerifier.cpp` | AMDGPU-specific metadata verification |

## Subdirectories

None. This is a leaf directory under `llvm/lib/`.

## For AI Agents

### Working In This Directory

1. **Constants only**: This directory defines constants and enumerations. Do not implement file I/O or parsing here—use separate tools for that.

2. **Format versions**: Different versions of formats (ELF, DWARF, MsgPack) have different layouts. Maintain version-aware constants.

3. **Endianness**: Binary formats may be big-endian or little-endian. Use utilities like `llvm::support::endian` for safe conversions.

4. **Cross-platform**: Constants must be consistent across all platforms. Test on multiple architectures.

5. **Backward compatibility**: Formats evolve. Support multiple versions if necessary.

6. **Documentation**: Each format constant should map back to an official specification. Document which spec revision each constant matches.

### Common Patterns

**Adding ELF constants:**
```cpp
// In ELF.cpp
namespace ELF {
  enum e_type {
    ET_NONE = 0,
    ET_REL = 1,
    ET_EXEC = 2,
    // ...
  };
}
```

**Magic number detection:**
```cpp
// In Magic.cpp
file_magic identify_magic(StringRef Magic) {
  if (Magic.startswith("\x7fELF"))
    return file_magic::elf;
  if (Magic.startswith("\xfe\xed\xfa"))
    return file_magic::mach_o;
  // ...
}
```

**DWARF tag constants:**
```cpp
// In Dwarf.cpp
enum Tag : uint16_t {
  DW_TAG_array_type = 0x04,
  DW_TAG_structure_type = 0x13,
  // ...
};
```

## Dependencies

### Internal

- **llvm/lib/Support/** — String, endian, and data utilities
- **llvm/include/llvm/BinaryFormat/** — Public BinaryFormat headers

### External

- Format specifications (ELF, COFF, Mach-O, DWARF, etc.)

### Dependents

- **llvm/lib/Object/** — Uses BinaryFormat constants for object file handling
- **llvm/lib/DebugInfo/** — Uses DWARF constants
- **llvm/lib/CodeGen/** — Uses format constants when generating object files
- **llvm/tools/llvm-objdump**, **llvm-readelf**, etc. — Inspection tools

## Notes for Developers

- **Official specifications**: Always consult official format specifications (ELF-spec.pdf, DWARF specs, etc.) when adding constants.
- **Test against real binaries**: Ensure constants match actual binaries in the wild. Use objdump, readelf, or similar tools.
- **Avoid circular dependencies**: BinaryFormat should not depend on Object or CodeGen (those depend on BinaryFormat).
- **Version tracking**: When formats evolve, update comments with the version that introduced each constant.
- **Enum naming**: Use consistent naming conventions (e.g., `DW_TAG_*` for DWARF, `ET_*` for ELF e_type).
- **Safety**: Use `cast_or_null<>` when reinterpreting raw bytes to avoid undefined behavior.

<!-- MANUAL: -->
