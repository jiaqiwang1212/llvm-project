<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Transform Transforms

## Purpose
Passes for the Transform dialect itself: the Transform interpreter pass that executes a transform script against a payload module, and related utilities.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `TransformInterpreterUtils.h` | Interpreter execution utilities |

## For AI Agents

### Working In This Directory
- `createTransformInterpreterPass` is the main entry point to run a transform script
- `TransformInterpreterUtils.h` provides `runTransformPipeline` for programmatic invocation

## Dependencies
- Depends on: Transform IR, Transform Interfaces

<!-- MANUAL: -->
