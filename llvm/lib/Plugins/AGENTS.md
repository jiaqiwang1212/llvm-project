<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Plugins

## Purpose
Dynamic pass plugin loading infrastructure. Allows third-party code to register custom optimization passes and analysis at runtime without modifying core LLVM.

## Key Files
| File | Description |
|------|-------------|
| `PassPlugin.cpp` | Pass plugin registration and loading |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Dynamic pass registration mechanism
- Plugin entry points and callbacks
- Integration with Extensions/ for hook registration
- dlopen/LoadLibrary abstractions for cross-platform loading

## Dependencies

### Internal
- `llvm/lib/Extensions/` — extension hooks
- `llvm/lib/Pass/` — pass infrastructure
- `llvm/lib/Support/` — dynamic library loading

<!-- MANUAL: -->
