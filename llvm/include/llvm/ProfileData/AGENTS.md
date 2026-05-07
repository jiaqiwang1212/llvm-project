<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ProfileData

## Purpose
Profile data collection and analysis for profile-guided optimization (PGO), sampling, and memory profiling. Handles instrumentation profiles, coverage information, and performance metrics.

## Key Files
| File | Description |
|------|-------------|
| `InstrProf.h` | Instrumentation profile reader/writer |
| `InstrProfReader.h` | Profile data input |
| `InstrProfWriter.h` | Profile data output |
| `SampleProf.h` | Sampling-based profiling |
| `SampleProfReader.h` | Sample profile input |
| `SampleProfWriter.h` | Sample profile output |
| `GCOV.h` | GCOV format support |
| `MemProf.h` | Memory profiling data structures |
| `MemProfReader.h` | Memory profile input |
| `MemProfSummary.h` | Memory profiling summary |
| `MemProfYAML.h` | YAML serialization for memprof |
| `FunctionId.h` | Stable function identification |
| `Coverage/` | Code coverage analysis |
| `ProfileCommon.h` | Common profile utilities |
| `ItaniumManglingCanonicalizer.h` | C++ name demangling |
| `DataAccessProf.h` | Data access profiling |
| `PGOCtxProfReader.h` | Context-sensitive PGO input |
| `PGOCtxProfWriter.h` | Context-sensitive PGO output |
| `InstrProfCorrelator.h` | Correlate binary with profiles |
| `SymbolRemappingReader.h` | Symbol name remapping |

## For AI Agents

### Working In This Directory
ProfileData manages multiple profiling modes:
- **Instrumentation-based (PGO)**: Runtime counter collection
- **Sampling-based**: Statistical profiling (less overhead)
- **Memory profiling**: Allocation tracking and hotspot detection
- **Coverage**: Code coverage reporting

### Common Patterns
- Reader/Writer pairs for each format
- InstrProfRecord iteration for PGO analysis
- Function ID mapping for stable identification across builds
- YAML serialization for inspection/debugging

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- `llvm/ADT` (DenseMap, SmallVector)

<!-- MANUAL: -->
