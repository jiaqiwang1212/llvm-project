<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ObjCopy

## Purpose

Object file transformation toolkit for copying, stripping, and modifying binaries. Implements the llvm-objcopy tool; supports section removal, renaming, addition, and format-specific mutations (ELF, COFF, Mach-O, Wasm, XCOFF). Enables debug info stripping, symbol modification, and cross-format conversions.

## Key Files

| File | Description |
|------|-------------|
| `CommonConfig.h` | Shared configuration for all formats: section commands, symbol rules, output format (8 KB) |
| `ConfigManager.h` | Config parsing and format auto-detection (1 KB) |
| `MultiFormatConfig.h` | Format-specific config selection (1 KB) |
| `ObjCopy.h` | Main objcopy entry point (1 KB) |

## Subdirectories (if applicable)

| Directory | Purpose |
|-----------|---------|
| `ELF/` | ELF-specific transformation logic; symbol stripping, section manipulation, relocation updates |
| `COFF/` | COFF-specific logic; export table updates, import library handling |
| `MachO/` | Mach-O-specific logic; load command patching, segment relocation, universal binary handling |
| `wasm/` | WebAssembly-specific logic; custom section addition, export/import modification |
| `XCOFF/` | XCOFF-specific logic (AIX) |

## For AI Agents

### Working In This Directory

**Read-only tasks:**
- Understanding objcopy command-line interface and configuration
- Reviewing section transformation strategies
- Analyzing symbol stripping logic

**Implementation tasks:**
- Adding new transformation options (section rename, symbol promotion, etc.)
- Implementing format-specific mutations
- Enhancing cross-format conversions

### Common Patterns

1. **Config-driven transformation**: CommonConfig captures user intent (strip debug, rename sections, add headers)
2. **Format dispatch**: ConfigManager selects format handler (ELF/COFF/MachO) based on input magic
3. **Section commands**: `add-section`, `remove-section`, `rename-section` apply in sequence
4. **Symbol modification**: Stripping, promotion, localization apply to symbol table
5. **Relocation rewriting**: Format-specific handlers patch relocations after section movement

### Important Notes

- **Destructive by design**: ObjCopy modifies binaries irreversibly; validates input before mutation
- **Format fidelity**: Preserves format-specific structures (ELF dynamic tables, COFF directories, Mach-O load commands)
- **Backed by tests**: Extensive test suite in llvm/tools/llvm-objcopy/; verify transformations don't break binary layout

## Dependencies

### Internal
- `llvm/Object/` — ObjectFile readers for input parsing
- `llvm/BinaryFormat/` — Format constants
- `llvm/Support/` — Error, Endianness, file I/O

### External
- None

<!-- MANUAL: -->
