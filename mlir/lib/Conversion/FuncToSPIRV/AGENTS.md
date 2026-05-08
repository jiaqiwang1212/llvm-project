<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToSPIRV Conversion

## Purpose
Lowers Func dialect ops to SPIR-V dialect ops. Converts `func.func` → `spirv.func`, `func.call` → `spirv.FunctionCall`, `func.return` → `spirv.Return`/`spirv.ReturnValue`.

## Key Files
| File | Description |
|------|-------------|
| `FuncToSPIRV.cpp` | Conversion patterns from func ops to spirv function ops |
| `FuncToSPIRVPass.cpp` | Pass definition |

## For AI Agents

### Working In This Directory
- SPIR-V functions must be within a `spirv.module`; this conversion expects that structure to be present.
- `spirv.func` carries execution model attributes; entry points must be decorated separately.
- Function types go through `SPIRVTypeConverter` — pointer types and struct args are adjusted per SPIR-V ABI.
- Multi-result functions are not directly supported by SPIR-V — callers must use output pointer parameters.

## Dependencies
- Source: `mlir/Dialect/Func`
- Target: `mlir/Dialect/SPIRV`

<!-- MANUAL: -->
