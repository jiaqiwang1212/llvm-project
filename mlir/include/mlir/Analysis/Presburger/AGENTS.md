<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Analysis/Presburger/

## Purpose
Presburger arithmetic library — a self-contained implementation of integer linear arithmetic (Presburger arithmetic) used for affine analysis in MLIR. Provides integer relations, polyhedra, simplex method, Barvinok's algorithm for counting lattice points, generating functions, and quasi-polynomials. Used by the Affine dialect for dependence analysis, loop bounds reasoning, and polyhedral transformations.

## Key Files
| File | Description |
|------|-------------|
| `IntegerRelation.h` | `IntegerRelation` — system of linear integer constraints (equalities + inequalities) |
| `PresburgerRelation.h` | `PresburgerRelation` — union of integer relations; supports set operations |
| `PresburgerSpace.h` | `PresburgerSpace` — variable space with domain/range/symbol/local dims |
| `Simplex.h` | `Simplex` — simplex method for integer/rational feasibility and optimization |
| `Matrix.h` | `Matrix<T>` — dense integer/rational matrix with row operations |
| `Fraction.h` | `Fraction` — exact rational arithmetic |
| `LinearTransform.h` | Linear transformation utilities for variable changes |
| `Barvinok.h` | Barvinok's algorithm for counting lattice points in polyhedra |
| `GeneratingFunction.h` | `GeneratingFunction` — rational generating function representation |
| `QuasiPolynomial.h` | `QuasiPolynomial` — quasi-polynomial (periodic coefficients) |
| `PWMAFunction.h` | Piecewise multi-affine function representation |
| `Utils.h` | GCD, LCM, floor/ceiling division, and arithmetic utilities |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- This library has no MLIR IR dependencies — it is pure math and can be used standalone.
- `IntegerRelation` is the core type; build one with `addEquality()`/`addInequality()` calls.
- `Simplex` is the workhorse for checking feasibility and finding optimal solutions.
- The Barvinok/GeneratingFunction infrastructure is used for parametric counting of loop iterations.
- All arithmetic uses `int64_t` coefficients; use `Fraction` for exact rational intermediate results.

### Common Patterns
- Build a polyhedron: `IntegerRelation rel(numDims, numSymbols); rel.addInequality(coeffs);`
- Check emptiness: `rel.isEmpty()` (uses Simplex internally).
- Project out variables: `rel.projectOut(varIdx, numVars)`.

## Dependencies

### Internal
- `mlir/Support/` (LLVM.h for SmallVector, etc.)

### External
- `llvm/ADT/` (SmallVector, ArrayRef)
- `llvm/Support/` (raw_ostream for printing)

<!-- MANUAL: -->
