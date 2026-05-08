<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AffineToStandard Conversion

## Purpose
Lowers the Affine dialect to standard loop and arithmetic dialects. Converts `affine.for` → `scf.for`, `affine.if` → `scf.if`, `affine.load`/`affine.store` → `memref.load`/`memref.store`, and `affine.apply` → `arith` index arithmetic.

## Key Files
| File | Description |
|------|-------------|
| `AffineToStandard.cpp` | All conversion patterns: affine loops, conditionals, memory ops, apply |

## For AI Agents

### Working In This Directory
- Affine map expressions in `affine.apply` are expanded to `arith.addi`/`arith.muli`/`arith.floordivsi` sequences.
- Loop bound affine maps with multiple results become `arith.maxsi`/`arith.minsi` over all results.
- `affine.parallel` lowers to `scf.parallel`.
- This is typically the first step after affine-level transformations (tiling, fusion) before further lowering.

## Dependencies
- Source: `mlir/Dialect/Affine`
- Target: `mlir/Dialect/SCF`, `mlir/Dialect/MemRef`, `mlir/Dialect/Arith`

<!-- MANUAL: -->
