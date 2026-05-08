<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Pass

## Purpose
Implements the MLIR pass manager infrastructure. The pass manager schedules passes over the IR op hierarchy, handles nesting (module-level vs. function-level passes), provides crash recovery, collects statistics, measures timing, and maintains the pass registry for command-line pipeline construction.

## Key Files
| File | Description |
|------|-------------|
| `Pass.cpp` | Core `PassManager` and `OpPassManager` implementation: pass scheduling, op-anchor nesting, `runOnOperation()` dispatch, pass invariant verification |
| `PassCrashRecovery.cpp` | Crash recovery handler: on LLVM fatal error during a pass, dumps the pre-crash IR to a file for reproduction |
| `IRPrinting.cpp` | `IRPrinterInstrumentation`: before/after pass IR printing (triggered by `--mlir-print-ir-before`, `--mlir-print-ir-after`, etc.) |
| `PassManagerOptions.cpp` | Registers global command-line options for the pass manager (`--mlir-pass-pipeline`, `--mlir-print-ir-*`, `--mlir-disable-threading`, etc.) |
| `PassRegistry.cpp` | Pass registry: `PassRegistration`, `parsePassPipeline()`, pipeline text format parsing, `PassPipelineCLParser` |
| `PassStatistics.cpp` | Pass statistics collection and reporting (`Pass::Statistic<T>`, `--mlir-pass-statistics`) |
| `PassTiming.cpp` | `PassTiming` instrumentation: records wall/user/system time per pass and reports a summary (`--mlir-timing`) |

## For AI Agents

### Working In This Directory
- Pass registration happens via `PassRegistration<MyPass>` instantiated in the pass's `.cpp` file; the registry in `PassRegistry.cpp` maps text names to factory functions.
- Pipeline text format: `"canonicalize,cse,func.func(inliner)"` — the parser in `PassRegistry.cpp` handles nesting.
- `Pass.cpp` contains the threading model; passes at the same nesting level run concurrently across op anchors when threading is enabled. Passes must be stateless between `runOnOperation()` calls.
- `PassDetail.h` is an internal header shared across this directory only.

### Common Patterns
- New passes are registered with `MLIR_DECLARE_EXPLICIT_TYPE_ID(MyPass)` + `MLIR_DEFINE_EXPLICIT_TYPE_ID(MyPass)` and a `PassRegistration<MyPass>` object.
- Instrumentation hooks (`PassInstrumentation`) can be added to the pass manager via `addInstrumentation()`.
- Statistics are declared as `Pass::Statistic<int64_t> myCount{this, "my-count", "Description"}`.

## Dependencies

### Internal
- `mlir/lib/IR/` — `MLIRContext`, `Operation`, IR printing
- `mlir/lib/Support/Timing.cpp` — timing utilities

### External
- `llvm/lib/Support` — `llvm::ThreadPool`, `llvm::CrashRecoveryContext`, ADT, command-line option parsing

<!-- MANUAL: -->
