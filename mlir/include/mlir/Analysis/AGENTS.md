<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Analysis/

## Purpose
Analysis framework headers for MLIR. Provides dialect-independent analyses including alias analysis, call graph construction, dataflow analysis framework, data layout queries, liveness, Presburger arithmetic (integer linear programming), slice analysis, and topological sort utilities. These analyses are consumed by optimization passes across many dialects.

## Key Files
| File | Description |
|------|-------------|
| `AliasAnalysis.h` | Top-level alias analysis interface and result types |
| `CallGraph.h` | Call graph construction and traversal |
| `CFGLoopInfo.h` | Loop information for CFG-based regions |
| `DataFlowFramework.h` | Generic dataflow analysis lattice framework |
| `DataLayoutAnalysis.h` | Analysis for querying data layout information |
| `FlatLinearValueConstraints.h` | Flat representation of linear arithmetic constraints on SSA values |
| `Liveness.h` | Liveness analysis (live-in, live-out sets per block) |
| `SliceAnalysis.h` | Forward/backward slice computation |
| `SliceWalk.h` | Slice traversal utilities |
| `SymbolTableAnalysis.h` | Cached symbol table analysis |
| `TopologicalSortUtils.h` | Topological ordering of ops and blocks |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `AliasAnalysis/` | Local alias analysis implementation (see `AliasAnalysis/AGENTS.md`) |
| `DataFlow/` | Concrete dataflow analyses: constants, dead code, integer ranges, liveness (see `DataFlow/AGENTS.md`) |
| `Presburger/` | Presburger arithmetic library: integer relations, polyhedra, simplex (see `Presburger/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Analyses are typically implemented as `DataFlowAnalysis` subclasses using `DataFlowFramework.h`.
- Alias analysis results use a three-valued `AliasResult` (NoAlias, MayAlias, MustAlias).
- Presburger analyses are standalone math utilities with no MLIR IR dependency.
- Liveness and slice analyses operate on `Operation *` and `Value` directly.

### Common Patterns
- Dataflow analyses define a `Lattice<T>` and override `visitOperation` / `visitBlock`.
- `getOrCreateFor<MyLattice>(point, value)` fetches lattice state at a program point.
- Slice analysis: `getForwardSlice(op, &slice, config)` / `getBackwardSlice(op, &slice, config)`.

## Dependencies

### Internal
- `mlir/IR/` (Operation, Value, Block, Region)
- `mlir/Interfaces/` (SideEffectInterfaces, CallInterfaces)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/ADT/` (DenseMap, SetVector, SmallVector)

<!-- MANUAL: -->
