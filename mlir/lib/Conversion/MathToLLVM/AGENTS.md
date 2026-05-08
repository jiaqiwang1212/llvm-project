<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToLLVM Conversion

## Purpose
Lowers Math dialect ops to LLVM IR intrinsics. Converts `math.sqrt`, `math.fma`, `math.absf`, `math.copysign`, `math.floor`, `math.ceil`, `math.round` etc. to corresponding `llvm.intr.*` ops.

## Key Files
| File | Description |
|------|-------------|
| `MathToLLVM.cpp` | Patterns mapping math ops to LLVM intrinsics |

## For AI Agents

### Working In This Directory
- `math.sqrt f32` → `llvm.intr.sqrt(f32)`, `math.fma` → `llvm.intr.fma`, `math.absf` → `llvm.intr.fabs`.
- `math.exp` / `math.log` / `math.sin` / `math.cos` do NOT have LLVM intrinsics — use `MathToLibm` or `PolynomialApproximation` for those.
- Operates on both scalar and vector types; vector versions use the same intrinsic names (LLVM intrinsics are overloaded).

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/LLVMIR`
- Utility: `mlir/Conversion/LLVMCommon`

<!-- MANUAL: -->
