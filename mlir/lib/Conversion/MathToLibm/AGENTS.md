<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MathToLibm Conversion

## Purpose
Lowers Math dialect ops to libm function calls. Converts transcendental ops (`math.exp`, `math.log`, `math.sin`, `math.cos`, etc.) to calls to the standard C math library, inserting `func.func` declarations for each libm symbol used.

## Key Files
| File | Description |
|------|-------------|
| `MathToLibm.cpp` | Patterns generating libm function declarations and replacing math ops with calls |

## For AI Agents

### Working In This Directory
- Each math op maps to a libm symbol: `math.exp` → `expf`/`exp`, `math.sin` → `sinf`/`sin`, etc.
- Function declarations are inserted once per symbol per module using `FunctionCallUtils::lookupOrCreateFn`.
- For vector types, the pass can optionally scalarize or emit vectorized libm calls (e.g., SVML on x86).
- Use this instead of `MathToLLVM` when targeting CPU without LLVM intrinsic support for specific ops.

## Dependencies
- Source: `mlir/Dialect/Math`
- Target: `mlir/Dialect/Func`, libm at link time

<!-- MANUAL: -->
