<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ProfileData

## Purpose
Handles profile data reading and writing for instrumentation-based profiling (PGO), sampling profiles, and memory profiling. Central infrastructure for profile-guided optimization.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Coverage/` | Coverage mapping data (see `Coverage/AGENTS.md`) |

## Key Files
| File | Description |
|------|-------------|
| `InstrProf.cpp` | Instrumentation profile format |
| `InstrProfReader.cpp` | Profile data reader |
| `InstrProfWriter.cpp` | Profile data writer |
| `InstrProfCorrelator.cpp` | Profile correlation with source |
| `MemProf.cpp` | Memory profiling format |
| `MemProfReader.cpp` | Memory profile reader |
| `MemProfSummary.cpp` | Memory profile summaries |
| `MemProfRadixTree.cpp` | Radix tree for memory profiles |
| `SampleProf.cpp` | Sampling profile format |
| `GCOV.cpp` | GCOV format support |
| `ProfileSummaryBuilder.cpp` | Profile statistics aggregation |
| `PGOCtxProfReader.cpp` | Context-sensitive PGO reader |
| `PGOCtxProfWriter.cpp` | Context-sensitive PGO writer |
| `DataAccessProf.cpp` | Data access profiling |
| `ItaniumManglingCanonicalizer.cpp` | Name demangling for profiles |
| `IndexedMemProfData.cpp` | Indexed memory profile data |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Multiple profile format support: LLVM IR, text, binary
- Profile aggregation and merging
- Integration with CodeGen for optimization hints
- PGO instrumentation infrastructure
- Memory profiling data structures

## Dependencies

### Internal
- `llvm/lib/IR/` — IR representation
- `llvm/lib/Bitstream/` — binary format
- `llvm/lib/Support/` — utilities

<!-- MANUAL: -->
