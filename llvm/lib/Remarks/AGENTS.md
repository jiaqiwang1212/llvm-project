<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Remarks

## Purpose
Optimization remark serialization and parsing in multiple formats (YAML, bitstream). Provides structured feedback about compiler transformations for analysis and reporting.

## Key Files
| File | Description |
|------|-------------|
| `Remark.cpp` | Remark data structure and utilities |
| `RemarkParser.cpp` | Generic remark parsing interface |
| `RemarkSerializer.cpp` | Generic remark serialization |
| `YAMLRemarkParser.cpp` | YAML format parser |
| `YAMLRemarkSerializer.cpp` | YAML format serialization |
| `BitstreamRemarkParser.cpp` | Bitstream format parser |
| `BitstreamRemarkSerializer.cpp` | Bitstream format serialization |
| `RemarkFormat.cpp` | Format detection and selection |
| `RemarkLinker.cpp` | Remark merging across modules |
| `RemarkStreamer.cpp` | Streaming remark output |
| `RemarkStringTable.cpp` | String deduplication |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Multi-format remark support
- Structured optimization feedback
- Integration with CodeGen and Transforms
- Remark linking and merging
- String table compression

## Dependencies

### Internal
- `llvm/lib/Support/` — utilities, error handling
- `llvm/lib/Bitstream/` — binary format
- `llvm/lib/IR/` — IR references

<!-- MANUAL: -->
