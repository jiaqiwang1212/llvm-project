<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# SPIRV Utils

## Purpose
Layout utility functions for the SPIRV dialect: computing decorated type layouts (std140, std430) required for SPIR-V uniform and storage buffers.

## Key Files
| File | Description |
|------|-------------|
| `LayoutUtils.h` | Type layout computation for SPIR-V decorated structs |

## For AI Agents

### Working In This Directory
- `VulkanLayoutUtils::decorateType` adds offset/stride decorations per Vulkan layout rules
- Required when lowering MemRef types to `spirv.struct` for uniform/storage buffer usage

## Dependencies
- Depends on: SPIRV IR

<!-- MANUAL: -->
