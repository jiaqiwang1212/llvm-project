<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor Utils

## Purpose
Utility functions for the sparse tensor compiler: the Merger class that drives sparse iteration space merging during sparsification.

## Key Files
| File | Description |
|------|-------------|
| `Merger.h` | Merger class for sparse iteration lattice computation |

## For AI Agents

### Working In This Directory
- `Merger` computes the merged iteration space of multiple sparse/dense operands for `linalg.generic`
- Used internally by the sparsification pass

## Dependencies
- Depends on: SparseTensor IR, Linalg IR

<!-- MANUAL: -->
