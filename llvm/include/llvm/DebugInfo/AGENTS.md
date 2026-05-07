<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DebugInfo

## Purpose

Unified debug information handling for multiple format backends. Provides platform-agnostic interfaces to read, parse, and manipulate debug data from ELF (DWARF), Windows (CodeView/PDB), eBPF (BTF), and other binary formats. Core hub for symbol resolution, source-level debugging, and debug format conversion.

## Key Files

| File | Description |
|------|-------------|
| `DIContext.h` | Abstract debug context interface for format-agnostic debug info access |

## Subdirectories (if applicable)

| Directory | Purpose |
|-----------|---------|
| `DWARF/` | DWARF debug format support (primary for ELF/Mach-O); abbrev, accelerator tables, compile units, line info, location, ranges, string tables |
| `CodeView/` | Microsoft CodeView debug format (Windows/COFF); type/symbol records, checksums, registers, inline lines, cross-module references |
| `PDB/` | Program Database format (Windows debug symbols); native and DIA (COM) implementations; raw symbol access |
| `BTF/` | Berkeley Packet Filter type information; eBPF debug metadata, parser, context |
| `GSYM/` | Apple's compact symbolication format; creator/reader, function info, dwarf transformer, ranges |
| `MSF/` | Microsoft Stream File container (used by PDB); block stream, builder, common structures |
| `LogicalView/` | High-level debug symbol visualization; readers, core symbol trees, handler |
| `Symbolize/` | Source location and symbol resolution; symbolize interface, markup for human-readable output |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding debug format structures (DWARF abbreviations, CodeView record types, PDB streams)
- Analyzing debug info consumption patterns
- Reviewing format-agnostic DIContext interfaces

**Implementation tasks:**
- Adding format-specific parsers or builders (e.g., new DWARF section type)
- Enhancing debug info querying APIs
- Implementing cross-format debug data transformations

### Common Patterns

1. **Format-agnostic access**: Implement via DIContext abstraction; subclasses for each format (DWARFContext, PDBContext, etc.)
2. **Lazy parsing**: Many DWARF sections are parsed on-demand (compilation units, location lists, ranges)
3. **Data extraction**: Use DataExtractor for endian-aware binary parsing (DWARF uses LEB128 variable-length integers)
4. **Visitor pattern**: CodeView and PDB use visitor-based traversal (CVSymbolVisitor, CVTypeVisitor)
5. **Symbol tables**: BTF and GSYM maintain compact symbol/type pools for efficient serialization

### Implementation Notes

- **DWARF** is the primary debug format for open-source toolchains (GCC, Clang on ELF/Mach-O)
- **CodeView/PDB** are Windows-specific; required for COFF object files and MSVC compatibility
- **BTF** is kernel-space debug metadata; minimal but sufficient for eBPF programs
- **GSYM** compresses DWARF debug info for Apple frameworks (binary size reduction with symbol lookups intact)
- **Symbolize** depends on all format-specific implementations; used by tools like llvm-symbolizer

## Dependencies

### Internal
- `llvm/BinaryFormat/` — DWARF constants (DW_TAG_*, DW_AT_*, DW_FORM_*), COFF/ELF/MachO type definitions
- `llvm/Object/` — Object file reading (ELFObjectFile, COFFObjectFile, MachOObjectFile); Binary and SymbolicFile interfaces
- `llvm/Support/` — DataExtractor, Error, Endianness

### External
- Platform-specific debug readers may require vendor SDKs (e.g., Windows SDK for PDB DIA)

<!-- MANUAL: -->
