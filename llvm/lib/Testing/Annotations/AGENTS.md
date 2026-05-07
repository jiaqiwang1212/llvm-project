<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Testing/Annotations

## Purpose
Parses source code annotations that describe test expectations. Enables embedding test specifications directly in source code for more maintainable test definitions.

## Key Files
| File | Description |
|------|-------------|
| (Annotation parsing implementation) | Source annotation parser |

## For AI Agents

### Working In This Directory
- Embedded test annotation format
- Annotation extraction and validation
- Integration with FileCheck/ for test execution
- Source location tracking

## Dependencies

### Internal
- `llvm/lib/Testing/` — testing infrastructure
- `llvm/lib/FileCheck/` — pattern matching

<!-- MANUAL: -->
