<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Analysis/Presburger

## Purpose
Self-contained Presburger arithmetic library: exact integer linear arithmetic over parametric integer sets and relations. Used as the mathematical foundation for the Affine dialect's dependence analysis, loop transformation legality checks, and polyhedral scheduling. Implements operations over integer polyhedra including union, intersection, complement, projection (Fourier-Motzkin / Omega), and exact integer counting (Barvinok's algorithm).

## Key Files
| File | Description |
|------|-------------|
| `IntegerRelation.cpp` | `IntegerRelation`: the core constraint system (equalities + inequalities over integer variables); supports Gaussian elimination, Fourier-Motzkin elimination, and normalization |
| `PresburgerRelation.cpp` | `PresburgerRelation`: union of `IntegerRelation`s representing Presburger sets/relations with set operations (union, intersect, subtract, complement) |
| `PresburgerSpace.cpp` | `PresburgerSpace`: variable space descriptor (domain, range, symbol, local variable counts) |
| `Matrix.cpp` | `Matrix<T>` and `IntMatrix`: dense integer matrix with row/column operations, GCD reduction, Hermite Normal Form |
| `LinearTransform.cpp` | `LinearTransform`: invertible integer linear transformation (used for unimodular transforms in loop tiling/skewing) |
| `Simplex.cpp` | `Simplex` and `LexSimplex`: rational/integer simplex solver used for feasibility and optimization over integer polyhedra |
| `PWMAFunction.cpp` | `PWMAFunction`: piecewise multi-affine function over Presburger sets (used for representing loop bounds) |
| `QuasiPolynomial.cpp` | `QuasiPolynomial`: polynomial with periodic coefficients, result of Barvinok counting |
| `Barvinok.cpp` | Barvinok's algorithm: exact integer point counting in parametric polytopes; generates `QuasiPolynomial` |
| `Utils.cpp` | Utility functions: GCD computation, extended Euclidean algorithm, modular arithmetic helpers |

## For AI Agents

### Working In This Directory
- This library has no MLIR IR dependencies — it operates purely on integer matrices and constraint systems.
- `IntegerRelation` and `Simplex` are the two heaviest files; profile before optimizing.
- The Barvinok implementation is research-grade; consult the paper ("Counting Lattice Points in Polyhedra" by Barvinok & Pommersheim) before modifying.
- Tests live in `mlir/unittests/Analysis/Presburger/`.

### Common Patterns
- Variables are tracked positionally in `PresburgerSpace`; always update the space when adding/removing variables.
- Integer arithmetic uses `llvm::APInt` or plain `int64_t` depending on operation; overflow checks are caller-responsibility.

## Dependencies

### Internal
- No mlir/lib/ dependencies; fully self-contained
- Used by `mlir/lib/Analysis/FlatLinearValueConstraints.cpp` and `mlir/lib/Dialect/Affine/`

### External
- `llvm/lib/Support` — `llvm::APInt`, `llvm::SmallVector`, `llvm::raw_ostream`

<!-- MANUAL: -->
