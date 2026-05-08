<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform Interfaces

## Purpose
Core interface definitions for the Transform dialect: `TransformOpInterface` (for all transform ops), `TransformHandleTypeInterface` (for handle types), and `MatchOpInterface` (for match/predicate ops).

## Key Files
| File | Description |
|------|-------------|
| `TransformInterfaces.h` | Core interface declarations |
| `TransformInterfaces.td` | ODS interface definitions |
| `MatchInterfaces.h` | Match interface declarations |
| `MatchInterfaces.td` | ODS match interface definitions |

## For AI Agents

### Working In This Directory
- Every transform op MUST implement `TransformOpInterface` to be executable
- `TransformHandleTypeInterface` is required for types used as transform op operands/results
- `MatchOpInterface` is for ops that select/filter other ops from a handle

## Dependencies
- Depends on: MLIR IR core; used by all transform extension ops

<!-- MANUAL: -->
