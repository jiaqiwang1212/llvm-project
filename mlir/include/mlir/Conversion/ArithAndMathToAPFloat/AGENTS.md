<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithAndMathToAPFloat

## Purpose
Declares lowering passes for Arith and Math dialect ops to APFloat-based software floating-point emulation. Enables floating-point operations on targets lacking native hardware FP support.

## Key Files
| File | Description |
|------|-------------|
| `ArithToAPFloat.h` | Pass declarations for Arith-to-APFloat conversion patterns |
| `MathToAPFloat.h` | Pass declarations for Math-to-APFloat conversion patterns |

## For AI Agents

### Working In This Directory
- Edit the respective `.h` files to add or modify pass declarations
- The implementation lives under `mlir/lib/Conversion/ArithAndMathToAPFloat/`

## Dependencies
- Source dialects: `include/mlir/Dialect/Arith/`, `include/mlir/Dialect/Math/`
- Target: APFloat software emulation via func calls

<!-- MANUAL: -->
