<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Complex Dialect

## Purpose
Provides arithmetic operations on complex numbers (add, mul, div, abs, re, im, etc.) over MLIR's built-in `complex<T>` type.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, attribute, and dialect definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Op names follow `complex.*` convention
- Lowered to libm calls (ComplexToLibm), standard scalar ops (ComplexToStandard), or LLVM (ComplexToLLVM)

### Common Patterns
- Operations are generic over `complex<f32>` and `complex<f64>`

## Dependencies
- Depends on: Arith dialect (for scalar sub-operations during lowering)

<!-- MANUAL: -->
