<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# DWARFLinker/Parallel

## Purpose

Parallel multi-threaded DWARF debug information linker. Accelerates DWARF linking for large binaries with multiple compile units by processing units concurrently. Maintains correctness while improving performance.

## Key Files

| File | Description |
|------|-------------|
| `AcceleratorRecordsSaver.cpp` | Saves accelerator table records (names, types) |
| `AcceleratorRecordsSaver.h` | Accelerator record interface |
| `ArrayList.h` | Growable array data structure |
| `DebugLineSectionEmitter.h` | Debug line section generation |
| `DependencyTracker.cpp` | Tracks dependencies between compile units |
| `DependencyTracker.h` | Dependency tracking interface |
| `DIEAttributeCloner.cpp` | Clones DIE attributes with relocation |
| `DIEAttributeCloner.h` | Attribute cloning interface |
| `DIEGenerator.h` | DIE generation interface |
| `DWARFEmitterImpl.cpp` | DWARF section emission |
| `DWARFEmitterImpl.h` | Emitter implementation |
| `DWARFLinker.cpp` | Parallel DWARF linker main logic |
| `DWARFLinkerCompileUnit.cpp` | Parallel compile unit processing |
| `DWARFLinkerCompileUnit.h` | Compile unit interface |
| `DWARFLinkerGlobalData.h` | Global shared data |
| `DWARFLinkerImpl.cpp` | Linker implementation |
| `DWARFLinkerImpl.h` | Linker interface |
| `DWARFLinkerTypeUnit.cpp` | Type unit processing |
| `DWARFLinkerTypeUnit.h` | Type unit interface |
| `DWARFLinkerUnit.cpp` | Generic unit processing |
| `DWARFLinkerUnit.h` | Unit interface |
| `OutputSections.cpp` | Output DWARF sections management |
| `OutputSections.h` | Output sections interface |
| `StringEntryToDwarfStringPoolEntryMap.h` | String deduplication map |
| `SyntheticTypeNameBuilder.cpp` | Builds synthetic type names |
| `SyntheticTypeNameBuilder.h` | Type name generation |
| `TypePool.h` | Type deduplication pool |

## For AI Agents

### Working In This Directory

1. Understand parallel DWARF linking architecture
2. Know thread synchronization and work-stealing patterns
3. Study dependency tracking to maintain correctness across parallel units
4. Test with large multi-unit binaries (Google Chrome, LLVM itself)
5. Verify thread safety of shared data structures
6. Benchmark parallel vs. classic linker performance
7. Handle edge cases: circular type references, forward declarations
8. Test on multi-core systems and NUMA architectures

### Key Patterns

- DependencyTracker identifies unit dependencies
- Work queue distributes compile units across threads
- Shared type pool deduplicates types across threads with synchronization
- Accelerator tables (name index, type index) built concurrently
- Address relocation applied per-unit without synchronization barriers

## Dependencies

### Internal
- Depends on: LLVM Support, LLVM DebugInfo, LLVM MC, threading libraries
- Used by: Modern linkers optimizing for multi-core systems

<!-- MANUAL: -->
