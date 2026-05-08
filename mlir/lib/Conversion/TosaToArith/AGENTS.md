<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToArith

## Purpose
Implements the TOSA to Arith lowering pass. Converts TOSA constant and cast ops into `arith` dialect equivalents, handling integer/float type casting and constant materialization.

## Key Files
| File | Description |
|------|-------------|
| `TosaToArith.cpp` | Conversion patterns from TOSA ops to arith ops |
| `TosaToArithPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTosaToArithConversionPatterns()`
- Handles `tosa.const`, `tosa.cast`, and related scalar/element-wise ops

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- TOSA casts lower to sequences of `arith.sitofp`, `arith.fptosi`, `arith.extsi`, etc.

## Dependencies
- Headers: `include/mlir/Conversion/TosaToArith/`
- Source dialect: `lib/Dialect/Tosa/`
- Target dialect: `lib/Dialect/Arith/`

<!-- MANUAL: -->
