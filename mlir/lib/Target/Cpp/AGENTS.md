<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Target/Cpp

## Purpose
Implements translation from the MLIR EmitC dialect to C++ source text. Enables generating compilable C++ code from MLIR IR, useful for embedded targets and frameworks that consume C++ rather than LLVM IR.

## Key Files
| File | Description |
|------|-------------|
| `TranslateToCpp.cpp` | `mlir::translateModuleToCpp()`: walks EmitC dialect ops and emits C++ declarations, expressions, and statements to an `llvm::raw_ostream` |
| `TranslateRegistration.cpp` | Registers the `"emit-c"` translation with `mlir-translate` |

## For AI Agents

### Working In This Directory
- Only EmitC dialect ops are supported; the input module must be fully lowered to EmitC before invoking this translation.
- C++ output style (indentation, include guards) is fixed; not configurable.
- Tests are in `mlir/test/Target/Cpp/`.

### Common Patterns
- The translator walks the op tree recursively, emitting C++ for each EmitC op.
- Function signatures are emitted from `emitc.func` ops; expressions from `emitc.expression` ops.

## Dependencies

### Internal
- `mlir/lib/Dialect/EmitC/` — EmitC dialect ops

### External
- `llvm/lib/Support` — `llvm::raw_ostream`

<!-- MANUAL: -->
