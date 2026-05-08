<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# ArithToEmitC Conversion

## Purpose
Lowers Arith dialect ops to EmitC dialect ops for C/C++ code emission. Converts integer and float arithmetic, comparisons, and casts to EmitC expression ops that emit valid C/C++ syntax.

## Key Files
| File | Description |
|------|-------------|
| `ArithToEmitC.cpp` | Conversion patterns for arith ops to emitc equivalents |
| `ArithToEmitCPass.cpp` | Pass definition and registration |

## For AI Agents

### Working In This Directory
- `arith.addi` → `emitc.add`, `arith.cmpi` → `emitc.cmp`, `arith.constant` → `emitc.constant`, etc.
- Integer overflow semantics are handled via C casts to unsigned types where needed (C UB for signed overflow).
- Float cast ops use C cast syntax (`(float)x`).
- Part of the EmitC lowering pipeline for generating portable C from MLIR.

## Dependencies
- Source: `mlir/Dialect/Arith`
- Target: `mlir/Dialect/EmitC`

<!-- MANUAL: -->
