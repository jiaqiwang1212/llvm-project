<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToLinalg

## Purpose
Implements the TOSA to Linalg lowering passes. Converts TOSA element-wise, reduction, and named convolution/matmul ops into `linalg.generic` and named Linalg ops for further lowering.

## Key Files
| File | Description |
|------|-------------|
| `TosaToLinalg.cpp` | Patterns for element-wise and reduction TOSA ops to linalg.generic |
| `TosaToLinalgPass.cpp` | Pass registration for the main TosaToLinalg pass |
| `TosaToLinalgNamed.cpp` | Patterns for TOSA named ops (conv2d, matmul) to linalg named ops |
| `TosaToLinalgNamedPass.cpp` | Pass registration for the TosaToLinalgNamed pass |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register element-wise patterns in `populateTosaToLinalgConversionPatterns()`
- Register named op patterns in `populateTosaToLinalgNamedConversionPatterns()`
- Two separate passes: one for generic element-wise ops, one for named compute ops

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Element-wise TOSA ops → `linalg.generic` with appropriate indexing maps
- `tosa.conv2d` / `tosa.matmul` → `linalg.conv_2d_*` / `linalg.matmul`

## Dependencies
- Headers: `include/mlir/Conversion/TosaToLinalg/`
- Source dialect: `lib/Dialect/Tosa/`
- Target dialect: `lib/Dialect/Linalg/`

<!-- MANUAL: -->
