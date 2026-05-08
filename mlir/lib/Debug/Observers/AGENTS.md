<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Debug/Observers

## Purpose
Implements observer implementations that subscribe to the MLIR action execution context. Observers receive callbacks when actions are fired and can log, profile, or otherwise react to compilation events.

## Key Files
| File | Description |
|------|-------------|
| `ActionLogging.cpp` | `ActionLogger`: logs action names, types, and associated IR snippets to a stream when actions are fired; activated by `--mlir-print-ir-tree-dir` and similar flags |
| `ActionProfiler.cpp` | `ActionProfiler`: records wall-clock timing for each action instance and produces a summary; useful for profiling which patterns/passes consume most time |

## For AI Agents

### Working In This Directory
- New observers implement the `ActionObserver` or `ExecutionContextObserver` interface.
- `ActionLogging.cpp` is the reference implementation for observers that capture IR snapshots.

## Dependencies

### Internal
- `mlir/lib/Debug/ExecutionContext.cpp`
- `mlir/lib/IR/` — IR printing

### External
- `llvm/lib/Support` — timing, `llvm::raw_ostream`

<!-- MANUAL: -->
