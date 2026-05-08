<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Debug/BreakpointManagers/

## Purpose
Concrete `BreakpointManager` implementations for the MLIR debug action framework. Each manager implements a different strategy for deciding whether to break on a given action during IR transformation.

## Key Files
| File | Description |
|------|-------------|
| `FileLineColLocBreakpointManager.h` | Breaks when an action's associated location matches a specified file/line/column |
| `TagBreakpointManager.h` | Breaks when an action's tag string matches a registered tag |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Register a breakpoint manager with `ExecutionContext::addBreakpointManager()`.
- `FileLineColLocBreakpointManager` is useful for stopping at a specific source location during transformation.
- `TagBreakpointManager` is useful for stopping at named action points (e.g., `"applyPattern"`).
- Multiple managers can be registered simultaneously; any match triggers a break.

### Common Patterns
- `FileLineColLocBreakpointManager mgr; mgr.addBreakpoint("file.mlir", 42); context.addBreakpointManager(&mgr);`

## Dependencies

### Internal
- `mlir/Debug/BreakpointManager.h`
- `mlir/IR/` (Location)

### External
- `llvm/ADT/` (StringRef)

<!-- MANUAL: -->
