<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# VectorToSCF

## Purpose
Implements the Vector to SCF lowering pass. Converts `vector.transfer_read` and `vector.transfer_write` ops into explicit `scf.for` loops with scalar loads/stores, handling out-of-bounds masking and permutation maps.

## Key Files
| File | Description |
|------|-------------|
| `VectorToSCF.cpp` | Conversion patterns expanding vector transfers into SCF loop nests |
| `CMakeLists.txt` | Build target definition |

## For AI Agents

### Working In This Directory
- Conversion patterns use `OpRewritePattern<OpTy>` (not full dialect conversion)
- Register patterns in `populateVectorToSCFConversionPatterns()`
- Pass options control full vs. partial unrolling and target rank for progressive lowering

### Common Patterns
- `matchAndRewrite()` implements the actual rewrite logic
- `vector.transfer_read` → `scf.for` loop + `memref.load` + `vector.insert`
- Masking logic generates `scf.if` guards for out-of-bounds dimensions
- Can be applied progressively: rank-N → rank-(N-1) in multiple passes

## Dependencies
- Headers: `include/mlir/Conversion/VectorToSCF/`
- Source dialect: `lib/Dialect/Vector/`
- Target dialects: `lib/Dialect/SCF/`, `lib/Dialect/MemRef/`

<!-- MANUAL: -->
