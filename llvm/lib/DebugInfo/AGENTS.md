<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DebugInfo

## Purpose
Debug information reading and processing infrastructure. Provides a unified interface for working with debug data in multiple formats: DWARF, CodeView (Microsoft), PDB, BTF, GSYM, and MSF containers. Used by debuggers, symbolization tools, and profilers.

## Subdirectories

- **DWARF/** — DWARF debug format reader (ELF, Linux, BSD)
- **CodeView/** — Microsoft CodeView debug format (PDB debug info)
- **PDB/** — Microsoft PDB file format (Windows)
- **BTF/** — Linux BPF Type Format (kernel debugging)
- **GSYM/** — Apple GSYM compact symbolication format
- **LogicalView/** — Abstract debug info view across formats
- **MSF/** — Multi-Stream File container format (used by PDB)
- **Symbolize/** — Symbol resolution for addresses across all formats

## For AI Agents

### Working In This Directory
- This directory is a grouping; root contains only CMakeLists.txt.
- Subdirectories are largely independent; each handles a specific debug format.
- Coordinate across formats when extending the symbolization or debug info APIs.
- LogicalView provides a common abstraction over all formats; leverage it for cross-format work.

### Dependencies

#### Internal
- Depends on `llvm/lib/IR/` (module information)
- Depends on `llvm/lib/Object/` (object file container formats)
- Depends on `llvm/lib/Support/` (error handling, utilities)
- Used by `llvm/tools/` (llvm-dwarfdump, llvm-symbolizer, etc.)

<!-- MANUAL: -->
