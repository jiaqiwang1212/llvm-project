<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CodeView

## Purpose
Microsoft CodeView debug format reader and writer. Handles PDB debug information parsing, symbol records, type records, and checksums. Core infrastructure for Windows debugging support.

## Key Files
| File | Description |
|------|-------------|
| `CodeViewRecordIO.cpp` | Record input/output and serialization |
| `RecordSerialization.cpp` | Serialization of CodeView records |
| `TypeRecordMapping.cpp` | Type record mapping and conversion |
| `SymbolRecordMapping.cpp` | Symbol record mapping and conversion |
| `CVTypeVisitor.cpp` | Type record visitor pattern implementation |
| `CVSymbolVisitor.cpp` | Symbol record visitor pattern implementation |
| `TypeTableCollection.cpp` | Collection and indexing of type records |
| `StringsAndChecksums.cpp` | String pool and checksum management |
| `TypeHashing.cpp` | Hash-based type lookup and deduplication |
| `TypeIndex.cpp` | Type index management |
| `TypeIndexDiscovery.cpp` | Type dependency discovery |
| `TypeStreamMerger.cpp` | Merging of type streams |
| `MergingTypeTableBuilder.cpp` | Type table with deduplication during building |
| `GlobalTypeTableBuilder.cpp` | Global type table construction |
| `AppendingTypeTableBuilder.cpp` | Appending-only type table builder |
| `LazyRandomTypeCollection.cpp` | Lazy-loaded type collection with random access |
| `SimpleTypeSerializer.cpp` | Built-in type serialization |
| `DebugSubsectionVisitor.cpp` | Subsection visitor pattern |
| `DebugSubsection.cpp` | Subsection base implementation |
| `DebugSubsectionRecord.cpp` | Subsection record management |
| `DebugChecksumsSubsection.cpp` | File checksum subsection handling |
| `DebugLinesSubsection.cpp` | Line number subsection handling |
| `DebugStringTableSubsection.cpp` | String table subsection handling |
| `DebugSymbolRVASubsection.cpp` | Symbol RVA subsection handling |
| `DebugSymbolsSubsection.cpp` | Symbol subsection handling |
| `DebugFrameDataSubsection.cpp` | Frame data subsection handling |
| `DebugInlineeLinesSubsection.cpp` | Inline function line info handling |
| `DebugCrossSubsection.cpp` | Cross-compilation unit references |
| `DebugCrossImpSubsection.cpp` | Cross-module import subsection |
| `SymbolDumper.cpp` | Symbol record dumping and printing |
| `SymbolRecordHelpers.cpp` | Helper functions for symbol records |
| `SymbolSerializer.cpp` | Symbol record serialization |
| `TypeDumpVisitor.cpp` | Type record dumping and printing |
| `TypeRecordHelpers.cpp` | Helper functions for type records |
| `RecordName.cpp` | Pretty-printing of record names |
| `Formatters.cpp` | Format-specific output helpers |
| `EnumTables.cpp` | Enumeration tables for CodeView values |
| `Line.cpp` | Line number information handling |
| `CodeViewError.cpp` | Error types and handling |
| `ContinuationRecordBuilder.cpp` | Multi-record continuation handling |

## For AI Agents

### Working In This Directory
- CodeView is complex; understand the visitor pattern before modifying.
- Type records and symbol records follow specific schemas; changes must respect format constraints.
- TypeTableCollection and merging logic are performance-critical; optimize carefully.
- Changes to serialization must maintain backward compatibility with existing PDB files.
- Coordinate with `../PDB/` for PDB-specific integration.

### Dependencies

#### Internal
- Depends on `llvm/lib/Support/` (error handling, binary I/O)
- Used by `../PDB/` (PDB file format)
- Used by Windows debugging tools and PDB readers

<!-- MANUAL: -->
