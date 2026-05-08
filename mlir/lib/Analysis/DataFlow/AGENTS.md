<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Analysis/DataFlow

## Purpose
Provides ready-to-use dataflow analyses built on top of the `DataFlowFramework`. Includes constant propagation, dead-code analysis, integer range analysis, liveness analysis, and both sparse and dense analysis base classes.

## Key Files
| File | Description |
|------|-------------|
| `ConstantPropagationAnalysis.cpp` | Sparse constant propagation: folds ops to constants using `OpFoldResult` and propagates values through control flow |
| `DeadCodeAnalysis.cpp` | Executable-block and live-value analysis: marks blocks/branch edges as live/dead to enable DCE |
| `DenseAnalysis.cpp` | Dense dataflow base classes: `AbstractDenseLattice`, `AbstractDenseForwardDataFlowAnalysis`, `AbstractDenseBackwardDataFlowAnalysis` for analyses that track per-program-point state over all memory |
| `IntegerRangeAnalysis.cpp` | Integer range inference: propagates `ConstantIntRanges` through arithmetic ops using `InferIntRangeInterface` |
| `LivenessAnalysis.cpp` | Dataflow-based liveness: computes live-in/live-out sets at each program point |
| `SparseAnalysis.cpp` | Sparse dataflow base classes: `AbstractSparseLattice`, `AbstractSparseForwardDataFlowAnalysis`, `AbstractSparseBackwardDataFlowAnalysis` for per-value lattice analyses |
| `StridedMetadataRangeAnalysis.cpp` | Range analysis for memref strided metadata (offsets, sizes, strides) |

## For AI Agents

### Working In This Directory
- New dataflow analyses subclass either `AbstractSparseForwardDataFlowAnalysis` (per-value lattices) or `AbstractDenseForwardDataFlowAnalysis` (per-point state over memory).
- All analyses are driven by `DataFlowSolver::run()` from `DataFlowFramework.cpp`.
- `DeadCodeAnalysis` is a prerequisite for many other analyses since it marks which blocks are executable.

### Common Patterns
- Lattice types must implement `join()` (meet-over-all-paths) and `isAtFixpoint()`.
- Transfer functions are implemented in `visitOperation()` and `visitControlFlowArguments()`.

## Dependencies

### Internal
- `mlir/lib/Analysis/DataFlowFramework.cpp` — solver and program point model
- `mlir/lib/IR/` — core IR
- `mlir/lib/Interfaces/InferIntRangeInterface.cpp`, `ControlFlowInterfaces.cpp`

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
