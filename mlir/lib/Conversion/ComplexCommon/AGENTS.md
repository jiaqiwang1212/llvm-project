<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexCommon Conversion

## Purpose
Provides shared utilities for lowering Complex dialect division operations. The division-to-standard expansion is complex enough (pun intended) to be factored out as shared infrastructure used by both `ComplexToStandard` and `ComplexToLLVM`.

## Key Files
| File | Description |
|------|-------------|
| `DivisionConverter.cpp` | Implements complex division using numerically stable Smith's method |

## For AI Agents

### Working In This Directory
- Complex division `(a+bi)/(c+di)` has precision issues with naive formulas; `DivisionConverter.cpp` uses Smith's algorithm to avoid overflow/underflow.
- This utility is linked into `ComplexToStandard` and `ComplexToLLVM` to avoid code duplication.
- The generated code uses `arith.*` or `llvm.*` ops depending on the consumer.

## Dependencies
- `mlir/Dialect/Complex`, `mlir/Dialect/Arith`

<!-- MANUAL: -->
