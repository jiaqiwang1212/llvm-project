<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Func Dialect

## Purpose
Implements the Func dialect — MLIR's standard function abstraction. Provides `func.func`, `func.call`, `func.call_indirect`, and `func.return` ops. Acts as the primary function-level container for most MLIR programs.

## Key Files
| File | Description |
|------|-------------|
| `IR/FuncOps.cpp` | `func.func`, `func.call`, `func.call_indirect`, `func.return` implementations |
| `Transforms/DuplicateFunctionElimination.cpp` | Eliminates duplicate functions with identical bodies |
| `Transforms/FuncConversions.cpp` | Type conversion patterns for function signatures and call sites |
| `Extensions/` | `BufferizableOpInterface` and inliner interface impls |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Core function ops |
| `Transforms/` | Function-level optimization passes |
| `Extensions/` | Interface implementations for bufferization and inlining |
| `TransformOps/` | Transform dialect extension for function transformations |
| `Utils/` | Shared function utilities |

## For AI Agents

### Working In This Directory
- `func.func` carries a `FunctionType` — type changes require updating both the function attribute and all call sites.
- `FuncConversions.cpp` provides `FuncOpSignatureConversion` patterns used by dialect conversion passes to rewrite function signatures.
- The inliner interface in `Extensions/` enables `mlir-opt --inline` to inline `func.call` ops.
- `DuplicateFunctionElimination` compares function bodies structurally — useful for deduplication after inlining/specialization.

### Common Patterns
- To change function signatures: use `ConvertFuncToLLVM`-style `FuncOpSignatureConversion` which rewrite function types via `TypeConverter`.
- Builders: `func::FuncOp::create()` with a `FunctionType` and optional `StringAttr` for visibility.
- Return type inference: `func.return` verifier checks operand types match enclosing function result types.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Arith`, `mlir/Transforms/InliningUtils`

<!-- MANUAL: -->
