<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Quant Dialect

## Purpose
Provides quantization types and operations for representing quantized neural network models. Defines uniform quantization types (per-tensor and per-axis) and ops for quantize/dequantize and fake-quantization.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op and type definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Quantization transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Quantization utility functions (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- `quant.uniform<storage:expressed:scale:zero_point>` is the canonical quantized type syntax
- Op names follow `quant.*` convention

### Common Patterns
- `quant.qcast` / `quant.dcast` cast between float and quantized types
- `quant.scast` performs storage-type casts for quantization-aware training

## Dependencies
- Depends on: Arith dialect (for quantization math), MemRef/Tensor types

<!-- MANUAL: -->
