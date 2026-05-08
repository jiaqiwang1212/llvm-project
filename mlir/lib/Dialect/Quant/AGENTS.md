<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Quant Dialect

## Purpose
Implements the Quant dialect — MLIR's quantization type system and operations. Provides uniform/per-axis quantized types (`!quant.uniform<i8:f32, scale:zp>`), dequantize/quantize ops, and passes for quantization-aware lowering of ML models.

## Key Files
| File | Description |
|------|-------------|
| `IR/QuantOps.cpp` | Op implementations: `quant.dcast`, `quant.qcast`, `quant.scast`, `quant.const` |
| `IR/QuantTypes.cpp` | Quantized type implementations: `UniformQuantizedType`, `UniformQuantizedPerAxisType`, `CalibratedQuantizedType` |
| `IR/QuantDialectBytecode.cpp` | Bytecode serialization for quant types/attrs |
| `IR/TypeParser.cpp` | Custom type parser for `!quant.uniform<...>` syntax |
| `Transforms/LowerQuantOps.cpp` | Lowers quant ops to arithmetic (dequantize → multiply-add) |
| `Transforms/NormalizeQuantTypes.cpp` | Normalizes quantized types to canonical representations |
| `Transforms/StripFuncQuantTypes.cpp` | Removes quant types from function signatures |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Quantized types, ops, bytecode, type parser |
| `Transforms/` | Quantized op lowering, type normalization, function stripping |
| `Utils/` | Shared quantization utilities |

## For AI Agents

### Working In This Directory
- Quantized types encode `storageType` (e.g., `i8`), `expressedType` (e.g., `f32`), scale, and zero point.
- `quant.dcast` (dequantize cast) converts quantized tensor to expressed-type tensor; `quant.qcast` (quantize cast) goes the other way.
- `LowerQuantOps.cpp` replaces `quant.dcast` with `(x - zp) * scale` arithmetic using Arith ops.
- `TypeDetail.h` contains the storage for quantized type parameters — internal only, not part of public API.
- Bytecode support in `QuantDialectBytecode.cpp` enables efficient `.mlirbc` serialization of models with quant types.

### Common Patterns
- Type verification: quant types check that storage integer type bit width is compatible with the expressed float type.
- `UniformQuantizedType::getQuantizedElementType()` extracts storage type from a quantized tensor type.

## Dependencies
- `mlir/Dialect/Arith`, `mlir/IR`, `mlir/Bytecode`

<!-- MANUAL: -->
