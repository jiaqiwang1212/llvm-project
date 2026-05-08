<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/IRDLToCpp

## Purpose
Translates IRDL (IR Definition Language) dialect definitions to C++ boilerplate code for MLIR dialects. Generates the type/op/attribute registration C++ that would otherwise be written by hand or via TableGen.

## Key Files
| File | Description |
|------|-------------|
| `IRDLToCpp.cpp` | Core translator: walks IRDL dialect, type, and op definitions and emits C++ class declarations and registration code |
| `TranslationRegistration.cpp` | Registers the `"irdl-to-cpp"` translation with `mlir-translate` |
| `TemplatingUtils.h` | Internal utilities for C++ code template expansion |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Templates/` | C++ code templates (string literals or files) used by the translator to fill in boilerplate |

## For AI Agents

### Working In This Directory
- IRDL is the runtime-interpretable alternative to TableGen; this target bridges IRDL back to static C++.
- The generated C++ is not meant for production use without review; it is a starting point for dialect authors.

## Dependencies

### Internal
- `mlir/lib/Dialect/IRDL/` — IRDL dialect ops

### External
- `llvm/lib/Support` — `llvm::raw_ostream`

<!-- MANUAL: -->
