<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SCFToOpenMP

## Purpose
Implements the SCF to OpenMP lowering pass. Converts `scf.parallel` loops into `omp.parallel` + `omp.wsloop` constructs for multi-threaded CPU execution via the OpenMP runtime.

## Key Files
| File | Description |
|------|-------------|
| `SCFToOpenMP.cpp` | Conversion patterns mapping scf.parallel to OpenMP worksharing loops |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpConversionPattern<OpTy>`
- Register patterns in `populateSCFToOpenMPConversionPatterns()`
- Reduction variables in `scf.parallel` become OpenMP reduction clauses

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- Use `adaptor.getOperands()` to access already-converted operands
- `scf.reduce` ops inside the parallel body map to OpenMP reduction combiners

## Dependencies
- Headers: `include/mlir/Conversion/SCFToOpenMP/`
- Source dialect: `lib/Dialect/SCF/`
- Target dialect: `lib/Dialect/OpenMP/`

<!-- MANUAL: -->
