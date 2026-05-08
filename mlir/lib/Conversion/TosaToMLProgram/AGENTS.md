<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToMLProgram

## Purpose
Implements the TOSA to MLProgram lowering pass. Converts TOSA variable ops (`tosa.variable`, `tosa.variable_read`, `tosa.variable_write`) into `ml_program` dialect global variable ops.

## Key Files
| File | Description |
|------|-------------|
| `TosaToMLProgram.cpp` | Conversion patterns from TOSA variable ops to ml_program globals |
| `TosaToMLProgramPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTosaToMLProgramConversionPatterns()`
- TOSA variable semantics (mutable named tensors) map to `ml_program.global` + load/store

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `tosa.variable` → `ml_program.global`, reads/writes → `ml_program.global_load/store`

## Dependencies
- Headers: `include/mlir/Conversion/TosaToMLProgram/`
- Source dialect: `lib/Dialect/Tosa/`
- Target dialect: `lib/Dialect/MLProgram/`

<!-- MANUAL: -->
