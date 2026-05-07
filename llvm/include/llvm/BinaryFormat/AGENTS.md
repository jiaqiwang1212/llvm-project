<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# BinaryFormat

## Purpose

Read-only definitions and constants for binary file formats. Defines enum types, field offsets, and metadata structures for ELF, COFF, Mach-O, DWARF, Wasm, XCOFF, and other formats. Used by Object readers and debug info parsers; not responsible for serialization (see llvm/Object/ and llvm/Bitcode/ for writers).

## Key Files

| File | Description |
|------|-------------|
| `ELF.h` | ELF header, section types, program headers, relocations, symbol flags, dynamic tags (48 KB) |
| `COFF.h` | COFF header, section flags, symbol metadata, relocation types (30 KB) |
| `MachO.h` | Mach-O universal binary, load commands, segment types, relocation flags (68 KB) |
| `Dwarf.h` | DWARF debug format constants: tags (DW_TAG_*), attributes (DW_AT_*), forms (DW_FORM_*), opcodes (38 KB) |
| `Dwarf.def` | Macro-based DWARF constant definitions for code generation (58 KB) |
| `Wasm.h` | WebAssembly section types, opcodes, export/import kinds, relocation types (16 KB) |
| `XCOFF.h` | XCOFF object format (AIX/Power Systems) header, section types, symbols (21 KB) |
| `GOFF.h` | GOFF format (z/OS record format) for mainframe systems (8 KB) |
| `Magic.h` | Binary file magic number recognition (ELF, COFF, Mach-O, Wasm, archive) |
| `ELFRelocs/` | Subdirectory with per-architecture ELF relocation type definitions |
| `DXContainer.h` | DirectX container format types and constants (21 KB) |
| `DXContainerConstants.def` | DirectX part kinds and feature flags |
| `MachO.def` | Mach-O load command and flag definitions |
| `DynamicTags.def` | ELF dynamic tag definitions |
| `Minidump.h` | Windows minidump crash dump format structures (9 KB) |
| `MinidumpConstants.def` | Minidump stream types and exception codes |
| `MsgPack.h` | MessagePack serialization format constants (3 KB) |
| `MsgPack.def` | MessagePack type/extension definitions |
| `MsgPackDocument.h` | MessagePack document tree abstraction (16 KB) |
| `MsgPackReader.h` | MessagePack deserializer (4 KB) |
| `MsgPackWriter.h` | MessagePack serializer (4 KB) |
| `SFrame.h` | Simple Frame format for stack unwinding metadata (6 KB) |
| `SFrameConstants.def` | SFrame frame row entry and flag definitions |
| `Swift.h` | Swift metadata format constants (0.6 KB) |
| `Swift.def` | Swift-specific type/attribute definitions |
| `WasmRelocs.def` | WebAssembly relocation type definitions |
| `WasmTraits.h` | Wasm type traits for section parsing (3 KB) |
| `AMDGPUMetadataVerifier.h` | AMDGPU shader metadata validation (2 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Looking up binary format constant values (e.g., ELF section flags, COFF symbol storage classes)
- Understanding format specifications (relocation types, dynamic tag meanings, section purposes)
- Cross-referencing format versions and extensions

**Implementation tasks (rare, usually format version updates):**
- Adding new enum values for format extensions (e.g., new ELF flags in newer glibc versions)
- Updating constant definitions when formats evolve
- Adding new binary format definitions (e.g., future container formats)

### Common Patterns

1. **Enum-based definitions**: Use `.def` files for macro-expanded constants; allows code generation and documentation
2. **Bitfield flags**: Section/symbol/relocation flags use bit masks; query with `& MASK`
3. **Per-architecture relocation tables**: ELF relocations vary by target (x86-64 vs ARM vs MIPS); see `ELFRelocs/`
4. **Format-specific traits**: `WasmTraits.h` provides type info; similar patterns for other specialized formats

### Important Notes

- **No serialization**: BinaryFormat defines constants only. Object writers use these constants but live in llvm/Object/ or backend code.
- **Immutable definitions**: Changes here affect every tool that reads binaries; coordinate with OMC planning before modifying constants.
- **Cross-tool consistency**: Tools like llvm-objdump, llvm-readelf, and linkers all consume these definitions.

## Dependencies

### Internal
- `llvm/Support/` — Error handling, endian utilities

### External
- Depends on published format specifications (ELF ABI, COFF spec, Mach-O reference, DWARF spec, etc.)
- No external third-party libraries

<!-- MANUAL: -->
