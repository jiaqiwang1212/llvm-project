<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Object

## Purpose

Platform-agnostic object file reading and manipulation. Provides unified interfaces (ObjectFile, Binary, SymbolicFile) for accessing ELF, COFF, Mach-O, Wasm, XCOFF, and other binary formats. Central hub for tools like llvm-objdump, llvm-readelf, linkers, and debug info readers.

## Key Files

| File | Description |
|------|-------------|
| `ObjectFile.h` | Abstract ObjectFile interface; sections, symbols, relocations, string tables (22 KB) |
| `Binary.h` | Abstract Binary interface; owner of raw binary data; error handling (6 KB) |
| `SymbolicFile.h` | Higher-level interface for symbol-aware files; base for ObjectFile and Archive (6 KB) |
| `ELFObjectFile.h` | ELF object file reader; template over ELF types; lazy section/symbol parsing (49 KB) |
| `ELF.h` | ELF type system (sections, symbols, relocations, dynamic table) (55 KB) |
| `ELFTypes.h` | ELF type traits for 32-bit/64-bit endian variations (32 KB) |
| `COFF.h` | COFF object file reader and type definitions (50 KB) |
| `COFFImportFile.h` | COFF import library (.lib) reader (5 KB) |
| `COFFModuleDefinition.h` | COFF module definition file (.def) parser (1 KB) |
| `MachO.h` | Mach-O object file reader; load commands, sections, symbols, relocations (35 KB) |
| `MachOUniversal.h` | Mach-O universal binary (fat) reader (5 KB) |
| `MachOUniversalWriter.h` | Mach-O universal binary writer (3 KB) |
| `Wasm.h` | WebAssembly object file reader; sections, code, data, exports, imports (14 KB) |
| `DXContainer.h` | DirectX container (DXIL/DXBC) reader (22 KB) |
| `Minidump.h` | Windows minidump crash dump reader (15 KB) |
| `TapiFile.h` | TAPI (Mach-O abstraction) format reader (2 KB) |
| `TapiUniversal.h` | TAPI universal file reader (3 KB) |
| `Archive.h` | Archive (.a, .lib) reader; member enumeration (13 KB) |
| `ArchiveWriter.h` | Archive writer for building .a files (3 KB) |
| `IRObjectFile.h` | LLVM IR module exposed as object file (3 KB) |
| `IRSymtab.h` | Symbol table for IR; enables LTO symbol visibility (11 KB) |
| `OffloadBinary.h` | Offload binary container (GPU kernel bundles) (10 KB) |
| `OffloadBundle.h` | Bundle reader for multi-device offload binaries (7 KB) |
| `GOFF.h` | GOFF object format reader (z/OS mainframe) (9 KB) |
| `GOFFObjectFile.h` | GOFF object file interface (5 KB) |
| `XCOFFObjectFile.h` | XCOFF object file reader (AIX/Power) (34 KB) |
| `CVDebugRecord.h` | CodeView debug record parser for COFF (1 KB) |
| `Error.h` | Object-specific error codes (3 KB) |
| `Decompressor.h` | DWARF section decompressor (.zdebug format) (1 KB) |
| `FaultMapParser.h` | Linux fault map parser for exception handling (5 KB) |
| `RelocationResolver.h` | Cross-platform relocation resolver (1 KB) |
| `StackMapParser.h` | LLVM stackmap section parser (14 KB) |
| `SFrameParser.h` | Simple Frame format unwinder (4 KB) |
| `BBAddrMap.h` | Basic block address map for function granularity (11 KB) |
| `BuildID.h` | GNU Build ID extractor from ELF (1 KB) |
| `ModuleSymbolTable.h` | IR module symbol table abstraction (2 KB) |
| `SymbolSize.h` | Symbol size computation (0.8 KB) |
| `WindowsResource.h` | Windows resource (.res) reader (9 KB) |
| `WindowsMachineFlag.h` | Windows machine type flags (1 KB) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding object file structure (sections, symbols, relocations)
- Analyzing symbol visibility and linking requirements
- Reviewing cross-format compatibility

**Implementation tasks:**
- Adding support for new object file formats
- Enhancing symbol/relocation queries
- Implementing format-specific optimizations

### Common Patterns

1. **Lazy parsing**: ObjectFile subclasses defer parsing until data is accessed (compile-heavy for large binaries)
2. **Section iteration**: Use ObjectFile::sections() to enumerate; each section provides data, relocations, and metadata
3. **Symbol queries**: Use symbol tables for address->name and name->address lookups
4. **Relocation resolution**: RelocationResolver translates reloc types to memory patches
5. **Archive membership**: Archive::Child wraps each member; supports nested offload bundles

### Architecture Notes

- **ELFObjectFile**: Most feature-complete; supports dynamic linking, versioning, and symbol visibility
- **COFF**: Windows focus; includes import/export tables and debug directory
- **Mach-O**: Universal binary support; separate handling for arm64e pointer authentication
- **Wasm**: Imported/exported functions, data segments, custom sections
- **IRObjectFile**: Special case; exposes LLVM IR module as object file for symbol querying in LTO

## Dependencies

### Internal
- `llvm/BinaryFormat/` — Format constants and type definitions (ELF, COFF, MachO, Wasm, XCOFF)
- `llvm/Support/` — Error, Endianness, DataExtractor
- `llvm/IR/` (for IRObjectFile) — Module, GlobalValue

### External
- None (pure C++ standard library)

<!-- MANUAL: -->
