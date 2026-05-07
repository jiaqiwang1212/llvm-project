<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CGData

## Purpose
Code generation data collection and analysis across LTO and incremental compilation. Aggregates profiling information for function outlines and code generation decisions.

## Key Files
| File | Description |
|------|-------------|
| `CodeGenData.h` | Main code generation data structure |
| `CodeGenDataReader.h` | Read codegen data from disk |
| `CodeGenDataWriter.h` | Write codegen data to disk |
| `CodeGenData.inc` | Data format definitions |
| `OutlinedHashTree.h` | Hash tree for code outlines |
| `OutlinedHashTreeRecord.h` | Hash tree serialization |
| `StableFunctionMap.h` | Stable function identification |
| `StableFunctionMapRecord.h` | Stable map serialization |
| `CGDataPatchItem.h` | Incremental update patches |

## For AI Agents

### Working In This Directory
CGData aggregates compiler-collected metrics for optimization:
- **Function outlines**: Identify code patterns and similarities
- **Hash trees**: Compact representation of function bodies
- **Stable mapping**: Consistent function IDs across builds
- **LTO integration**: Share data across link-time optimization phases

### Common Patterns
- Collect codegen metrics during compilation
- Store in lightweight binary format
- Load for incremental/cross-module analysis
- Use with PGO and other optimization data

## Dependencies

### Internal
- `llvm/Support` (StringRef, Error, raw_ostream)
- `llvm/ADT` (DenseMap, SmallVector)

<!-- MANUAL: -->
