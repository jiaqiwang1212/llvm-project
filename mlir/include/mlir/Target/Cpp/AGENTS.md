<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Target/Cpp/

## Purpose
Headers for the C/C++ emitter target. Translates MLIR modules using the EmitC dialect to C or C++ source code. The emitter walks the EmitC dialect ops and produces compilable C/C++ output, enabling MLIR as a C/C++ codegen backend.

## Key Files
| File | Description |
|------|-------------|
| `CppEmitter.h` | `translateToCpp()` — translate an MLIR module or op to C/C++ source on a `raw_ostream` |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Input must consist primarily of EmitC dialect ops; standard ops like `func.func` are also supported.
- `translateToCpp()` takes a `declareVariablesAtTop` flag for C89-style variable declarations.
- The emitter is used by the `--emit-c` / `--emit-cpp` translation registration.

### Common Patterns
- `mlir::emitc::translateToCpp(module, llvm::outs(), /*declareVariablesAtTop=*/false);`

## Dependencies

### Internal
- `mlir/Dialect/EmitC/` (EmitC ops and types)
- `mlir/IR/` (ModuleOp, Operation)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/raw_ostream.h`

<!-- MANUAL: -->
