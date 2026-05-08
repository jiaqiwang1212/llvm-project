<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToEmitC

## Purpose
Implements the SCF to EmitC lowering pass. Converts structured control flow ops (`scf.for`, `scf.if`, `scf.while`) into EmitC ops (`emitc.for`, `emitc.if`) that emit directly as C control flow constructs.

## Key Files
| File | Description |
|------|-------------|
| `SCFToEmitC.cpp` | Conversion patterns from SCF ops to EmitC control flow ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateSCFToEmitCConversionPatterns()`
- EmitC preserves structured form (no CFG lowering), enabling readable C emission

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- Loop variables and yield values map to EmitC variable declarations

## Dependencies
- Headers: `include/mlir/Conversion/SCFToEmitC/`
- Source dialect: `lib/Dialect/SCF/`
- Target dialect: `lib/Dialect/EmitC/`

<!-- MANUAL: -->
