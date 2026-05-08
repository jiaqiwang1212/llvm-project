<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Quant IR

## Purpose
Core op, type, and dialect definitions for the Quant dialect.

## Key Files
| File | Description |
|------|-------------|
| `Quant.h` | Op and type class declarations |
| `QuantBase.td` | Dialect definition and base classes |
| `QuantOps.td` | ODS op definitions |
| `QuantTypes.h` | Quantized type class declarations (UniformQuantizedType, etc.) |
| `QuantDialectBytecode.td` | Bytecode serialization definitions |

## For AI Agents

### Working In This Directory
- Edit `QuantOps.td` to add new quantization ops
- `QuantTypes.h` declares `UniformQuantizedType` and `UniformQuantizedPerAxisType`

## Dependencies
- Depends on: MLIR built-in integer/float types

<!-- MANUAL: -->
