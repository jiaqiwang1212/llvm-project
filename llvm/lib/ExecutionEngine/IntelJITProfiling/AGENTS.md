<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IntelJITProfiling

## Purpose
Header files and type definitions for Intel JIT profiling integration. Provides the Intel ittnotify API declarations and configuration for VTune and other Intel profiling tools.

## Key Files
| File | Description |
|------|-------------|
| `jitprofiling.h` | Intel JIT profiling API declarations |
| `ittnotify_config.h` | ITT (Instrumentation and Tracing Technology) configuration |
| `ittnotify_types.h` | ITT type definitions |
| `IntelJITEventsWrapper.h` | Wrapper for Intel JIT Events API |

## For AI Agents

### Working In This Directory
- This directory contains headers only; no implementation.
- Coordinate with `../IntelJITEvents/` for the actual listener implementation.
- Changes to type definitions must maintain compatibility with Intel's profiling SDK.

### Dependencies

#### Internal
- No internal dependencies; provides API declarations for external profilers

<!-- MANUAL: -->
