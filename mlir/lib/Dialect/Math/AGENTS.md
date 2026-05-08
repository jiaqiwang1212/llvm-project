<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Math Dialect

## Purpose
Implements the Math dialect — transcendental and mathematical functions (`math.exp`, `math.log`, `math.sin`, `math.sqrt`, `math.powf`, `math.fma`, etc.) over scalar and vector types. Provides a portable representation that can be lowered to libm calls, LLVM intrinsics, or polynomial approximations.

## Key Files
| File | Description |
|------|-------------|
| `IR/MathDialect.cpp` | Dialect registration |
| `IR/MathOps.cpp` | All math op implementations and folders |
| `Transforms/AlgebraicSimplification.cpp` | Algebraic identities: `log(exp(x)) → x`, `pow(x,2) → x*x`, etc. |
| `Transforms/ExpandOps.cpp` | Expands complex ops into simpler sequences (e.g., `math.powf` → loops) |
| `Transforms/ExtendToSupportedTypes.cpp` | Promotes narrow float types to supported widths before math ops |
| `Transforms/PolynomialApproximation.cpp` | Replaces transcendental ops with fast polynomial approximations |
| `Transforms/SincosFusion.cpp` | Fuses `sin`+`cos` on the same argument into a single `sincos` call |
| `Transforms/UpliftToFMA.cpp` | Detects `a*b+c` patterns and rewrites to `math.fma` |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Math ops |
| `Transforms/` | Algebraic simplification, expansion, polynomial approximation, FMA uplift |

## For AI Agents

### Working In This Directory
- Math ops are polymorphic: they accept `f16`, `f32`, `f64`, and `vector<NxfX>` types.
- `PolynomialApproximation.cpp` generates fast (but approximate) implementations using Horner's method — suitable for GPU kernels.
- `ExtendToSupportedTypes.cpp` is needed when a target lacks native `f16` math; it promotes to `f32`, computes, then truncates.
- `SincosFusion.cpp` is important for GPU targets where a combined `sincos` instruction exists.
- Lowering options: `MathToLibm`, `MathToLLVM`, `MathToNVVM`, `MathToROCDL` (all in Conversion/).

### Common Patterns
- Folders: implement `fold()` using `APFloat` arithmetic for constant inputs.
- Algebraic simplification: pattern-match op trees with `matchPattern` / `m_Op<>`.
- Polynomial approximation: use `PolynomialApproximation` pass before lowering to LLVM for throughput-critical GPU code.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/Dialect/Vector`, `mlir/Dialect/LLVMIR` (for lowering)

<!-- MANUAL: -->
