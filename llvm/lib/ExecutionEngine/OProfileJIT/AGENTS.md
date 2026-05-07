<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# OProfileJIT

## Purpose
Integration layer for OProfile JIT event listener. Sends JIT compilation events to OProfile (Linux profiler) to enable performance analysis and symbol resolution in profiling output.

## Key Files
| File | Description |
|------|-------------|
| `OProfileJITEventListener.cpp` | Event listener implementation for OProfile |
| `OProfileWrapper.cpp` | Wrapper for OProfile shared library calls |

## For AI Agents

### Working In This Directory
- This is a thin integration layer for OProfile (Linux-only profiler).
- Changes should maintain compatibility with OProfile's shared library API.
- Platform-specific: OProfile is Linux-only; guard includes appropriately.

### Dependencies

#### Internal
- Depends on parent ExecutionEngine infrastructure
- Optionally links to OProfile shared library at runtime

<!-- MANUAL: -->
