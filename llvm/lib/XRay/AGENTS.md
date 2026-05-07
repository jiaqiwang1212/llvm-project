<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# XRay

## Purpose
XRay instrumentation data parsing and runtime support. Provides infrastructure for low-overhead function call tracing and dynamic performance analysis.

## Key Files
| File | Description |
|------|-------------|
| `Trace.cpp` | XRay trace file parsing and analysis |
| `Profile.cpp` | Profile data aggregation |
| `FDRRecords.cpp` | Flight Data Recorder (FDR) format records |
| `FDRRecordProducer.cpp` | FDR record generation |
| `FDRTraceExpander.cpp` | FDR trace expansion and processing |
| `FDRTraceWriter.cpp` | FDR trace output |
| `FileHeaderReader.cpp` | XRay file format header parsing |
| `InstrumentationMap.cpp` | Instrumentation mapping |
| `LogBuilderConsumer.cpp` | Log data consumer |
| `RecordInitializer.cpp` | Record initialization |
| `RecordPrinter.cpp` | Record output formatting |
| `BlockIndexer.cpp` | Block indexing for traces |
| `BlockPrinter.cpp` | Block formatting |
| `BlockVerifier.cpp` | Block validation |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- XRay trace file format specification
- Flight Data Recorder (FDR) implementation
- Function tracing and latency measurement
- Profile data aggregation and reporting
- Instrumentation point tracking

## Dependencies

### Internal
- `llvm/lib/IR/` — IR references for instrumentation
- `llvm/lib/Support/` — utilities, error handling

<!-- MANUAL: -->
