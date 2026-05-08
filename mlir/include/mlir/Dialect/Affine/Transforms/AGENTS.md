<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine Transforms

## Purpose
Transformation pass declarations for the Affine dialect: loop tiling, unrolling, vectorization, loop invariant code motion, normalization, and more.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | C++ pass registration declarations |
| `Passes.td` | ODS pass definitions (generates registration boilerplate) |
| `Transforms.h` | Public C++ APIs for individual transformations |

## For AI Agents

### Working In This Directory
- Add new passes in `Passes.td` with options, then declare the entry point in `Passes.h`
- Implementations live in `lib/Dialect/Affine/Transforms/`

### Common Patterns
- Pass names follow `createAffine*Pass()` convention
- Transformation functions accept `func::FuncOp` or `AffineForOp` as root operations

## Dependencies
- Depends on: Affine IR, MemRef dialect, Vector dialect (for vectorization)

<!-- MANUAL: -->
