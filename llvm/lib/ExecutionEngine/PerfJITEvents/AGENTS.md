<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PerfJITEvents

## Purpose
Integration layer for Linux perf JIT event listener. Sends JIT compilation events to Linux perf profiler to enable symbol resolution and performance profiling of JIT-compiled code.

## Key Files
| File | Description |
|------|-------------|
| `PerfJITEventListener.cpp` | Event listener implementation for Linux perf |

## For AI Agents

### Working In This Directory
- This is a thin integration layer for Linux perf.
- Changes should maintain compatibility with perf's JIT event API.
- Platform-specific: Linux only; guard includes appropriately.

### Dependencies

#### Internal
- Depends on parent ExecutionEngine infrastructure
- Optionally integrates with Linux kernel perf subsystem

<!-- MANUAL: -->
