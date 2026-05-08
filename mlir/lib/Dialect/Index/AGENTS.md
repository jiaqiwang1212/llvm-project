<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Index Dialect

## Purpose
Implements the Index dialect — a portable integer dialect for index computations that abstracts over pointer-width integers. Provides `index.add`, `index.mul`, `index.ceildivs`, etc. on the `index` type, which is target-width (32 or 64 bits).

## Key Files
| File | Description |
|------|-------------|
| `IR/IndexDialect.cpp` | Dialect registration |
| `IR/IndexOps.cpp` | All index arithmetic op implementations and folders |
| `IR/IndexAttrs.cpp` | `index.bool` and `index.integer` attribute implementations |
| `IR/InferIntRangeInterfaceImpls.cpp` | Integer range inference for all index ops |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Ops, attributes, integer range inference |

## For AI Agents

### Working In This Directory
- The `index` type is not `i32` or `i64` — it is target-width and must be treated as opaque until lowered via `IndexToLLVM` or `IndexToSPIRV`.
- `InferIntRangeInterfaceImpls.cpp` enables `scf.for` bound inference and other loop analyses.
- Op folders perform compile-time evaluation when operands are `index.constant` attrs.
- `IndexCastOp` provides casts between `index` and `iN` types.

### Common Patterns
- Constant folding: `index.constant` produces an `IntegerAttr` with `IndexType`; folders use `APInt` arithmetic.
- Integer range inference: implement `inferResultRanges()` using closed `ConstantIntRanges`.

## Dependencies
- `mlir/IR`, `mlir/Interfaces/InferIntRangeInterface`

<!-- MANUAL: -->
