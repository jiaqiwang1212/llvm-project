<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XRay

## Purpose
XRay function tracing and instrumentation infrastructure. Provides low-overhead tracing for runtime behavior analysis, profiling, and debugging.

## Key Files
| File | Description |
|------|-------------|
| `InstrumentationMap.h` | XRay instrumentation metadata |
| `Trace.h` | Trace data structures and analysis |
| `FDRRecords.h` | Flight Data Recorder (FDR) record types |
| `FDRLogBuilder.h` | FDR log construction |
| `FDRRecordConsumer.h` | FDR record consumption interface |
| `FDRRecordProducer.h` | FDR record generation |
| `FDRTraceExpander.h` | Expand compressed trace records |
| `FDRTraceWriter.h` | Write FDR trace files |
| `FileHeaderReader.h` | Read XRay trace file headers |
| `BlockIndexer.h` | Index trace blocks |
| `BlockPrinter.h` | Pretty-print trace blocks |
| `BlockVerifier.h` | Validate trace integrity |
| `Graph.h` | Trace call graph construction |
| `Profile.h` | Profiling statistics from traces |
| `RecordPrinter.h` | Pretty-print individual records |
| `XRayRecord.h` | Common record format |
| `YAMLXRayRecord.h` | YAML serialization |

## For AI Agents

### Working In This Directory
XRay provides runtime tracing infrastructure:
- **Instrumentation**: Compiler-inserted probe points in functions
- **FDR mode**: Fast, continuous tracing with ring buffer
- **Trace analysis**: Parse and analyze recorded traces
- **Call graphs**: Extract calling relationships from traces
- **Performance profiling**: Statistical analysis of trace data

### Common Patterns
- Instrument functions via -fxray-instrument flag
- FDR logging for continuous, low-overhead tracing
- Parse binary trace files for offline analysis
- Build call graphs and extract hotspots

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- `llvm/ADT` (SmallVector, DenseMap)

<!-- MANUAL: -->
