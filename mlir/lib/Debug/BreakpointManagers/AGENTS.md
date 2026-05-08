<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Debug/BreakpointManagers

## Purpose
Implements breakpoint manager implementations for the MLIR action debug infrastructure. Breakpoint managers match incoming actions against user-configured breakpoint specifications and signal the execution context to pause or log.

## Key Files
| File | Description |
|------|-------------|
| `FileLineColLocBreakpointManager.cpp` | `FileLineColLocBreakpointManager`: matches actions associated with a `FileLineColLoc` location against user-specified file/line/column breakpoints; used by the debugger hook to stop at source-level positions |

## For AI Agents

### Working In This Directory
- New breakpoint managers implement the `BreakpointManager` interface from `mlir/include/mlir/Debug/BreakpointManagers/`.
- The file/line/col manager is primarily useful when IR retains source locations from the original program.

## Dependencies

### Internal
- `mlir/lib/Debug/ExecutionContext.cpp`
- `mlir/lib/IR/Location.cpp` — `FileLineColLoc`

### External
- `llvm/lib/Support` — ADT

<!-- MANUAL: -->
