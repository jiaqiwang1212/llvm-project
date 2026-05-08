<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Analysis/DataFlow/

## Purpose
Concrete dataflow analysis implementations built on `DataFlowFramework.h`. Provides constant propagation, dead code analysis, integer range analysis, liveness, sparse/dense analysis base classes, and strided metadata range analysis. These analyses are used by optimization passes for value inference and dead code elimination.

## Key Files
| File | Description |
|------|-------------|
| `ConstantPropagationAnalysis.h` | Sparse constant propagation lattice and analysis |
| `DeadCodeAnalysis.h` | Dead code analysis: determines which ops/blocks are unreachable |
| `DenseAnalysis.h` | `AbstractDenseLattice` and `DenseDataFlowAnalysis<>` base for dense (whole-program-state) analyses |
| `IntegerRangeAnalysis.h` | Integer range inference using `InferIntRangeInterface` |
| `LivenessAnalysis.h` | Dataflow liveness: which values are live at each program point |
| `SparseAnalysis.h` | `AbstractSparseLattice` and `SparseDataFlowAnalysis<>` base for per-value analyses |
| `StridedMetadataRangeAnalysis.h` | Range analysis for strided memref metadata values |
| `Utils.h` | Utility functions for dataflow analyses |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Analyses derive from either `SparseDataFlowAnalysis<LatticeType>` (per-SSA-value lattice) or `DenseDataFlowAnalysis<LatticeType>` (per-program-point state).
- Run analyses via `DataFlowSolver`: `solver.load<MyAnalysis>(); solver.initializeAndRun(op);`.
- `ConstantPropagationAnalysis` and `DeadCodeAnalysis` are typically run together.
- `IntegerRangeAnalysis` requires `InferIntRangeInterface` to be implemented by ops.

### Common Patterns
- Lattice definition: `struct MyLattice : Lattice<MyValue> { ... };`
- Analysis: `struct MyAnalysis : SparseDataFlowAnalysis<MyLattice> { void visitOperation(...) override; };`
- Query result: `solver.lookupState<MyLattice>(value)->getValue()`.

## Dependencies

### Internal
- `mlir/Analysis/DataFlowFramework.h` (solver and lattice base classes)
- `mlir/IR/` (Value, Operation, Block)
- `mlir/Interfaces/` (InferIntRangeInterface, ControlFlowInterfaces)

### External
- `llvm/ADT/`

<!-- MANUAL: -->
