<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Complex Dialect

## Purpose
Implements the Complex dialect, providing complex number arithmetic operations (`complex.add`, `complex.mul`, `complex.abs`, `complex.exp`, etc.) over `complex<f32>` and `complex<f64>` types.

## Key Files
| File | Description |
|------|-------------|
| `IR/ComplexDialect.cpp` | Dialect registration |
| `IR/ComplexOps.cpp` | Op implementations and verifiers for all complex arithmetic ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Ops and dialect definition |

## For AI Agents

### Working In This Directory
- Complex ops are strongly typed — operands must be `complex<T>` and results are `complex<T>` or `T` (for real-valued ops like `abs`).
- There are no Transforms in this directory; lowering is handled by `ComplexToStandard`, `ComplexToLLVM`, and `ComplexToLibm` conversions.
- Op folder methods perform constant folding via `std::complex<APFloat>` arithmetic.

### Common Patterns
- Verifiers check that operand element type matches result element type.
- Folders extract `complex<float>` constant attributes and compute results at compile time.

## Dependencies
- `mlir/IR`, `mlir/Dialect/Arith` (used in lowering)

<!-- MANUAL: -->
