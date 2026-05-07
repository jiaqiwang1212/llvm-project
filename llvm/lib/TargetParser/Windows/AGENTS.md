<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# TargetParser/Windows

## Purpose
Windows-specific host detection implementation. Provides Windows APIs for detecting host architecture and OS capabilities.

## Key Files
| File | Description |
|------|-------------|
| (Platform-specific C++ implementations) | Runtime host detection using Windows APIs |

## For AI Agents

### Working In This Directory
- GetSystemInfo and related Windows API calls
- Registry-based CPU feature detection
- IsProcessorFeaturePresent for ISA support
- Architecture detection on Windows platforms

## Dependencies

### Internal
- `llvm/lib/TargetParser/` — public target detection interface

<!-- MANUAL: -->
