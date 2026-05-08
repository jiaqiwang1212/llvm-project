<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ControlFlow Transforms

## Purpose
Bufferization and structural type conversion helpers for the ControlFlow dialect. Provides interface implementations so cf ops participate in the bufferization and type conversion frameworks.

## Key Files
| File | Description |
|------|-------------|
| `BufferDeallocationOpInterfaceImpl.h` | Buffer deallocation interface impls for cf ops |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for cf ops |
| `StructuralTypeConversions.h` | Structural type conversion patterns for cf branch ops |

## For AI Agents

### Working In This Directory
- Register these impls in driver/tool setup to enable bufferization of cf-level code
- `StructuralTypeConversions.h` provides patterns for converting block argument types across branches

## Dependencies
- Depends on: ControlFlow IR, Bufferization IR

<!-- MANUAL: -->
