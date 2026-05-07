<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# GSYM

## Purpose
Apple GSYM (Global Symbol) compact symbolication format. Provides lightweight debug symbol information optimized for fast address-to-symbol lookup and compact storage. Used in Apple tools and alternative to full DWARF for embedded/size-constrained scenarios.

## Key Files
| File | Description |
|------|-------------|
| `GsymCreator.cpp` | Main GSYM file creator and builder |
| `GsymCreatorV1.cpp` | GSYM v1 format creation |
| `GsymCreatorV2.cpp` | GSYM v2 format creation |
| `GsymReader.cpp` | GSYM file reader |
| `GsymReaderV1.cpp` | GSYM v1 format reading |
| `GsymReaderV2.cpp` | GSYM v2 format reading |
| `GsymContext.cpp` | GSYM context and lookup management |
| `FunctionInfo.cpp` | Per-function symbol information |
| `InlineInfo.cpp` | Inline function information |
| `CallSiteInfo.cpp` | Call site information and inlining |
| `LineTable.cpp` | Line number table |
| `GlobalData.cpp` | Global symbol data |
| `LookupResult.cpp` | Address lookup result representation |
| `MergedFunctionsInfo.cpp` | Handling for merged/identical functions |
| `Header.cpp` | GSYM file header parsing |
| `HeaderV2.cpp` | GSYM v2 header handling |
| `FileWriter.cpp` | GSYM file writing utilities |
| `DwarfTransformer.cpp` | Conversion from DWARF to GSYM |
| `ExtractRanges.cpp` | Address range extraction utilities |
| `ObjectFileTransformer.cpp` | Transformation of object files to GSYM |

## For AI Agents

### Working In This Directory
- GSYM is optimized for fast lookup; changes should maintain O(log n) lookup performance.
- DwarfTransformer converts DWARF to GSYM; coordinate with DWARF directory for compatibility.
- V1 and V2 formats coexist; maintain backward compatibility when extending.
- GsymCreator and GsymReader are the main APIs; understand the format before modifying.
- Inline function tracking is complex; test thoroughly when changing inlining logic.

### Dependencies

#### Internal
- Depends on `../DWARF/` (DWARF source for transformation)
- Depends on `llvm/lib/Object/` (object file format)
- Depends on `llvm/lib/Support/` (error handling, utilities)
- Used by Apple debugging tools and symbolizers

<!-- MANUAL: -->
