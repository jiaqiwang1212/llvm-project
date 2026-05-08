<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Shape Transforms

## Purpose
Transformation passes for the Shape dialect: shape inference, constraint removal, and bufferization interface implementations.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for shape ops |

## For AI Agents

### Working In This Directory
- Implementations live in `lib/Dialect/Shape/Transforms/`

## Dependencies
- Depends on: Shape IR, Bufferization IR

<!-- MANUAL: -->
