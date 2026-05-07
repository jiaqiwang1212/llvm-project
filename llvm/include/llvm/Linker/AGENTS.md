<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linker

## Purpose
IR-level module linking for LTO and interprocedural optimization. Merges multiple LLVM modules with symbol resolution and relocation.

## Key Files
| File | Description |
|------|-------------|
| `Linker.h` | Main linker interface with linkModules() |
| `IRMover.h` | Low-level IR merging and relocation |

## For AI Agents

### Working In This Directory
The Linker module provides module-level composition:
- **linkModules()**: Merge two modules with symbol collision handling
- **Symbol renaming**: Automatic renaming for conflicts
- **Type merging**: Unify duplicate type definitions
- **Function aliasing**: Create aliases instead of copies when appropriate

### Common Patterns
- Create Linker(dest_module) then call linkInModule(src_module)
- Handle linking errors via Expected<void> return
- Control linking via LinkFlags (LinkOnlyNeeded, etc.)

## Dependencies

### Internal
- `llvm/IR` (Module, Function, GlobalVariable)
- `llvm/Support` (Error, SmallVector)

<!-- MANUAL: -->
