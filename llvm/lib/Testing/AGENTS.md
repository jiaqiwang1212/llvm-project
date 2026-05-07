<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Testing

## Purpose
Test infrastructure and utilities for LLVM testing. Provides common testing helpers, annotation parsing, and support libraries used across LLVM test suite.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `Annotations/` | Source annotation parsing (see `Annotations/AGENTS.md`) |
| `Support/` | Test support utilities (see `Support/AGENTS.md`) |

## Key Files
| File | Description |
|------|-------------|
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Testing infrastructure for LLVM/Clang projects
- Support utilities for writing regression tests
- Integration with FileCheck/ for output validation
- Annotation-based test specification

## Dependencies

### Internal
- `llvm/lib/FileCheck/` — pattern matching for tests
- `llvm/lib/Support/` — utilities

<!-- MANUAL: -->
