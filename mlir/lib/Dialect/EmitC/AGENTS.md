<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# EmitC Dialect

## Purpose
Implements the EmitC dialect, which provides ops that map directly to C/C++ constructs for source-level code emission. Used as a compilation target to generate portable C code from MLIR programs — an alternative to LLVM IR lowering.

## Key Files
| File | Description |
|------|-------------|
| `IR/EmitC.cpp` | Op implementations: `emitc.call`, `emitc.constant`, `emitc.cast`, `emitc.if`, `emitc.for`, etc. |
| `Transforms/FormExpressions.cpp` | Converts op-based computation to C expression trees |
| `Transforms/Transforms.cpp` | General EmitC transformation passes |
| `Transforms/TypeConversions.cpp` | Type conversion utilities for C types |
| `Transforms/WrapFuncInClass.cpp` | Wraps functions in C++ class wrappers |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | All EmitC ops and types |
| `Transforms/` | Expression formation, type conversions, C++ class wrapping |

## For AI Agents

### Working In This Directory
- EmitC ops are printed directly by the `mlir-translate --mlir-to-cpp` emitter; op structure must match C syntax constraints.
- `emitc.call` carries a `callee` string attribute — it emits `callee(args...)` literally.
- `emitc.opaque` wraps arbitrary C expressions as a string — use sparingly, breaks analysis.
- `FormExpressions.cpp` converts SSA-form arithmetic into nested expression trees to emit clean C without temp variables.
- Types in EmitC include `emitc.ptr<T>`, `emitc.array<N x T>`, and opaque types for C typedefs.

### Common Patterns
- Verifiers ensure `emitc.call` argument types are expressible in C.
- `WrapFuncInClass` restructures `func.func` ops into a C++ struct with `operator()`.

## Dependencies
- `mlir/Dialect/Func`, `mlir/Dialect/Arith`, `mlir/Dialect/ControlFlow`

<!-- MANUAL: -->
