<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/mlir-tblgen

## Purpose
Implements `MlirTblgenMain()`, the entry-point library for `mlir-tblgen`. Extends the LLVM TableGen tool with MLIR-specific code generation backends (op declarations, op definitions, dialect C++ boilerplate, interface headers, pass declarations, Python bindings, etc.).

## Key Files
| File | Description |
|------|-------------|
| `MlirTblgenMain.cpp` | `mlir::MlirTblgenMain()`: calls `llvm::TableGenMain()` with the MLIR backend registry; all MLIR-specific generators are registered via `GenRegistration` objects in `mlir/lib/TableGen/GenInfo.cpp` |

## For AI Agents

### Working In This Directory
- Backend implementations are in `mlir/lib/TableGen/` and additional per-backend files in this directory may exist (though most backends live in `mlir/lib/TableGen/`).
- To add a new backend: implement the generator function, instantiate a `GenRegistration("--gen-my-thing", "Description", myGeneratorFn)` at file scope, and link it into `mlir-tblgen`.
- Run backends with: `mlir-tblgen --gen-op-decls -I mlir/include mlir/include/mlir/Dialect/Foo/IR/FooOps.td`

### Common Patterns
- Generators take a `llvm::RecordKeeper&` and write to `llvm::raw_ostream&`.
- Backend selection is via `--gen-*` command-line flags handled by `llvm::TableGenMain`.

## Dependencies

### Internal
- `mlir/lib/TableGen/` — TableGen record wrappers
- `llvm/lib/TableGen` — `llvm::RecordKeeper`, `llvm::TableGenMain`

### External
- `llvm/lib/Support` — `llvm::cl`, `llvm::raw_ostream`

<!-- MANUAL: -->
