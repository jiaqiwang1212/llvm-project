<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# IndexToSPIRV Conversion

## Purpose
Lowers the Index dialect to SPIR-V dialect. Converts `index.*` arithmetic ops to SPIR-V integer arithmetic ops using the appropriate integer width for the SPIR-V target (typically `i32` for Vulkan/OpenCL).

## Key Files
| File | Description |
|------|-------------|
| `IndexToSPIRV.cpp` | Conversion patterns from index ops to spirv integer ops |

## For AI Agents

### Working In This Directory
- SPIR-V typically uses 32-bit indices; the conversion maps `index` to `i32` unless the `Int64` capability is declared.
- Overflow behavior follows SPIR-V integer semantics (wrapping).
- `index.casts` between `index` and `iN` types become `spirv.UConvert`/`spirv.SConvert` as appropriate.

## Dependencies
- Source: `mlir/Dialect/Index`
- Target: `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
