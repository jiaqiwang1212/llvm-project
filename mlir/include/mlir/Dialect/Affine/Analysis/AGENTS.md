<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Affine Analysis

## Purpose
Analysis passes and utilities for affine programs: dependence analysis, loop analysis, affine structures (polyhedra), and nested-matcher pattern matching over affine loops.

## Key Files
| File | Description |
|------|-------------|
| `AffineAnalysis.h` | Dependence analysis for affine memory ops |
| `AffineStructures.h` | FlatAffineValueConstraints and polyhedral data structures |
| `LoopAnalysis.h` | Loop trip-count, IV bounds, and related queries |
| `NestedMatcher.h` | Recursive loop-nest pattern matcher |
| `Utils.h` | Shared analysis helpers |

## For AI Agents

### Working In This Directory
- Header-only interfaces; implementations live in `lib/Dialect/Affine/Analysis/`
- `AffineStructures.h` wraps the Presburger library for constraint systems

### Common Patterns
- Use `getComputedBounds` / `getBound` from `LoopAnalysis.h` when querying trip counts
- `FlatAffineValueConstraints` is the main polyhedral constraint-set class

## Dependencies
- Depends on: MLIR Presburger library, Affine IR types

<!-- MANUAL: -->
