<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IRReader

## Purpose
Unified IR reader that automatically detects and parses both binary bitcode and text-based IR formats. Central entry point for loading LLVM IR from files.

## Key Files
| File | Description |
|------|-------------|
| `IRReader.cpp` | Unified IR reader with format auto-detection |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Format detection: bitcode vs text IR
- Error recovery and diagnostic reporting
- Integration with Bitcode/ and AsmParser/ readers
- Module loading abstraction

## Dependencies

### Internal
- `llvm/lib/Bitcode/` — binary bitcode reading
- `llvm/lib/AsmParser/` — text IR parsing
- `llvm/lib/IR/` — IR representation

<!-- MANUAL: -->
