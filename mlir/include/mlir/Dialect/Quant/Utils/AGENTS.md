<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Quant Utils

## Purpose
Utility functions for quantization: fake-quantization parameter computation and uniform quantization math helpers.

## Key Files
| File | Description |
|------|-------------|
| `FakeQuantSupport.h` | Fake-quantization parameter conversion helpers |
| `UniformSupport.h` | Uniform quantization scale/zero-point computation utilities |

## For AI Agents

### Working In This Directory
- `FakeQuantSupport.h` converts min/max ranges to scale/zero-point for uniform quant
- `UniformSupport.h` provides the math for quantize/dequantize operations

## Dependencies
- Depends on: Quant IR types

<!-- MANUAL: -->
