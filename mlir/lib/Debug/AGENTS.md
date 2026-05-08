<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Debug

## Purpose
Implements MLIR's action-based debugging infrastructure. Actions are named, structured events that occur during compilation (e.g., pattern applications, pass executions). This library provides the execution context hooks that allow external debuggers and loggers to observe, pause, or intercept these actions at runtime.

## Key Files
| File | Description |
|------|-------------|
| `CLOptionsSetup.cpp` | Registers command-line options for the debug infrastructure (`--mlir-debug-only`, action filter options) and wires up the default observers |
| `DebugCounter.cpp` | `DebugCounter`: counts action occurrences and enables/disables actions after a configured threshold (useful for bisecting miscompiles) |
| `DebuggerExecutionContextHook.cpp` | Installs a debugger-friendly breakpoint hook into the action execution context, enabling debugger breakpoints on specific action names |
| `ExecutionContext.cpp` | `MLIRContext` execution context: the action dispatch table, observer registration, and action firing protocol |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `BreakpointManagers/` | Breakpoint manager implementations that match actions to user-defined breakpoints (see `BreakpointManagers/AGENTS.md`) |
| `Observers/` | Observer implementations: action logging and profiling (see `Observers/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- Actions are fired via `MLIRContext::executeAction<ActionT>(callback, ...)` — the infrastructure calls registered observers before/after invoking the callback.
- Observers implement `ActionActiveObserver` or `ActionObserver`; register with `context.registerActionHandler()`.
- `DebugCounter` is the recommended tool for narrowing down the first bad transformation in a miscompile.

### Common Patterns
- Action types are declared with `MLIR_DECLARE_EXPLICIT_ACTION_TYPE_ID(MyAction)`.
- Observers are composed; multiple observers can be active simultaneously.

## Dependencies

### Internal
- `mlir/lib/IR/MLIRContext.cpp` — action dispatch is integrated into `MLIRContext`
- `mlir/lib/Pass/` — pass execution fires actions

### External
- `llvm/lib/Support` — `llvm::DebugCounter`, ADT

<!-- MANUAL: -->
