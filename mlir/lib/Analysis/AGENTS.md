<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Analysis

## Purpose
Implements standalone IR analyses that are dialect-independent and reusable across passes. Covers alias analysis, dataflow analysis framework, liveness, slice analysis, loop information, data layout analysis, and Presburger arithmetic (the mathematical foundation for affine analysis).

## Key Files
| File | Description |
|------|-------------|
| `AliasAnalysis.cpp` | Top-level `AliasAnalysis` class: composable alias query dispatcher over registered `AliasAnalysisImpl` backends |
| `CallGraph.cpp` | `CallGraph` construction and `CallGraphSCC` iteration for inter-procedural analyses |
| `CFGLoopInfo.cpp` | `CFGLoopInfo`: natural loop detection over CFG regions using dominator trees |
| `DataFlowFramework.cpp` | Generic forward/backward dataflow analysis framework: `DataFlowSolver`, `DataFlowAnalysis`, `ProgramPoint`, lattice infrastructure |
| `DataLayoutAnalysis.cpp` | `DataLayoutAnalysis`: collects data layout specs from enclosing ops for target-aware queries |
| `FlatLinearValueConstraints.cpp` | `FlatLinearValueConstraints`: integer linear arithmetic constraint system over SSA values (extends Presburger) |
| `Liveness.cpp` | `Liveness` and `LivenessBlockInfo`: SSA liveness (live-in, live-out) for each block |
| `SliceAnalysis.cpp` | Forward/backward use-def slice computation (`getForwardSlice`, `getBackwardSlice`) |
| `SliceWalk.cpp` | Value slice walker utilities used by `SliceAnalysis` |
| `TopologicalSortUtils.cpp` | Topological sort and SCCs for op and block orderings |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AliasAnalysis/` | Local alias analysis implementation (see `AliasAnalysis/AGENTS.md`) |
| `DataFlow/` | Built-in dataflow analyses: constant propagation, dead code, integer ranges, liveness, sparse/dense frameworks (see `DataFlow/AGENTS.md`) |
| `Presburger/` | Presburger arithmetic library: integer sets, polyhedra, parametric polytopes, Barvinok's algorithm (see `Presburger/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `DataFlowFramework.cpp` is the extensibility point for new analyses; implement `DataFlowAnalysis` subclasses and register them with the `DataFlowSolver`.
- `FlatLinearValueConstraints.cpp` depends heavily on `Presburger/`; changes to `IntegerRelation` will ripple here.
- Analyses in this directory must not modify the IR; they are read-only query engines.

### Common Patterns
- Analyses are instantiated and queried via the `DataFlowSolver` or directly as standalone objects (e.g., `Liveness liveness(funcOp)`).
- Alias analysis implementations register themselves with `AliasAnalysis::addAnalysisImplementation<T>()`.
- Slice queries return `SetVector<Operation*>` or `SetVector<Value>`.

## Dependencies

### Internal
- `mlir/lib/IR/` — all core IR types
- `mlir/lib/Interfaces/` — `CallInterfaces`, `ControlFlowInterfaces`, `SideEffectInterfaces`

### External
- `llvm/lib/Support` — `llvm::DenseSet`, `llvm::SetVector`, graph utilities

<!-- MANUAL: -->
