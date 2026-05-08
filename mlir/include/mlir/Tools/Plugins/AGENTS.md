<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# include/mlir/Tools/Plugins/

## Purpose
Plugin loading infrastructure headers for dynamically extending MLIR tools. Provides `DialectPlugin` and `PassPlugin` for loading shared libraries at runtime that register additional dialects or passes into a running MLIR tool, enabling out-of-tree dialect/pass development without recompiling the tool.

## Key Files
| File | Description |
|------|-------------|
| `DialectPlugin.h` | `DialectPlugin` — dynamically loads a shared library and registers its dialects |
| `PassPlugin.h` | `PassPlugin` — dynamically loads a shared library and registers its passes |

## Subdirectories
(none)

## For AI Agents

### Working In This Directory
- Plugin shared libraries must export `mlirGetDialectPluginInfo()` (for dialect plugins) or `mlirGetPassPluginInfo()` (for pass plugins).
- Plugins are loaded via `--load-dialect-plugin=path/to/plugin.so` in `mlir-opt`.
- The plugin ABI is versioned; plugin and tool must be built against compatible MLIR versions.

### Common Patterns
- Plugin entry point: `extern "C" DialectPluginLibraryInfo LLVM_ATTRIBUTE_WEAK mlirGetDialectPluginInfo() { return {MLIR_PLUGIN_API_VERSION, "MyDialect", "1.0", registerMyDialect}; }`

## Dependencies

### Internal
- `mlir/IR/` (DialectRegistry)
- `mlir/Pass/` (PassRegistry)

### External
- `llvm/Support/DynamicLibrary.h` (shared library loading)

<!-- MANUAL: -->
