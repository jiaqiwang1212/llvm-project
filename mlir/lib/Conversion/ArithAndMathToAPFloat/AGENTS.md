<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithAndMathToAPFloat Conversion

## Purpose
Provides compile-time constant folding for Arith and Math ops by evaluating them using LLVM's APFloat library. Used to fold floating-point constant expressions at compile time with correct IEEE 754 semantics.

## Key Files
| File | Description |
|------|-------------|
| `ArithToAPFloat.cpp` | Constant folding for arith float ops using APFloat |
| `MathToAPFloat.cpp` | Constant folding for math transcendental ops using APFloat |
| `Utils.cpp` | Shared APFloat conversion utilities |
| `Utils.h` | Internal utilities header |

## For AI Agents

### Working In This Directory
- This is a folder/evaluator library, not a traditional lowering pass — it provides fold hooks that plug into the constant folding infrastructure.
- APFloat is used for correct IEEE 754 rounding mode semantics in constant evaluation.
- `MathToAPFloat.cpp` provides evaluation of `math.exp`, `math.log`, etc. for constant inputs.
- Results are `FloatAttr` / `DenseElementsAttr` for scalar and vector constants respectively.

## Dependencies
- Source: `mlir/Dialect/Arith`, `mlir/Dialect/Math`
- LLVM `APFloat`

<!-- MANUAL: -->
