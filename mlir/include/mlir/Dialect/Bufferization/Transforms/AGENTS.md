<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization Transforms

## Purpose
Bufferization analysis, pass declarations, and transformation utilities: One-Shot analysis, module bufferization, buffer utilities, deallocation insertion, and copy-on-write analysis.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |
| `Transforms.h` | Public transformation APIs |
| `Bufferize.h` | `bufferizeOp` entry point and options |
| `OneShotAnalysis.h` | One-Shot Bufferization analysis driver |
| `OneShotModuleBufferize.h` | Module-level One-Shot Bufferization entry point |
| `BufferUtils.h` | Buffer utility functions (copy insertion, cast insertion) |
| `BufferViewFlowAnalysis.h` | Alias and view-flow analysis for buffers |
| `FuncBufferizableOpInterfaceImpl.h` | Func dialect bufferization interface impls |

## For AI Agents

### Working In This Directory
- `OneShotAnalysis.h` is the main entry for running the analysis phase
- `FuncBufferizableOpInterfaceImpl.h` must be registered to bufferize function boundaries

## Dependencies
- Depends on: Bufferization IR, MemRef dialect, Func dialect

<!-- MANUAL: -->
