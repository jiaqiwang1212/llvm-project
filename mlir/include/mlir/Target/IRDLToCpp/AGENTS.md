<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/IRDLToCpp/

## Purpose
Headers for translating IRDL (IR Definition Language) dialect definitions to C++ code. Enables generating C++ dialect boilerplate from IRDL definitions, providing a path from dynamic dialect specifications to static compiled dialects.

## Key Files
| File | Description |
|------|-------------|
| `IRDLToCpp.h` | `translateIRDLToCpp()` — translate an IRDL dialect definition to C++ header/source |
| `TranslationRegistration.h` | Registers the IRDL-to-C++ translation with `mlir-translate` |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Input is an MLIR module containing `irdl.dialect` ops.
- Output is C++ code that can be compiled to create a static MLIR dialect.
- This translation is registered as `--irdl-to-cpp` in `mlir-translate`.

## Dependencies

### Internal
- `mlir/Dialect/IRDL/` (IRDL dialect ops)
- `mlir/IR/` (ModuleOp)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/raw_ostream.h`

<!-- MANUAL: -->
