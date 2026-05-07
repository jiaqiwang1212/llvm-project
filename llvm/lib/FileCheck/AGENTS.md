<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FileCheck

## Purpose
Implements the FileCheck pattern matching engine that verifies compiler output against expected patterns. Powers test automation across LLVM and Clang test suites.

## Key Files
| File | Description |
|------|-------------|
| `FileCheck.cpp` | Main FileCheck implementation |
| `FileCheckImpl.h` | Internal implementation details |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Understand pattern matching grammar and evaluation
- Pattern directives: CHECK, CHECK-NEXT, CHECK-SAME, CHECK-NOT, etc.
- Regular expression and variable capture mechanisms
- Used extensively in testing infrastructure

## Dependencies

### Internal
- `llvm/lib/Support/` — string utilities, regex support

<!-- MANUAL: -->
