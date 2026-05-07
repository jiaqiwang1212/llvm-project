<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Remarks

## Purpose
Optimization remarks framework for reporting compiler transformations (inlined, vectorized, etc.) in human-readable and machine-parseable formats (YAML, bitstream).

## Key Files
| File | Description |
|------|-------------|
| `Remark.h` | Core remark data structure |
| `RemarkParser.h` | Parse remarks from files |
| `RemarkSerializer.h` | Serialize remarks to output |
| `YAMLRemarkSerializer.h` | YAML-specific serialization |
| `BitstreamRemarkSerializer.h` | Bitstream format serialization |
| `BitstreamRemarkContainer.h` | Bitstream container format |
| `RemarkFormat.h` | Format definitions and utilities |
| `RemarkStreamer.h` | Stream remarks during compilation |
| `RemarkStringTable.h` | Intern strings for compact storage |
| `RemarkLinker.h` | Combine remarks from multiple sources |
| `HotnessThresholdParser.h` | Filter remarks by hotness |

## For AI Agents

### Working In This Directory
Remarks provide post-compilation diagnostics:
- **Remark types**: Passed, Missed, Analysis, AnalysisFPCommute, AnalysisAliasing, Failure
- **Formats**: YAML (human-readable), bitstream (compact), JSON (alt)
- **Filtering**: Hotness-based filtering for important remarks
- **Linking**: Aggregate remarks from parallel builds

### Common Patterns
- Streamer-based emission during IR passes
- Parser for external remark consumption
- String table interning for binary formats
- Hotness-based filtering in optimization reports

## Dependencies

### Internal
- `llvm/Support` (StringRef, raw_ostream, Error)
- `llvm/IR` (for remark generation)

<!-- MANUAL: -->
