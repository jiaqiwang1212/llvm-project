<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToTensor

## Purpose
Implements the TOSA to Tensor lowering pass. Converts TOSA reshape, slice, pad, and concatenation ops into `tensor` dialect ops (`tensor.reshape`, `tensor.extract_slice`, `tensor.pad`, `tensor.insert_slice`).

## Key Files
| File | Description |
|------|-------------|
| `TosaToTensor.cpp` | Conversion patterns from TOSA shape-manipulation ops to tensor ops |
| `TosaToTensorPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTosaToTensorConversionPatterns()`
- Static shape information from TOSA is preserved in the `tensor` op attributes

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `tosa.reshape` → `tensor.collapse_shape` + `tensor.expand_shape` as needed

## Dependencies
- Headers: `include/mlir/Conversion/TosaToTensor/`
- Source dialect: `lib/Dialect/Tosa/`
- Target dialect: `lib/Dialect/Tensor/`

<!-- MANUAL: -->
