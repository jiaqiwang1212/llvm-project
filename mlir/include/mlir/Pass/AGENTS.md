<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Pass/

## Purpose
Pass manager infrastructure headers. Defines the `Pass` base class, `PassManager`, pass pipeline configuration, pass options, instrumentation hooks, and the pass registry. This is the entry point for defining and running MLIR transformation and analysis passes.

## Key Files
| File | Description |
|------|-------------|
| `Pass.h` | `Pass` base class; `OperationPass<>`, `InterfacePass<>` CRTP templates |
| `PassBase.td` | TableGen base classes for pass definitions (`Pass`, `PassBase`) |
| `PassManager.h` | `PassManager` and `OpPassManager` — schedule and run pass pipelines |
| `PassOptions.h` | `PassOptions` — type-safe pass option parsing and declaration |
| `PassRegistry.h` | Pass registry: `registerPass()`, pipeline text parsing |
| `PassInstrumentation.h` | `PassInstrumentation` hooks: before/after pass/pipeline callbacks |
| `AnalysisManager.h` | `AnalysisManager` — cache and invalidate analyses per operation |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- New passes are declared in dialect-specific `Passes.td` files using `def MyPass : Pass<"my-pass", "OpType">`.
- The generated `PassBase` class (from TableGen) provides option fields and `getArgument()`.
- Pass implementations inherit from the generated base and override `runOnOperation()`.
- `AnalysisManager` is accessed via `getAnalysis<MyAnalysis>()` inside `runOnOperation()`.
- Thread safety: passes run concurrently on sibling ops; do not share mutable state across pass instances.

### Common Patterns
- Pass declaration: `def MyPass : Pass<"my-pass", "func::FuncOp"> { let options = [...]; }`
- Pass implementation: `struct MyPass : impl::MyPassBase<MyPass> { void runOnOperation() override; }`
- Pipeline text: `"func.func(cse,canonicalize)"` parsed via `parsePassPipeline()`.
- Instrumentation: subclass `PassInstrumentation` and add via `pm.addInstrumentation()`.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, Operation)
- `mlir/Support/` (LogicalResult, TypeID)
- `mlir/Analysis/` (analyses consumed via AnalysisManager)

### External
- `llvm/ADT/` (StringRef, SmallVector)
- `llvm/Support/` (CommandLine for option registration)

<!-- MANUAL: -->
