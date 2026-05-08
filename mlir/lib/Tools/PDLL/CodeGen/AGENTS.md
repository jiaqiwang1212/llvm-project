<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/PDLL/CodeGen

## Purpose
Implements the two PDLL code generation backends: C++ source generation (for static pattern compilation) and PDL dialect op generation (for runtime interpretation via the PDL bytecode interpreter).

## Key Files
| File | Description |
|------|-------------|
| `CPPGen.cpp` | `mlir::pdll::codegen::generateCPP()`: walks the PDLL AST and emits a C++ `.cpp` file containing `RewritePattern` subclasses; used by `mlir-pdll --output-type=cpp` |
| `MLIRGen.cpp` | `mlir::pdll::codegen::generateMLIR()`: walks the PDLL AST and emits `pdl.*` dialect ops into an `mlir::ModuleOp`; used by `mlir-pdll --output-type=mlir` and the default path for runtime PDL interpretation |

## For AI Agents

### Working In This Directory
- `MLIRGen.cpp` is the more commonly used backend; its output is interpreted by the PDL bytecode interpreter in `mlir/lib/Rewrite/ByteCode.cpp`.
- `CPPGen.cpp` output is human-readable C++ patterns useful when you want zero-overhead pattern matching compiled into a pass.
- Both backends must stay in sync with changes to PDLL AST node types.

### Common Patterns
- `MLIRGen` uses `OpBuilder` to construct PDL dialect ops directly into an `mlir::Block`.
- `CPPGen` uses string templating via `llvm::raw_ostream` to emit C++ code fragments.

## Dependencies

### Internal
- `mlir/lib/Tools/PDLL/AST/` — AST nodes consumed by both backends
- `mlir/lib/Dialect/PDL/` — PDL dialect ops emitted by `MLIRGen`
- `mlir/lib/IR/` — `OpBuilder`, `MLIRContext`

### External
- `llvm/lib/Support` — `llvm::raw_ostream`, ADT

<!-- MANUAL: -->
