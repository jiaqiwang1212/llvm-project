<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TargetParser/Unix

## Purpose
Unix-specific host detection implementation. Provides platform APIs for detecting host architecture and OS at runtime.

## Key Files
| File | Description |
|------|-------------|
| (Platform-specific C++ implementations) | Runtime host detection using Unix APIs |

## For AI Agents

### Working In This Directory
- sysconf/sysctl calls for processor detection
- /proc filesystem parsing on Linux
- Architecture detection on various Unix platforms
- Cross-platform abstraction for host querying

## Dependencies

### Internal
- `llvm/lib/TargetParser/` — public target detection interface

<!-- MANUAL: -->
