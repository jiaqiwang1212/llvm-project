<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IntelJITEvents

## Purpose
Integration layer for Intel JIT profiler event listener. Sends JIT compilation events to Intel's profiling infrastructure (VTune, etc.) to enable detailed performance analysis of JIT-compiled code.

## Key Files
| File | Description |
|------|-------------|
| `IntelJITEventListener.cpp` | Event listener implementation for Intel profiling |

## For AI Agents

### Working In This Directory
- This is a thin integration layer that bridges ExecutionEngine events to Intel profiling APIs.
- Changes should maintain backward compatibility with Intel's profiling tools.
- Coordinate with `../IntelJITProfiling/` (header definitions and wrapper headers).

### Dependencies

#### Internal
- Depends on `../IntelJITProfiling/` (jitprofiling.h, ittnotify_*)
- Depends on parent ExecutionEngine infrastructure

<!-- MANUAL: -->
