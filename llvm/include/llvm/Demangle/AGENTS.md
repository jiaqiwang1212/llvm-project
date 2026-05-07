<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Demangle

## Purpose
C++, Rust, D, and Swift symbol name demangling. Converts compiler-mangled names back to human-readable declarations for debugging and profiling.

## Key Files
| File | Description |
|------|-------------|
| `Demangle.h` | Universal demangling interface |
| `DemangleConfig.h` | Configuration for demangling behavior |
| `ItaniumDemangle.h` | Itanium ABI demangling (C++/Rust) |
| `ItaniumNodes.def` | Itanium node type definitions |
| `MicrosoftDemangle.h` | Microsoft mangling scheme demangling |
| `MicrosoftDemangleNodes.h` | Microsoft node type definitions |
| `StringViewExtras.h` | String utilities |
| `Utility.h` | Common utilities |
| `README.txt` | Format documentation |

## For AI Agents

### Working In This Directory
Demangle provides language-aware name demangling:
- **Itanium ABI**: Primary C++ and Rust demangling scheme
- **Microsoft mangling**: MSVC and Windows symbol names
- **D and Swift**: Language-specific demangling
- **Selective output**: Control verbosity (types, templates, etc.)

### Common Patterns
- demangle(symbol_name) for simple demangling
- Context class for custom output handling
- Selective demangling of components
- Error handling for malformed names

## Dependencies

### Internal
- `llvm/Support` (StringRef, raw_ostream)

<!-- MANUAL: -->
