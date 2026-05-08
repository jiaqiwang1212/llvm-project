<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Transforms

## Purpose
Implements dialect-independent transformation passes that operate on arbitrary MLIR IR. These passes are universally applicable regardless of which dialects are in use. They include classical compiler optimizations (CSE, SCCP, inlining, LICM, canonicalization) and IR-level utilities (dead value elimination, debug info stripping, topological sorting).

## Key Files
| File | Description |
|------|-------------|
| `Canonicalizer.cpp` | Canonicalization pass: applies all registered `canonicalize` patterns via the greedy rewrite driver; supports `--top-down` mode |
| `CSE.cpp` | Common Subexpression Elimination: removes duplicate pure computations using a scoped dominator-tree walk |
| `SCCP.cpp` | Sparse Conditional Constant Propagation: combines constant folding with dead-code elimination using `DataFlowSolver` |
| `InlinerPass.cpp` | Inlining pass driver: calls the `Inliner` utility from `Utils/Inliner.cpp` with configurable depth/cost limits |
| `LoopInvariantCodeMotion.cpp` | LICM pass: hoists loop-invariant ops above `LoopLikeOpInterface` loops |
| `Mem2Reg.cpp` | Memory-to-register promotion: promotes `MemorySlot`-compliant alloc/load/store sequences to SSA values |
| `SROA.cpp` | Scalar Replacement of Aggregates: splits composite memory slots into independent per-field slots before Mem2Reg |
| `RemoveDeadValues.cpp` | Removes SSA values and ops that contribute no observable effect (aggressive DCE) |
| `SymbolDCE.cpp` | Dead symbol elimination: removes private/nested symbols (functions, globals) with no uses |
| `SymbolPrivatize.cpp` | Marks public symbols as private when they are not referenced externally |
| `ControlFlowSink.cpp` | Sinks ops into their use-dominating regions/blocks to reduce live ranges |
| `BubbleDownMemorySpaceCasts.cpp` | Pushes memory-space cast ops toward their users (enables subsequent folding) |
| `GenerateRuntimeVerification.cpp` | Inserts runtime assertion ops for ops implementing `RuntimeVerifiableOpInterface` |
| `CompositePass.cpp` | `CompositeFixedPointPass`: repeatedly applies a sub-pipeline until the IR stabilizes |
| `LocationSnapshot.cpp` | Snapshots current IR locations to a file (useful for debugging location tracking) |
| `OpStats.cpp` | Collects and prints operation count statistics for a module |
| `PrintIR.cpp` | `print-ir` pass: dumps the current IR to stderr (debugging utility) |
| `StripDebugInfo.cpp` | Removes all debug location information from the IR |
| `TopologicalSort.cpp` | Topologically sorts ops within blocks/regions for DAG regions |
| `ViewOpGraph.cpp` | Emits a Graphviz `.dot` representation of the op dependency graph |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Utils/` | Shared transformation utilities: dialect conversion, inliner, greedy rewrite driver, region utilities (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Passes here must be dialect-agnostic; they may only query ops through interfaces, not by dialect-specific op type.
- Pass options are declared via `Pass::Option<T>` fields and described in the corresponding `.td` file in `mlir/include/mlir/Transforms/`.
- Generated pass registration code is included from `mlir/include/mlir/Transforms/Passes.h.inc`.

### Common Patterns
- Passes implement `void runOnOperation()` and call `getOperation()` to get their root op.
- Passes signal failure by calling `signalPassFailure()`.
- Most passes use `PatternRewriter` or utilities from `Utils/`; direct IR mutation outside a rewriter is rare.

## Dependencies

### Internal
- `mlir/lib/IR/` — core IR
- `mlir/lib/Analysis/` — dataflow solver, liveness, slice analysis
- `mlir/lib/Interfaces/` — `LoopLikeInterface`, `SideEffectInterfaces`, `MemorySlotInterfaces`, etc.
- `mlir/lib/Transforms/Utils/` — inliner, greedy rewrite driver, dialect conversion

### External
- `llvm/lib/Support` — ADT, `llvm::DominatorTree` utilities

<!-- MANUAL: -->
