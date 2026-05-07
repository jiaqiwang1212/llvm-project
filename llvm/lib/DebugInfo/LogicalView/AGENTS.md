<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LogicalView

## Purpose
Abstract logical view over debug information across multiple formats (DWARF, CodeView, PDB, BTF, GSYM). Provides a unified interface for tools to interact with debug data regardless of the underlying format, enabling format-agnostic debugging and symbolization.

## Key Files
| File | Description |
|------|-------------|
| `LVReaderHandler.cpp` | Reader handler and format dispatcher |

## For AI Agents

### Working In This Directory
- LogicalView provides format abstraction; understand the interface before extending.
- This is a small directory with limited implementation; most work is in format-specific directories.
- Changes here should generalize across all supported formats.
- Coordinate with format-specific implementations (DWARF, CodeView, etc.) for integration.

### Dependencies

#### Internal
- Depends on `../DWARF/`, `../CodeView/`, `../PDB/`, `../BTF/`, `../GSYM/` (format readers)
- Depends on `llvm/lib/Support/` (error handling, utilities)
- Used by format-agnostic debugging and analysis tools

<!-- MANUAL: -->
