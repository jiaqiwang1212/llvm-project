<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARF

## Purpose
DWARF debug information reader. Parses and provides access to DWARF debug data in ELF and other object files. Core infrastructure for Linux, BSD, and Unix debugging support. Used by debuggers, symbolizers, and analysis tools.

## Key Files
| File | Description |
|------|-------------|
| `DWARFContext.cpp` | Main DWARF context and parsing entry point |
| `DWARFUnit.cpp` | Compilation unit (CU) and type unit abstractions |
| `DWARFCompileUnit.cpp` | Compilation unit-specific parsing |
| `DWARFTypeUnit.cpp` | Type unit-specific parsing |
| `DWARFDie.cpp` | DIE (Debug Info Entry) manipulation and access |
| `DWARFDebugInfoEntry.cpp` | DIE parsing and structure |
| `DWARFAbbreviationDeclaration.cpp` | Abbreviation table parsing |
| `DWARFDebugAbbrev.cpp` | Abbreviation section (.debug_abbrev) handling |
| `DWARFFormValue.cpp` | Form value interpretation (attributes) |
| `DWARFExpression.cpp` | DWARF expression evaluation (location lists, etc.) |
| `DWARFLocationExpression.cpp` | Location expression parsing and evaluation |
| `DWARFExpressionPrinter.cpp` | Expression printing for debugging output |
| `DWARFDebugLine.cpp` | Line number table (.debug_line) parsing |
| `DWARFDebugLoc.cpp` | Location list (.debug_loc) handling |
| `DWARFDebugRangeList.cpp` | Range list (.debug_ranges) handling |
| `DWARFDebugRnglists.cpp` | New-format range lists (.debug_rnglists) handling |
| `DWARFListTable.cpp` | List table abstraction (ranges, locs) |
| `DWARFDebugFrame.cpp` | Frame info (.debug_frame, .eh_frame) parsing |
| `DWARFCFIPrinter.cpp` | CFI (Call Frame Info) pretty-printing |
| `DWARFDebugAranges.cpp` | Address range table (.debug_aranges) |
| `DWARFDebugArangeSet.cpp` | Single arange set parsing |
| `DWARFAddressRange.cpp` | Address range representation |
| `DWARFAcceleratorTable.cpp` | Name lookup accelerator table (.debug_names, .debug_pubnames) |
| `DWARFDebugPubTable.cpp` | Public symbol table (.debug_pubnames, .debug_pubtypes) |
| `DWARFDebugMacro.cpp` | Macro info (.debug_macro) parsing |
| `DWARFDebugAddr.cpp` | Address table (.debug_addr) handling |
| `DWARFGdbIndex.cpp` | GDB index section (.gdb_index) support |
| `DWARFUnitIndex.cpp` | Unit index (.debug_cu_index, .debug_tu_index) |
| `DWARFUnwindTablePrinter.cpp` | Unwind table printing for debugging |
| `DWARFVerifier.cpp` | DWARF format verification and validation |

## For AI Agents

### Working In This Directory
- **DWARFContext.cpp** is the main entry point; understand context creation before modifying.
- DIE traversal is fundamental; many tools rely on correct DIE parsing.
- Expression evaluation (DWARFExpression) is complex and error-prone; test thoroughly.
- New DWARF standards (DWARF 4, 5) add new forms and structures; stay current with spec.
- DWARFVerifier provides testing infrastructure; add tests for new parsing code.
- Location and range list handling has both old (.debug_loc, .debug_ranges) and new (.debug_loclists, .debug_rnglists) formats; support both.

### Dependencies

#### Internal
- Depends on `llvm/lib/Object/` (object file parsing)
- Depends on `llvm/lib/BinaryFormat/` (DWARF constants)
- Depends on `llvm/lib/Support/` (error handling, data structures)
- Used by `../Symbolize/`, debuggers, and profiling tools

<!-- MANUAL: -->
