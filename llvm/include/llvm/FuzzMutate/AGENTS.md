<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuzzMutate

## Purpose
IR mutation and fuzzing support for libFuzzer integration. Generates random IR transformations to test compiler robustness.

## Key Files
| File | Description |
|------|-------------|
| `IRMutator.h` | IR mutation engine |
| `RandomIRBuilder.h` | Random IR construction utilities |
| `OpDescriptor.h` | Operation metadata for fuzzing |
| `Operations.h` | Supported mutation operations |
| `Random.h` | Randomness utilities |
| `FuzzerCLI.h` | Command-line fuzzer framework |

## For AI Agents

### Working In This Directory
FuzzMutate provides structured IR fuzzing:
- **IR mutation**: Random but valid IR transformations
- **Operation descriptors**: Metadata for legal mutations
- **Seed corpus**: Starting points for fuzzing campaigns
- **libFuzzer integration**: Target interface for fuzzer engine

### Common Patterns
- Create RandomIRBuilder with seed and module
- Use IRMutator to apply mutations
- OpDescriptor for defining mutation possibilities
- Corpus of valid IR as fuzzing seeds

## Dependencies

### Internal
- `llvm/IR` (Module, Function, Instruction)
- `llvm/Support` (random_device, mt19937)
- `llvm/ADT` (SmallVector, DenseMap)

<!-- MANUAL: -->
