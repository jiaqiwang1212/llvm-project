<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Dialect Utils

## Purpose
Shared utility headers used across multiple MLIR dialects: indexing utilities, reshape op utilities, static value helpers, structured op utilities, and verification utilities. Not a dialect itself — purely a shared utility library.

## Key Files
| File | Description |
|------|-------------|
| `IndexingUtils.h` | Indexing map and multi-dimensional indexing helpers |
| `ReshapeOpsUtils.h` | Utilities for reshape/expand/collapse shape ops |
| `StaticValueUtils.h` | Helpers for mixed static/dynamic value handling (OpFoldResult) |
| `StructuredOpsUtils.h` | Structured op utilities (iteration space, operand helpers) |
| `StructuredOpsUtils.td` | ODS definitions for structured op utilities |
| `VerificationUtils.h` | Common verification helper functions |

## For AI Agents

### Working In This Directory
- `StaticValueUtils.h` provides `getConstantIntValue`, `isConstantIntValue`, `getMixedValues` — use these when handling ops with mixed static/dynamic operands
- `IndexingUtils.h` provides `computeSuffixProduct`, `delinearize` etc. for multi-dim index math

## Dependencies
- Used by: Linalg, Tensor, MemRef, and other structured dialects

<!-- MANUAL: -->
