<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# LTO

## Purpose
Link-time optimization (LTO) APIs and infrastructure. Provides configuration, module linking, thin LTO support, and compiler plugin integration for whole-program optimization.

## Key Files
| File | Description |
|------|-------------|
| `LTO.h` | Main LTO C++ API (recommended) |
| `Config.h` | LTO configuration options |
| `LTOBackend.h` | Backend optimization pass |
| (legacy/) | Deprecated C API (legacy support) |

## For AI Agents

### Working In This Directory
LTO enables whole-program optimization across object files. Key usage patterns:
- **Configuration**: Tune optimization levels, parallelism, machine type
- **Module linking**: Combine multiple modules for global analysis
- **Thin LTO**: Parallel LTO variant for large codebases
- **Plugin integration**: Custom passes via plugin API

### Common Patterns
- AddPass pattern for extending optimization pipeline
- Config struct for controlling behavior
- Lazy module loading for thin LTO
- Cache management for incremental builds

## Dependencies

### Internal
- `llvm/IR` (Module, Function, PassManager)
- `llvm/Support` (Error, SmallVector)
- `llvm/Linker` (module linking)

<!-- MANUAL: -->
