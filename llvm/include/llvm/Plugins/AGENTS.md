<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Plugins

## Purpose
Dynamic pass plugin loading framework. Enables third-party extensions to register custom IR passes without rebuilding LLVM.

## Key Files
| File | Description |
|------|-------------|
| `PassPlugin.h` | Plugin API and registration macros |

## For AI Agents

### Working In This Directory
Plugins enable runtime extension of the pass pipeline:
- **Dynamic loading**: Load .so/.dll files containing custom passes
- **Pass registration**: Plugins register passes via callback functions
- **Version compatibility**: Plugin API version negotiation
- **Callback-based registration**: Plugins call into LLVM to register passes

### Common Patterns
- LLVM_PLUGIN_CALLBACK macro for plugin entry point
- Plugin constructor receives PassBuilder reference
- registerFunctionPassBuilderCallback() for pipeline extension
- Plugin discovery from standard locations

## Dependencies

### Internal
- `llvm/IR` (Pass interfaces)
- `llvm/Support` (StringRef, Error)

<!-- MANUAL: -->
