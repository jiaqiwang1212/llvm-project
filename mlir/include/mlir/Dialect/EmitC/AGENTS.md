<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# EmitC Dialect

## Purpose
Dialect for emitting C or C++ source code directly from MLIR IR. Provides ops that map to C constructs (variables, expressions, calls, casts, includes) enabling direct C code generation as a compilation target.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, attribute, and interface definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes and type conversions (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- EmitC is a target dialect — lower to it, then use the emitter to produce `.c`/`.cpp` files
- Op names follow `emitc.*` convention

### Common Patterns
- `emitc.call_opaque` represents an arbitrary C function call
- `emitc.include` inserts `#include` directives in generated output
- Types include `emitc.ptr<T>`, `emitc.array<NxT>`, `emitc.opaque<"type_string">`

## Dependencies
- Receives lowerings from: Arith, Func, Math, MemRef, SCF dialects

<!-- MANUAL: -->
