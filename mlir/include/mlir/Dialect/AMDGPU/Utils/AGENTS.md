<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# AMDGPU Utils

## Purpose
Chipset version abstraction for AMDGPU. Provides the `Chipset` struct that encodes GFX major/minor version numbers used to gate feature availability.

## Key Files
| File | Description |
|------|-------------|
| `Chipset.h` | `Chipset` struct with version parsing and comparison |

## For AI Agents

### Working In This Directory
- Use `Chipset::parse("gfx90a")` to convert architecture strings to version structs
- Version comparisons (`>=`, `<`) determine which MFMA/WMMA ops are available

## Dependencies
- Standalone utility; no dialect dependencies

<!-- MANUAL: -->
