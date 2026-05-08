<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToLibm Conversion

## Purpose
Lowers Complex dialect ops to libm function calls. Converts transcendental complex ops (`complex.exp`, `complex.log`, `complex.pow`, `complex.sqrt`, etc.) to calls to the standard C math library (`cexpf`, `clogf`, `cpowf`, etc.).

## Key Files
| File | Description |
|------|-------------|
| `ComplexToLibm.cpp` | Patterns mapping complex transcendental ops to libm function declarations and calls |

## For AI Agents

### Working In This Directory
- Each complex op maps to a libm function: `complex.exp` → `cexpf`/`cexp`, `complex.sqrt` → `csqrtf`/`csqrt`, etc.
- Function declarations are inserted into the module as `func.func @cexpf(!complex.f32) -> !complex.f32`.
- `complex<f32>` uses the `cexpf` variant; `complex<f64>` uses `cexp`.
- The `FunctionCallUtils` infrastructure handles creating function declarations.

## Dependencies
- Source: `mlir/Dialect/Complex`
- Target: `mlir/Dialect/Func` (libm call declarations), libm at link time

<!-- MANUAL: -->
