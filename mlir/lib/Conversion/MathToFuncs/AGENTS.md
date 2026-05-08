<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToFuncs Conversion

## Purpose
Lowers Math dialect ops to function calls for ops that don't have direct LLVM intrinsic equivalents. Generates scalar function implementations (e.g., integer `math.ipowi`, `math.fpowi`) as `func.func` declarations or definitions.

## Key Files
| File | Description |
|------|-------------|
| `MathToFuncs.cpp` | Patterns creating function declarations/definitions for math ops lacking direct intrinsics |

## For AI Agents

### Working In This Directory
- Handles `math.ipowi` (integer power), `math.fpowi` (float base, integer exponent) which have no single LLVM intrinsic.
- Generates a helper function with a loop-based implementation and replaces op with a call to it.
- Also handles `math.ctlz`/`math.cttz` on vector types that LLVM doesn't natively support.
- Must run before final LLVM lowering; the generated `func.func` ops will be lowered by `FuncToLLVM`.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/Func`, `mlir/Dialect/Arith`, `mlir/Dialect/SCF`

<!-- MANUAL: -->
