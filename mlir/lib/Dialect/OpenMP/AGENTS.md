<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OpenMP Dialect

## Purpose
Implements the OpenMP dialect — MLIR representation of OpenMP parallel programming constructs. Models parallel regions (`omp.parallel`), worksharing loops (`omp.wsloop`), tasks (`omp.task`), simd (`omp.simd`), target offload (`omp.target`), reductions, and synchronization primitives.

## Key Files
| File | Description |
|------|-------------|
| `IR/OpenMPDialect.cpp` | Dialect registration, all OpenMP op implementations, clause verifiers |
| `Transforms/MarkDeclareTarget.cpp` | Marks functions as declare target for device compilation |
| `Transforms/OpenMPOffloadPrivatizationPrepare.cpp` | Prepares privatization for target offload regions |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | All OpenMP ops and dialect definition |
| `Transforms/` | Target offload preparation, declare target marking |

## For AI Agents

### Working In This Directory
- OpenMP ops use clause attributes extensively — `omp.parallel` carries `num_threads`, `proc_bind`, `private`, `reduction` clauses as operands or attributes.
- `omp.target` regions are for GPU/device offload; `MarkDeclareTarget.cpp` propagates device-callable annotations through call chains.
- Reductions use `omp.declare_reduction` to define custom reduction operations (init, combiner, atomicUpdate regions).
- The `OpenMPToLLVM` conversion lowers to LLVM OpenMP runtime calls (`__kmpc_*`).
- `omp.private` and `omp.firstprivate` clauses require `OpenMPOffloadPrivatizationPrepare` before target lowering.

### Common Patterns
- Clause verification: each clause has a dedicated verifier helper checking operand count vs. symbol list count.
- Reduction lowering: `omp.declare_reduction` is looked up by symbol reference and its regions are inlined at reduction points.

## Dependencies
- `mlir/Dialect/LLVMIR`, `mlir/Dialect/Func`, `mlir/Dialect/SCF`, `mlir/Dialect/MemRef`

<!-- MANUAL: -->
