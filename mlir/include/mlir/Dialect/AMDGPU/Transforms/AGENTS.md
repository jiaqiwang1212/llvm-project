<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU Transforms

## Purpose
Transformation pass declarations for the AMDGPU dialect, such as emulation of unsupported ops on specific chipsets.

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pass registration declarations |
| `Passes.td` | ODS pass definitions |

## For AI Agents

### Working In This Directory
- Implementations live in `lib/Dialect/AMDGPU/Transforms/`
- Check `Utils/Chipset.h` for chipset version-gating logic

## Dependencies
- Depends on: AMDGPU IR, GPU dialect

<!-- MANUAL: -->
