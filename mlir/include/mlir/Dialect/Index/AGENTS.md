<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Index Dialect

## Purpose
Provides arithmetic and comparison operations on the MLIR `index` type (a platform-width integer). Replaces ad-hoc use of `arith` ops on index-typed values with dedicated, semantically correct ops.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, attribute, and dialect definitions (see `IR/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `index` type is target-width; do not assume 32- or 64-bit
- Op names follow `index.*` convention (e.g., `index.add`, `index.ceildivs`, `index.cmp`)

### Common Patterns
- Lowered to `arith` ops or LLVM via IndexToLLVM/IndexToSPIRV conversions
- Implements `ValueBoundsOpInterface` for static analysis

## Dependencies
- No dialect dependencies (foundational)

<!-- MANUAL: -->
