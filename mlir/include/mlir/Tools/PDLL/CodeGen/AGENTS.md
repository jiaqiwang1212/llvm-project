<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/PDLL/CodeGen/

## Purpose
PDLL code generation backends. Provides two code generators: one that emits C++ rewrite pattern code (for static compilation) and one that emits MLIR PDL dialect IR (for runtime interpretation via the PDL interpreter).

## Key Files
| File | Description |
|------|-------------|
| `CPPGen.h` | `codegenPDLLToCPP()` — generate C++ pattern code from a PDLL AST module |
| `MLIRGen.h` | `codegenPDLLToMLIR()` — generate MLIR PDL dialect IR from a PDLL AST module |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- `CPPGen` output is a `.cpp.inc` file included by dialect pattern files.
- `MLIRGen` output is a `pdl.module` op containing `pdl.pattern` ops.
- Choose C++ codegen for static patterns, MLIR codegen for runtime/dynamic use via PDL interpreter.

### Common Patterns
- C++ gen: `codegenPDLLToCPP(*astModule, llvm::outs());`
- MLIR gen: `OwningOpRef<ModuleOp> pdlModule = codegenPDLLToMLIR(ctx, *astModule);`

## Dependencies

### Internal
- `mlir/Tools/PDLL/AST/` (input AST)
- `mlir/Dialect/PDL/` (output PDL dialect ops)
- `mlir/IR/` (MLIRContext, ModuleOp)

### External
- `llvm/Support/raw_ostream.h`

<!-- MANUAL: -->
