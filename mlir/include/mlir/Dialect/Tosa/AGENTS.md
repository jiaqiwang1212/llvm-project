<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Tosa Dialect

## Purpose
TOSA (Tensor Operator Set Architecture) dialect. Implements the TOSA specification for portable ML operator semantics: element-wise, convolution, pooling, activation, data layout, and control flow ops. Used as a stable IR for ML framework interop.

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `IR/` | Op, interface, and compliance definitions (see `IR/AGENTS.md`) |
| `Transforms/` | Transformation passes (see `Transforms/AGENTS.md`) |
| `Utils/` | Conversion and quantization utilities (see `Utils/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- TOSA ops are defined by the TOSA specification; avoid adding non-spec ops here
- ODS `.td` files generate C++ — edit `.td`, not generated `.h.inc` files

### Common Patterns
- Op names follow `tosa.*` convention (e.g., `tosa.conv2d`, `tosa.matmul`, `tosa.clamp`)
- Lowered via TosaToLinalg, TosaToArith, TosaToSCF, TosaToTensor conversions

## Dependencies
- Depends on: Tensor dialect, Arith dialect, Quant dialect (for quantized types)

<!-- MANUAL: -->
