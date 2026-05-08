<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor Dialect

## Purpose
Provides a sparse tensor abstraction: sparse encoding attributes that annotate tensor types with storage formats (CSR, CSC, COO, etc.) and ops for sparse tensor manipulation, iteration, and code generation. Enables auto-parallelization and vectorization of sparse computations.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, type, interface, and attribute definitions (see `IR/AGENTS.md`) |
| `Pipelines/` | Pre-built sparse compilation pipelines (see `Pipelines/AGENTS.md`) |
| `TransformOps/` | Transform dialect extension ops (see `TransformOps/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Sparse compiler utilities (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Sparse encoding is attached to tensor types as `#sparse_tensor.encoding<{...}>` attribute
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- `sparse_tensor.compress` / `sparse_tensor.expand` / `sparse_tensor.foreach` are core iteration ops
- Sparse Linalg ops are lowered via sparsification passes to dense loops over sparse storage

## Dependencies
- Depends on: Tensor dialect, Linalg dialect, SCF dialect, MemRef dialect

<!-- MANUAL: -->
