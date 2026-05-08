<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Debug/Observers/

## Purpose
Action observer implementations for the MLIR debug framework. Observers passively monitor action execution without stopping it, enabling logging and profiling of MLIR transformation activity.

## Key Files
| File | Description |
|------|-------------|
| `ActionLogging.h` | `ActionLogger` — logs action execution events to a `raw_ostream` (name, IR snapshot before/after) |
| `ActionProfiler.h` | `ActionProfiler` — records timing and frequency of action executions for profiling |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Observers are registered with `ExecutionContext::addObserver()`.
- `ActionLogger` is useful for debugging transformation pipelines by printing which actions fire and on what IR.
- `ActionProfiler` helps identify hot transformation patterns.
- Observers do not affect execution control; use `BreakpointManager` (in `BreakpointManagers/`) to pause execution.

### Common Patterns
- `ActionLogger logger(llvm::errs()); context.addObserver(&logger);`

## Dependencies

### Internal
- `mlir/Debug/ExecutionContext.h`
- `mlir/IR/` (Operation)

### External
- `llvm/Support/raw_ostream.h`

<!-- MANUAL: -->
