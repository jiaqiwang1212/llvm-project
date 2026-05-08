<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SparseTensor Transforms

## Purpose
Transformation passes for the SparseTensor dialect: sparsification (converting dense linalg ops to sparse loops), rewriting, bufferization interface implementations, and GPU offload preparation.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for sparse ops |

## For AI Agents

### Working In This Directory
- The sparsification pass (`createSparsificationPass`) is the core sparse compiler pass
- Implementations live in `lib/Dialect/SparseTensor/Transforms/`

## Dependencies
- Depends on: SparseTensor IR, Linalg dialect, SCF dialect, MemRef dialect

<!-- MANUAL: -->
