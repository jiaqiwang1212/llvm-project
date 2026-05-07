<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FileCheck

## Purpose
FileCheck pattern matching engine for compiler tests. Verifies textual output (assembly, IR) against expected patterns with support for variables, regex, and cross-checks.

## Key Files
| File | Description |
|------|-------------|
| `FileCheck.h` | Main FileCheck interface |

## For AI Agents

### Working In This Directory
FileCheck is the standard LLVM test utility:
- **Pattern matching**: Exact strings, regexes, or variable captures
- **Directives**: CHECK, CHECK-NEXT, CHECK-LABEL, CHECK-NOT, CHECK-DAG, etc.
- **Variable capture**: Store and reuse substrings in subsequent patterns
- **Cross-checks**: Validate relationships between matched items

### Common Patterns
- FileCheck files contain directives inline with expected output
- Integration with llvm-lit test runner
- Common in IR tests (test/CodeGen/, test/Transforms/)
- Regex patterns for architecture-independent checks

## Dependencies

### Internal
- `llvm/Support` (StringRef, Regex)
- `llvm/ADT` (SmallVector)

<!-- MANUAL: -->
