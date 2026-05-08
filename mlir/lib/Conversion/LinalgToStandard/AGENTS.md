<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LinalgToStandard Conversion

## Purpose
Lowers Linalg named ops to standard library function calls. Converts `linalg.matmul`, `linalg.matvec`, `linalg.dot` etc. to calls to an external BLAS/linalg runtime library rather than generating inline loop code.

## Key Files
| File | Description |
|------|-------------|
| `LinalgToStandard.cpp` | Conversion patterns: linalg named ops → func.call to runtime library functions |

## For AI Agents

### Working In This Directory
- Named ops like `linalg.matmul` are converted to calls like `mlir_linalg_fill_f32_viewsxsxf32_viewf32(...)`.
- Function declarations are synthesized based on operand types and shapes.
- This path is an alternative to `Linalg.Loops` (inline loop generation) and vectorization — used when a pre-compiled library is preferred.
- The library function naming convention encodes element type and memref rank into the symbol name.

## Dependencies
- Source: `mlir/Dialect/Linalg`
- Target: `mlir/Dialect/Func` (library call declarations), `mlir/Dialect/MemRef`

<!-- MANUAL: -->
