<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Math Dialect

## Purpose
Provides mathematical functions (exp, log, sin, cos, sqrt, pow, erf, etc.) on scalar and vector floating-point types. Complementary to the Arith dialect for transcendental and special functions.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Approximation and lowering passes (see `Transforms/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Op names follow `math.*` convention (e.g., `math.exp`, `math.log`, `math.sqrt`)
- Lowered to libm, LLVM intrinsics, NVVM, or polynomial approximations

### Common Patterns
- Ops are type-polymorphic over `f16`, `f32`, `f64`, and vector variants
- Implements `ArithFastMathInterface` for fast-math flag propagation

## Dependencies
- Depends on: Arith dialect (for lowering to scalar ops)

<!-- MANUAL: -->
