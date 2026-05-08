<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ShapeToStandard

## Purpose
Implements the Shape to Standard lowering pass. Converts `shape` dialect operations (shape computation, broadcasting, constraint checking) into standard dialect ops (`arith`, `tensor`, `scf`) for concrete execution.

## Key Files
| File | Description |
|------|-------------|
| `ShapeToStandard.cpp` | Core conversion patterns for shape ops to standard dialect |
| `ShapeToStandard.td` | TableGen definitions for shape-to-standard conversion patterns |
| `ConvertShapeConstraints.cpp` | Patterns for lowering shape constraint assertions |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateShapeToStandardConversionPatterns()`
- Shape `!shape.shape` type lowers to `tensor<Nxindex>` or `!shape.witness`

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Shape constraint ops lower to `cf.assert` or are eliminated when provably valid
- `.td` patterns handle simple structural rewrites; `.cpp` handles complex ones

## Dependencies
- Headers: `include/mlir/Conversion/ShapeToStandard/`
- Source dialect: `lib/Dialect/Shape/`
- Target dialects: `lib/Dialect/Arith/`, `lib/Dialect/SCF/`, `lib/Dialect/Tensor/`

<!-- MANUAL: -->
