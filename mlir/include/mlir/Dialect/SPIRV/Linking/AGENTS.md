<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV Linking

## Purpose
SPIR-V module combining/linking utilities: merges multiple `spirv.module` ops into one, resolving symbol conflicts and merging capability/extension requirements.

## Key Files
| File | Description |
|------|-------------|
| `ModuleCombiner.h` | SPIR-V module combiner/linker declarations |

## For AI Agents

### Working In This Directory
- Use `combine` from `ModuleCombiner.h` to merge separately compiled SPIR-V modules

## Dependencies
- Depends on: SPIRV IR

<!-- MANUAL: -->
