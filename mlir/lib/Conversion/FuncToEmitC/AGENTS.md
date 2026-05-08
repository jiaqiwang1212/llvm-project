<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# FuncToEmitC Conversion

## Purpose
Lowers Func dialect ops to EmitC dialect ops for C/C++ emission. Converts `func.func` → `emitc.func`, `func.call` → `emitc.call`, `func.return` → `emitc.return`.

## Key Files
| File | Description |
|------|-------------|
| `FuncToEmitC.cpp` | Conversion patterns from func ops to emitc function ops |
| `FuncToEmitCPass.cpp` | Pass definition |

## For AI Agents

### Working In This Directory
- `func.func` carries MLIR types; EmitC requires types to be expressible as C types — type conversion must be complete before this runs.
- `emitc.func` supports the same attribute structure as `func.func` but emits directly as C function syntax.
- Function argument and return types go through `EmitCTypeConverter`.

## Dependencies
- Source: `mlir/Dialect/Func`
- Target: `mlir/Dialect/EmitC`

<!-- MANUAL: -->
