<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Fuzzer

## Purpose
Contains the LibFuzzer implementation for coverage-guided fuzz testing. LibFuzzer is a library-based in-process fuzzer that drives target functions with mutated inputs to find crashes and undefined behavior.

## Key Files
| File | Description |
|------|-------------|
| `README.txt` | LibFuzzer documentation and usage guide |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- LibFuzzer is implemented as a compiler-rt library (not in llvm/lib)
- This directory holds supplementary documentation and build configuration
- Reference for fuzzing LLVM tools and libraries
- Integration point with FuzzMutate/ for IR-level fuzzing

## Dependencies

### Internal
- `llvm/lib/FuzzMutate/` — IR mutation for coverage-guided fuzzing

<!-- MANUAL: -->
