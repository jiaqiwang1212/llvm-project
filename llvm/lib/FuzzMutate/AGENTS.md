<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuzzMutate

## Purpose
Provides IR mutation strategies and random IR generation for coverage-guided fuzzing. Enables systematic generation and transformation of LLVM IR to discover compiler bugs and optimizations issues.

## Key Files
| File | Description |
|------|-------------|
| `FuzzerCLI.cpp` | Command-line interface for fuzz driver |
| `IRMutator.cpp` | IR mutation strategies and transformations |
| `OpDescriptor.cpp` | LLVM operation descriptors |
| `Operations.cpp` | Operation metadata and properties |
| `RandomIRBuilder.cpp` | Random valid IR generation |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Understand IR mutation operators and constraints
- Valid IR generation respects type system and instruction semantics
- Random seed control for reproducible fuzzing
- Integration with LibFuzzer for continuous integration

## Dependencies

### Internal
- `llvm/lib/IR/` — IR representation and types
- `llvm/lib/Fuzzer/` — LibFuzzer driver

<!-- MANUAL: -->
