<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# mlir/lib/Tools/Plugins

## Purpose
Implements dynamic plugin loading for MLIR tools. Allows users to load external shared libraries containing additional dialects or passes into `mlir-opt` and other tools at runtime via `--load-dialect-plugin` and `--load-pass-plugin` flags.

## Key Files
| File | Description |
|------|-------------|
| `DialectPlugin.cpp` | `DialectPlugin`: loads a shared library and calls its `mlirGetDialectPluginInfo()` entry point to register the contained dialects into the `DialectRegistry` |
| `PassPlugin.cpp` | `PassPlugin`: loads a shared library and calls its `mlirGetPassPluginInfo()` entry point to register the contained passes into the global pass registry |

## For AI Agents

### Working In This Directory
- Plugin shared libraries must export `mlirGetDialectPluginInfo()` (for dialect plugins) or `mlirGetPassPluginInfo()` (for pass plugins) with C linkage.
- Plugin loading uses `llvm::sys::DynamicLibrary`; the library remains loaded for the process lifetime.
- This mirrors the `clang` / `opt` plugin mechanism; the API is intentionally similar.

### Common Patterns
- Plugin entry point prototype: `extern "C" ::mlir::DialectPluginLibraryInfo mlirGetDialectPluginInfo();`
- Plugins register dialects via the `DialectRegistry` passed to `mlirGetDialectPluginInfo`.

## Dependencies

### Internal
- `mlir/lib/IR/` — `DialectRegistry`
- `mlir/lib/Pass/PassRegistry.cpp` — pass registration

### External
- `llvm/lib/Support` — `llvm::sys::DynamicLibrary`

<!-- MANUAL: -->
