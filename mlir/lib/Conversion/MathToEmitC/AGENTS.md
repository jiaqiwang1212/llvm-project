<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToEmitC Conversion

## Purpose
Lowers Math dialect ops to EmitC ops for C/C++ emission. Converts transcendental ops like `math.exp`, `math.log`, `math.sqrt` to `emitc.call` ops invoking the corresponding C `<math.h>` functions.

## Key Files
| File | Description |
|------|-------------|
| `MathToEmitC.cpp` | Patterns mapping math ops to emitc.call ops |
| `MathToEmitCPass.cpp` | Pass definition |

## For AI Agents

### Working In This Directory
- `math.sqrt` → `emitc.call "sqrtf"` (f32) or `emitc.call "sqrt"` (f64).
- `math.exp` → `emitc.call "expf"` / `emitc.call "exp"`, etc.
- Function names follow C `<math.h>` conventions with `f` suffix for single precision.
- Part of the EmitC lowering pipeline; pairs with `ArithToEmitC` for complete arithmetic coverage.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/EmitC`

<!-- MANUAL: -->
