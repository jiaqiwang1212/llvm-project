<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# CGData

## Purpose

Code generation data collection infrastructure for LTO (Link-Time Optimization). Gathers profiling and analysis data during code generation for use in optimization decisions.

## Key Files

| File | Description |
|------|-------------|
| `CodeGenData.cpp` | Code generation data structures and management |
| `CodeGenDataReader.cpp` | CGData format reader |
| `CodeGenDataWriter.cpp` | CGData format writer |
| `OutlinedHashTree.cpp` | Hash tree for outlined function deduplication |
| `OutlinedHashTreeRecord.cpp` | Hash tree record serialization |
| `StableFunctionMap.cpp` | Stable function identifier mapping |
| `StableFunctionMapRecord.cpp` | Stable function map record serialization |

## For AI Agents

### Working In This Directory

1. Understand CodeGen data: outlinable code, function similarities, duplication opportunities
2. Know hash tree structure for function matching
3. Test data collection during ThinLTO and LTO
4. Verify stability: same code produces same hashes across builds
5. Test cross-module function deduplication
6. Handle data format versioning and compatibility

### Key Patterns

- Outlined functions identified by content hash
- Hash tree enables efficient similarity matching
- StableFunctionMap maintains consistent IDs across builds
- Data collected during compilation, consumed during linking

## Dependencies

### Internal
- Depends on: LLVM IR, CodeGen (lib/CodeGen), LTO (lib/LTO)
- Used by: ThinLTO, full LTO, distributed builds

<!-- MANUAL: -->
