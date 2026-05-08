<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TensorToLinalg

## Purpose
Implements the Tensor to Linalg lowering pass. Converts `tensor` dialect ops (pad, scatter, etc.) into `linalg` generic or named ops for further lowering through the Linalg pipeline.

## Key Files
| File | Description |
|------|-------------|
| `TensorToLinalg.cpp` | Core conversion patterns from tensor ops to linalg ops |
| `TensorToLinalgPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTensorToLinalgConversionPatterns()`
- Output tensor types are preserved; Linalg ops operate on tensors (not buffers) at this stage

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `tensor.pad` → `linalg.generic` with boundary fill logic

## Dependencies
- Headers: `include/mlir/Conversion/TensorToLinalg/`
- Source dialect: `lib/Dialect/Tensor/`
- Target dialect: `lib/Dialect/Linalg/`

<!-- MANUAL: -->
