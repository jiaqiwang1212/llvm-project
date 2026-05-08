<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToControlFlow

## Purpose
Implements the SCF to ControlFlow lowering pass. Converts structured control flow ops (`scf.for`, `scf.if`, `scf.while`, `scf.parallel`) into unstructured `cf` dialect branch ops (`cf.br`, `cf.cond_br`).

## Key Files
| File | Description |
|------|-------------|
| `SCFToControlFlow.cpp` | Conversion patterns from SCF ops to CF branch ops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateSCFToControlFlowConversionPatterns()`
- Loop carried values become block arguments on the generated loop header block

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- `scf.for` → header block + body block + exit block with `cf.br`/`cf.cond_br`
- `scf.if` → then block + else block + merge block

## Dependencies
- Headers: `include/mlir/Conversion/SCFToControlFlow/`
- Source dialect: `lib/Dialect/SCF/`
- Target dialect: `lib/Dialect/ControlFlow/`

<!-- MANUAL: -->
