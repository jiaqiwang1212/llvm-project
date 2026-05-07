<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Telemetry

## Purpose
Pluggable telemetry framework for collecting and reporting compiler metrics and diagnostics. Enables structured data collection without hardcoding telemetry backends.

## Key Files
| File | Description |
|------|-------------|
| `Telemetry.cpp` | Telemetry infrastructure and callbacks |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Event-based telemetry infrastructure
- Pluggable backend support
- Metrics collection hooks
- Integration points for compiler monitoring

## Dependencies

### Internal
- `llvm/lib/Support/` — utilities

<!-- MANUAL: -->
