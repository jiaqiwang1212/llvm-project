<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ConvertToEmitC

## Purpose
Provides the generic EmitC conversion infrastructure. Declares pass and interface for converting arbitrary dialect ops to EmitC ops for C/C++ code emission.

## Key Files
| File | Description |
|------|-------------|
| `ConvertToEmitCPass.h` | Pass pipeline entry-points and option structs |
| `ConvertToEmitCPatternInterface.td` | TableGen interface definitions for EmitC conversion patterns |
| `ToEmitCInterface.h` | C++ interface declarations generated from the TableGen interface |
| `CMakeLists.txt` | Build rules for this directory |

## For AI Agents

### Working In This Directory
- Edit `.td` files to add or modify interfaces; regenerate with `mlir-tblgen`
- `ConvertToEmitCPass.h` declares the generic umbrella pass
- The implementation lives under `mlir/lib/Conversion/ConvertToEmitC/`

## Dependencies
- Target dialect: `include/mlir/Dialect/EmitC/`
- Consumed by all `*ToEmitC` passes

<!-- MANUAL: -->
