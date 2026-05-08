<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Debug/

## Purpose
Action-based debugging infrastructure for MLIR. Provides an execution context and breakpoint system that allows external tools (debuggers, IDEs) to observe and control MLIR transformations at the granularity of "actions" (e.g., applying a pattern, running a pass). This is the foundation for the MLIR debugger (`mlir-debugger`) and step-through transformation debugging.

## Key Files
| File | Description |
|------|-------------|
| `ExecutionContext.h` | `ExecutionContext` — registers action handlers and dispatches action execution |
| `BreakpointManager.h` | `BreakpointManager` base class — decides whether to break on a given action |
| `CLOptionsSetup.h` | CLI option registration for enabling debug execution context |
| `Counter.h` | `DebugCounter` — count-based breakpoint triggering |
| `DebuggerExecutionContextHook.h` | Hook for integrating with external debugger processes |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `BreakpointManagers/` | Concrete breakpoint manager implementations (see `BreakpointManagers/AGENTS.md`) |
| `Observers/` | Action observer implementations for logging and profiling (see `Observers/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- The debug framework is opt-in: wrap transformation logic with `ExecutionContext::executeAction<MyAction>(...)`.
- `BreakpointManager` subclasses decide (given an `Action`) whether execution should pause.
- This infrastructure is separate from LLVM's `DEBUG()` macros — it is structural, not log-based.
- CLI setup via `CLOptionsSetup.h` adds `--mlir-debug-*` flags to tool option parsers.

### Common Patterns
- Define a custom action: `struct MyAction : ActionImpl<MyAction> { ... }`.
- Register handler: `context.registerActionHandler(myHandler)`.
- Break on file/line: use `FileLineColLocBreakpointManager`.

## Dependencies

### Internal
- `mlir/IR/` (MLIRContext, Operation, Action)
- `mlir/Support/` (LogicalResult)

### External
- `llvm/Support/` (raw_ostream)

<!-- MANUAL: -->
