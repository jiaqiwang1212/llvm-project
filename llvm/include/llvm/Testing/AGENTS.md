<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Testing

## Purpose
Test support utilities and infrastructure. Provides source annotations, test helpers, and integration with Google Mock for unit and integration tests.

## Key Files
| File | Description |
|------|-------------|
| `Annotations/` | Source code annotations for tests |
| `Support/` | Test support utilities |
| `ADT/` | ADT-specific test helpers |
| `Demangle/` | Demangle test utilities |

## For AI Agents

### Working In This Directory
Testing provides utilities for compiler test development:
- **Source annotations**: Mark code for test coverage and validation
- **Test helpers**: Assertion and utility functions for tests
- **GMock adapters**: Integration with Google Mock framework
- **ADT testing**: Utilities for testing abstract data types

### Common Patterns
- Use Annotations for source-level test markers
- SupportHelpers for common test operations
- GMockAdaptors for custom mock comparisons
- Integration with unittest and gtest frameworks

## Dependencies

### Internal
- `llvm/Support` (StringRef, raw_ostream)
- `llvm/ADT` (various container tests)
- Google Mock/gtest (external)

<!-- MANUAL: -->
