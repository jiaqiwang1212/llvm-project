<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor Pipelines

## Purpose
Pre-built pass pipeline declarations for the complete sparse tensor compilation workflow: sparsification, bufferization, and lowering to dense loops.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pipeline registration and option struct declarations |

## For AI Agents

### Working In This Directory
- Use `buildSparseCompiler` pipeline for end-to-end sparse tensor compilation
- Options control parallelization, vectorization width, and enable/disable GPU offload

## Dependencies
- Depends on: SparseTensor Transforms, Linalg, Bufferization, SCF pipelines

<!-- MANUAL: -->
