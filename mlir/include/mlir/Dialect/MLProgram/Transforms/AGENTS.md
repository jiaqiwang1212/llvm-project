<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# MLProgram Transforms

## Purpose
Transformation passes for the MLProgram dialect: bufferization interface implementations and other structural passes.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `BufferizableOpInterfaceImpl.h` | Bufferization interface impls for ml_program ops |

## For AI Agents

### Working In This Directory
- Register bufferizable impls to allow mlprogram globals to participate in One-Shot Bufferization

## Dependencies
- Depends on: MLProgram IR, Bufferization IR

<!-- MANUAL: -->
