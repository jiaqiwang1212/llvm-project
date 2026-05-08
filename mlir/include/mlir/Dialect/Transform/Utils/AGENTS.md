<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform Utils

## Purpose
Utility types and functions for the Transform dialect: `DiagnosedSilenceableFailure` for communicable transform errors, `RaggedArray` for irregular arrays of handles, and general transform utility functions.

## Key Files
| File | Description |
|------|-------------|
| `DiagnosedSilenceableFailure.h` | DiagnosedSilenceableFailure and DiagnosedDefiniteFailure types |
| `RaggedArray.h` | RaggedArray: a 2D array with irregular row sizes |
| `Utils.h` | General transform utility functions |

## For AI Agents

### Working In This Directory
- Transform ops return `DiagnosedSilenceableFailure`: use `success()`, `silenceableFailure()`, or `definiteFailure()`
- `RaggedArray` is used for grouped handle payloads (e.g., per-op operand lists)

## Dependencies
- Depends on: Transform Interfaces, MLIR IR core

<!-- MANUAL: -->
