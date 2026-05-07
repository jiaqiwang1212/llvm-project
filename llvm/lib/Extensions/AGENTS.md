<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Extensions

## Purpose
Implements optional extension hook registration system that allows dynamically loaded plugins to register passes and other components into the LLVM compilation pipeline without modifying core code.

## Key Files
| File | Description |
|------|-------------|
| `Extensions.cpp` | Extension registration infrastructure |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Register extension hooks for dynamic pass loading
- Used by Plugins/ directory for plugin integration
- Understand how optional components hook into the compilation pipeline

## Dependencies

### Internal
- `llvm/lib/Plugins/` — dynamic plugin loading system

<!-- MANUAL: -->
