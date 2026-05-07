<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Symbolize

## Purpose
Symbol resolution for addresses across all debug formats. Converts program addresses to symbol names, source files, and line numbers. Core infrastructure for symbolizers, crash handlers, and profiling tools.

## Key Files
| File | Description |
|------|-------------|
| `Symbolize.cpp` | Main symbolization API and address-to-symbol lookup |
| `SymbolizableObjectFile.cpp` | Symbolization interface for object files |
| `DIPrinter.cpp` | Debug info printing and formatting |
| `Markup.cpp` | Markup output formatting for symbolization results |
| `MarkupFilter.cpp` | Markup filtering and processing |

## For AI Agents

### Working In This Directory
- **Symbolize.cpp** is the main API; understand the lookup interface before modifying.
- SymbolizableObjectFile abstracts over multiple debug formats; changes should work across all formats.
- DIPrinter handles output formatting; coordinate with tool requirements.
- Markup support provides structured output for log analysis; maintain backward compatibility.
- Performance is critical; symbol lookups happen in hot paths.

### Dependencies

#### Internal
- Depends on `../DWARF/`, `../CodeView/`, `../PDB/`, `../BTF/`, `../GSYM/` (format-specific lookups)
- Depends on `llvm/lib/Object/` (object file format)
- Depends on `llvm/lib/Support/` (error handling, utilities)
- Used by `llvm/tools/llvm-symbolizer` and external crash handlers

<!-- MANUAL: -->
