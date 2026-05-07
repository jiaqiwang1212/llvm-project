<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Telemetry

## Purpose
Pluggable telemetry and metrics framework for compiler instrumentation and performance analysis.

## Key Files
| File | Description |
|------|-------------|
| `Telemetry.h` | Telemetry interface and utilities |

## For AI Agents

### Working In This Directory
Telemetry provides instrumentation points for monitoring:
- **Event tracking**: Emit custom events during compilation
- **Metrics collection**: Track counters and timers
- **Pluggable backends**: Route telemetry to external systems
- **Performance analysis**: Identify compiler bottlenecks

### Common Patterns
- Register telemetry events during compilation
- Query telemetry backends for custom metrics
- Integrate with profiling and logging systems
- Disable at compile-time for zero overhead

## Dependencies

### Internal
- `llvm/Support` (StringRef)

<!-- MANUAL: -->
