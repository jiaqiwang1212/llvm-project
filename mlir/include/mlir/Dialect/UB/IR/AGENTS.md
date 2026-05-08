<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# UB IR

## Purpose
Core op, interface, and matcher definitions for the UB dialect.

## Key Files
| File | Description |
|------|-------------|
| `UBOps.h` | Op class declarations |
| `UBOps.td` | ODS op definitions |
| `UBOpsInterfaces.td` | ODS interface definitions (UBOpInterface) |
| `UBMatchers.h` | Pattern matchers for detecting UB ops |

## For AI Agents

### Working In This Directory
- `UBOpInterface` identifies ops that produce or propagate poison/UB values
- `UBMatchers.h` provides helpers to detect poison in patterns

## Dependencies
- Depends on: MLIR IR core

<!-- MANUAL: -->
