<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TosaToSCF

## Purpose
Implements the TOSA to SCF lowering pass. Converts TOSA control flow ops (`tosa.if`, `tosa.while_loop`) into SCF dialect structured control flow ops (`scf.if`, `scf.while`).

## Key Files
| File | Description |
|------|-------------|
| `TosaToSCF.cpp` | Conversion patterns from TOSA control flow to SCF ops |
| `TosaToSCFPass.cpp` | Pass registration and pipeline entry point |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateTosaToSCFConversionPatterns()`
- TOSA uses tensor-typed loop-carried values; SCF preserves these semantics

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `tosa.while_loop` → `scf.while` with `scf.condition` terminator

## Dependencies
- Headers: `include/mlir/Conversion/TosaToSCF/`
- Source dialect: `lib/Dialect/Tosa/`
- Target dialect: `lib/Dialect/SCF/`

<!-- MANUAL: -->
