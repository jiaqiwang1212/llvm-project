<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Linker

## Purpose
IR-level linker that merges multiple LLVM modules into a single module. Core component of LTO and used by runtime environments that need to link compiled code.

## Key Files
| File | Description |
|------|-------------|
| `LinkModules.cpp` | Module merging and linking logic |
| `IRMover.cpp` | IR entity movement between modules |
| `LinkDiagnosticInfo.h` | Diagnostic information for linker errors |
| `CMakeLists.txt` | Build configuration |

## For AI Agents

### Working In This Directory
- Module merging semantics and conflict resolution
- Symbol/function/type unification across modules
- Appending vs linking modes
- Integration with LTO/ for whole-program optimization

## Dependencies

### Internal
- `llvm/lib/IR/` — IR representation
- `llvm/lib/Transforms/` — may reference transformation passes

<!-- MANUAL: -->
