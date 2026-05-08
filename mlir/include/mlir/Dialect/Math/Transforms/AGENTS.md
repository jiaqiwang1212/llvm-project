<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Math Transforms

## Purpose
Transformation passes for the Math dialect: polynomial approximations for transcendental functions (exp, erf, tanh), and lowering to scalar function calls.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Approximation.h` | Polynomial approximation patterns for math ops |

## For AI Agents

### Working In This Directory
- `Approximation.h` provides `populateMathPolynomialApproximationPatterns` for fast SW approximations
- Implementations live in `lib/Dialect/Math/Transforms/`

## Dependencies
- Depends on: Math IR, Arith dialect, Vector dialect

<!-- MANUAL: -->
