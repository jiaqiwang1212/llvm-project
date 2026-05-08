<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ComplexToStandard Conversion

## Purpose
Lowers Complex dialect ops to standard Arith and Math dialect ops. Converts complex arithmetic by decomposing into real/imaginary component operations using `arith.*` and `math.*` ops — a target-independent lowering path.

## Key Files
| File | Description |
|------|-------------|
| `ComplexToStandard.cpp` | Patterns decomposing complex ops into arith/math component operations |

## For AI Agents

### Working In This Directory
- The standard lowering path: `complex.add` → two `arith.addf`, `complex.mul` → `(ac-bd) + (ad+bc)i` using four `arith.mulf` and two `arith.addf`/`arith.subf`.
- Transcendental ops (`complex.exp`, `complex.log`) expand using `math.*` ops.
- Division uses `ComplexCommon/DivisionConverter` for numerical stability.
- Prefer this over `ComplexToLibm` when targeting platforms without libm or when inlining is needed.

## Dependencies
- Source: `mlir/Dialect/Complex`
- Target: `mlir/Dialect/Arith`, `mlir/Dialect/Math`
- Utility: `mlir/Conversion/ComplexCommon`

<!-- MANUAL: -->
